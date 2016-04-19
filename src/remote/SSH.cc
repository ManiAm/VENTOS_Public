/****************************************************************************/
/// @file    SSH.cc
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author  second author name
/// @date    Apr 2016
///
/****************************************************************************/
// VENTOS, Vehicular Network Open Simulator; see http:?
// Copyright (C) 2013-2015
/****************************************************************************/
//
// This file is part of VENTOS.
// VENTOS is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32) || defined(__CYGWIN__) || defined(_WIN64)
#include <ws2tcpip.h>
#else
#include <netinet/tcp.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif

#include "SSH.h"
#include <omnetpp.h>
#include "utf8.h"

namespace VENTOS {

SSH::~SSH()
{
    if(SSH_session)
    {
        ssh_disconnect(SSH_session);
        ssh_free(SSH_session);
    }
}


// constructor
SSH::SSH(std::string host, int port, std::string username, std::string password)
{
    struct hostent *he = gethostbyname(host.c_str());
    if (he == NULL)
        throw cRuntimeError("hostname %s is invalid!", host.c_str());

    struct in_addr **addr_list = (struct in_addr **) he->h_addr_list;

    char IPAddress[100];
    for(int i = 0; addr_list[i] != NULL; i++)
        strcpy(IPAddress, inet_ntoa(*addr_list[i]));

    std::cout << std::endl << "Pinging " << IPAddress << " ... ";
    std::cout.flush();

    // test if IPAdd is alive?
    std::string cmd = "ping -c 1 -s 1 " + std::string(IPAddress) + " > /dev/null 2>&1";
    int result = system(cmd.c_str());

    if(result != 0)
        throw cRuntimeError("host at %s is not responding!", IPAddress);

    std::cout << "Done!" << std::endl;
    std::cout << "Creating SSH session ... ";
    std::cout.flush();

    SSH_session = ssh_new();
    if (SSH_session == NULL)
        throw cRuntimeError("SSH session error!");

    std::cout << "Done! \n";
    std::cout.flush();

    ssh_options_set(SSH_session, SSH_OPTIONS_HOST, host.c_str());
    ssh_options_set(SSH_session, SSH_OPTIONS_PORT, &port);
    ssh_options_set(SSH_session, SSH_OPTIONS_USER, username.c_str());
    int verbosity = SSH_LOG_NOLOG;
    ssh_options_set(SSH_session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);

    printf("SSH to %s@%s at port %d \n", username.c_str(), host.c_str(), port );
    std::cout.flush();

    int rc = ssh_connect(SSH_session);
    if (rc != SSH_OK)
        throw cRuntimeError("Error connecting to localhost: %s", ssh_get_error(SSH_session));

    // Verify the server's identity
    if (verify_knownhost() < 0)
    {
        ssh_disconnect(SSH_session);
        ssh_free(SSH_session);
        throw cRuntimeError("oh no!");
    }

    // Get the protocol version of the session
    std::cout << "SSH version: " << ssh_get_version(SSH_session) << std::endl;

    // openssh version
    std::cout << "OpenSSH version: " << ssh_get_openssh_version(SSH_session) << std::endl;

    // get the server banner
    std::cout << "Server banner: " << ssh_get_serverbanner(SSH_session) << std::endl;

    // get issue banner
    char *str = ssh_get_issue_banner(SSH_session);
    if(str)
        std::cout << "Issue banner: " << str << std::endl;

    std::cout << "Authenticating ..." << std::endl;
    authenticate(password);

    std::cout << "Connected to the remote board successfully!" << std::endl;
    std::cout << std::endl << std::flush;
}


int SSH::verify_knownhost()
{
    ASSERT(SSH_session);

    ssh_key srv_pubkey;
    int rc = ssh_get_publickey(SSH_session, &srv_pubkey);
    if (rc < 0)
        return -1;

    unsigned char *hash = NULL;
    size_t hlen;
    rc = ssh_get_publickey_hash(srv_pubkey, SSH_PUBLICKEY_HASH_SHA1, &hash, &hlen);
    ssh_key_free(srv_pubkey);
    if (rc < 0)
        return -1;

    int state = ssh_is_server_known(SSH_session);

    switch (state)
    {
    case SSH_SERVER_KNOWN_OK:
        break; /* ok */
    case SSH_SERVER_KNOWN_CHANGED:
        fprintf(stderr, "Host key for server changed: it is now:\n");
        ssh_print_hexa("Public key hash", hash, hlen);
        fprintf(stderr, "For security reasons, connection will be stopped\n");
        free(hash);
        return -1;
    case SSH_SERVER_FOUND_OTHER:
        fprintf(stderr, "The host key for this server was not found but an other type of key exists.\n");
        fprintf(stderr, "An attacker might change the default server key to confuse your client into thinking the key does not exist\n");
        free(hash);
        return -1;
    case SSH_SERVER_FILE_NOT_FOUND:
        fprintf(stderr, "Could not find known host file.\n");
        fprintf(stderr, "If you accept the host key here, the file will be automatically created.\n");
        /* fallback to SSH_SERVER_NOT_KNOWN behavior */
    case SSH_SERVER_NOT_KNOWN:
    {
        char *hexa = ssh_get_hexa(hash, hlen);
        fprintf(stderr,"The server is unknown. Do you trust the host key?\n");
        fprintf(stderr, "Public key hash: %s\n", hexa);
        free(hexa);
        char buf[10];
        if (fgets(buf, sizeof(buf), stdin) == NULL)
        {
            free(hash);
            return -1;
        }
        if (strncasecmp(buf, "yes", 3) != 0)
        {
            free(hash);
            return -1;
        }
        if (ssh_write_knownhost(SSH_session) < 0)
        {
            fprintf(stderr, "Error %s\n", strerror(errno));
            free(hash);
            return -1;
        }
        break;
    }
    case SSH_SERVER_ERROR:
        fprintf(stderr, "Error %s", ssh_get_error(SSH_session));
        free(hash);
        return -1;
    }

    free(hash);
    return 0;
}


void SSH::authenticate(std::string password)
{
    // Try to authenticate through the "none" method
    int rc = 0;
    do {
        rc = ssh_userauth_none(SSH_session, NULL);
    } while (rc == SSH_AUTH_AGAIN);  // In nonblocking mode, you've got to call this again later.

    if (rc == SSH_AUTH_ERROR)
        throw cRuntimeError("Authentication failed.");

    // this requires the function ssh_userauth_none() to be called before the methods are available.
    int method = ssh_userauth_list(SSH_session, NULL);

    std::cout << "Supported authentication methods: ";
    if(method & SSH_AUTH_METHOD_PASSWORD)
        std::cout << "PASSWORD, ";
    if(method & SSH_AUTH_METHOD_PUBLICKEY)
        std::cout << "PUBLICKEY, ";
    if(method & SSH_AUTH_METHOD_HOSTBASED)
        std::cout << "HOSTBASED, ";
    if(method & SSH_AUTH_METHOD_INTERACTIVE)
        std::cout << "INTERACTIVE, ";
    if(method & SSH_AUTH_METHOD_GSSAPI_MIC)
        std::cout << "GSSAPI_MIC, ";

    std::cout << std::endl;
    std::cout.flush();

    while (rc != SSH_AUTH_SUCCESS)
    {
        // Try to authenticate with public key first
        if (method & SSH_AUTH_METHOD_PUBLICKEY)
        {
            rc = ssh_userauth_publickey_auto(SSH_session, NULL, NULL);
            if (rc == SSH_AUTH_ERROR)
                throw cRuntimeError("Authentication failed.");
            else if (rc == SSH_AUTH_SUCCESS)
                break;
        }

        // Try to authenticate with keyboard interactive"
        if (method & SSH_AUTH_METHOD_INTERACTIVE)
        {
            rc = authenticate_kbdint();
            if (rc == SSH_AUTH_ERROR)
                throw cRuntimeError("Authentication failed.");
            else if (rc == SSH_AUTH_SUCCESS)
                break;
        }

        // Try to authenticate with password
        if (method & SSH_AUTH_METHOD_PASSWORD)
        {
            // make sure the password is in UFT-8
            std::string temp;
            utf8::replace_invalid(password.begin(), password.end(), back_inserter(temp));
            password = temp;

            // Authenticate ourselves
            rc = ssh_userauth_password(SSH_session, NULL, password.c_str());
            if (rc == SSH_AUTH_ERROR)
                throw cRuntimeError("Authentication failed.");
            else if (rc == SSH_AUTH_SUCCESS)
                break;
        }

        // In SSH2 sometimes we need to ask the password interactively!
        char *passUser = getpass("Password: ");

        rc = ssh_userauth_password(SSH_session, NULL, passUser);
        if (rc == SSH_AUTH_ERROR)
            throw cRuntimeError("Authentication failed.");
        else if (rc == SSH_AUTH_SUCCESS)
            break;
    }
}


int SSH::authenticate_kbdint()
{
    ASSERT(SSH_session);

    int rc = ssh_userauth_kbdint(SSH_session, NULL, NULL);
    while (rc == SSH_AUTH_INFO)
    {
        const char *name = ssh_userauth_kbdint_getname(SSH_session);
        const char *instruction = ssh_userauth_kbdint_getinstruction(SSH_session);
        int nprompts = ssh_userauth_kbdint_getnprompts(SSH_session);
        if (strlen(name) > 0)
            printf("%s\n", name);

        if (strlen(instruction) > 0)
            printf("%s\n", instruction);

        for (int iprompt = 0; iprompt < nprompts; iprompt++)
        {
            char echo;
            const char *prompt = ssh_userauth_kbdint_getprompt(SSH_session, iprompt, &echo);
            if (echo)
            {
                char buffer[128], *ptr;
                printf("%s", prompt);

                if (fgets(buffer, sizeof(buffer), stdin) == NULL)
                    return SSH_AUTH_ERROR;

                buffer[sizeof(buffer) - 1] = '\0';

                if ((ptr = strchr(buffer, '\n')) != NULL)
                    *ptr = '\0';

                if (ssh_userauth_kbdint_setanswer(SSH_session, iprompt, buffer) < 0)
                    return SSH_AUTH_ERROR;

                memset(buffer, 0, strlen(buffer));
            }
            else
            {
                char *ptr = getpass(prompt);
                if (ssh_userauth_kbdint_setanswer(SSH_session, iprompt, ptr) < 0)
                    return SSH_AUTH_ERROR;
            }
        }

        rc = ssh_userauth_kbdint(SSH_session, NULL, NULL);
    }

    return rc;
}


int SSH::run_command(std::string command)
{
    ASSERT(SSH_session);

    ssh_channel channel = ssh_channel_new(SSH_session);
    if (channel == NULL)
        return SSH_ERROR;

    int rc = ssh_channel_open_session(channel);
    if (rc != SSH_OK)
    {
        ssh_channel_free(channel);
        return rc;
    }

    rc = ssh_channel_request_exec(channel, command.c_str());
    if (rc != SSH_OK)
    {
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        return rc;
    }

    char buffer[256];
    int nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
    while (nbytes > 0)
    {
        if (write(1, buffer, nbytes) != (unsigned int) nbytes)
        {
            ssh_channel_close(channel);
            ssh_channel_free(channel);
            return SSH_ERROR;
        }

        nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
    }

    if (nbytes < 0)
    {
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        return SSH_ERROR;
    }

    ssh_channel_send_eof(channel);
    ssh_channel_close(channel);
    ssh_channel_free(channel);

    return SSH_OK;
}

}

