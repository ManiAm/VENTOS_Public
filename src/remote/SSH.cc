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
#include <fstream>
#include "utf8.h"
#include <omnetpp.h>

namespace VENTOS {

SSH::~SSH()
{
    // free SFTP session first
    if(SFTP_session)
        sftp_free(SFTP_session);

    // then free SSH session
    if(SSH_session)
    {
        ssh_disconnect(SSH_session);
        ssh_free(SSH_session);
    }
}


// constructor
SSH::SSH(std::string host, int port, std::string username, std::string password)
{
    this->this_host = host;

    checkHost(host);

    SSH_session = ssh_new();
    if (SSH_session == NULL)
        throw cRuntimeError("SSH session error!");

    ssh_options_set(SSH_session, SSH_OPTIONS_HOST, host.c_str());
    ssh_options_set(SSH_session, SSH_OPTIONS_PORT, &port);
    ssh_options_set(SSH_session, SSH_OPTIONS_USER, username.c_str());
    int verbosity = SSH_LOG_NOLOG;
    ssh_options_set(SSH_session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);

    printf("  SSH to %s@%s at port %d \n", username.c_str(), host.c_str(), port);
    std::cout.flush();

    int rc = ssh_connect(SSH_session);
    if (rc != SSH_OK)
        throw cRuntimeError("Error connecting to localhost: %s", ssh_get_error(SSH_session));

    // verify the server's identity
    if (verify_knownhost() < 0)
    {
        ssh_disconnect(SSH_session);
        ssh_free(SSH_session);
        throw cRuntimeError("oh no!");
    }

    // get the protocol version of the session
    printf("  SSH version @%s: %d \n", host.c_str(), ssh_get_version(SSH_session));
    std::cout.flush();

    // get the server banner
    printf("  Server banner @%s: %s \n", host.c_str(), ssh_get_serverbanner(SSH_session));
    std::cout.flush();

    // get issue banner
    char *str = ssh_get_issue_banner(SSH_session);
    if(str)
        std::cout << "Issue banner: " << str << std::endl;

    printf("  Authenticating to %s ... Please Wait \n", host.c_str());
    std::cout.flush();
    authenticate(password);

    SFTP_session = sftp_new(SSH_session);
    if (SFTP_session == NULL)
        throw cRuntimeError("Error allocating SFTP session: %s", ssh_get_error(SSH_session));

    rc = sftp_init(SFTP_session);
    if (rc != SSH_OK)
    {
        sftp_free(SFTP_session);
        throw cRuntimeError("Error initializing SFTP session: %s.", ssh_get_error(SFTP_session));
    }
}


std::string SSH::getHost()
{
    return this->this_host;
}


void SSH::checkHost(std::string host)
{
    struct hostent *he = gethostbyname(host.c_str());  // needs Internet connection to resolve DNS names
    if (he == NULL)
        throw cRuntimeError("hostname %s is invalid!", host.c_str());

    struct in_addr **addr_list = (struct in_addr **) he->h_addr_list;

    char IPAddress[100];
    for(int i = 0; addr_list[i] != NULL; i++)
        strcpy(IPAddress, inet_ntoa(*addr_list[i]));

    std::cout << "  Pinging " << IPAddress << "\n";
    std::cout.flush();

    // test if IPAdd is alive?
    std::string cmd = "ping -c 1 -s 1 " + std::string(IPAddress) + " > /dev/null 2>&1";
    int result = system(cmd.c_str());
    if(result != 0)
        throw cRuntimeError("host at %s is not responding!", IPAddress);
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


    printf("  Supported authentication methods @%s: ", getHost().c_str());
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


void SSH::copyFile_SCP(boost::filesystem::path source, boost::filesystem::path dest)
{
    ASSERT(SSH_session);

    // make sure file at 'source' exists
    if (!boost::filesystem::exists(source))
        throw cRuntimeError("File %s not found!", source.c_str());

    ssh_scp SCP_session = ssh_scp_new(SSH_session, SSH_SCP_WRITE | SSH_SCP_RECURSIVE, dest.c_str());
    if (SCP_session == NULL)
        throw cRuntimeError("Error allocating SCP session: %s", ssh_get_error(SSH_session));

    int rc = ssh_scp_init(SCP_session);
    if (rc != SSH_OK)
    {
        ssh_scp_free(SCP_session);
        throw cRuntimeError("Error initializing SCP session: %s.", ssh_get_error(SCP_session));
    }

    // read file contents into a string
    std::ifstream ifs(source.c_str());
    std::string content( (std::istreambuf_iterator<char>(ifs) ),
            (std::istreambuf_iterator<char>()    ) );
    int length = content.size();

    std::string fileName = source.filename().string();
    rc = ssh_scp_push_file(SCP_session, fileName.c_str(), length, S_IRWXU);
    if (rc != SSH_OK)
        throw cRuntimeError("Can't open remote file: %s", ssh_get_error(SSH_session));

    rc = ssh_scp_write(SCP_session, content.c_str(), length);
    if (rc != SSH_OK)
        throw cRuntimeError("Can't write to remote file: %s", ssh_get_error(SSH_session));

    ssh_scp_close(SCP_session);
    ssh_scp_free(SCP_session);
}


void SSH::copyFile_SFTP(boost::filesystem::path source, boost::filesystem::path dest)
{
    ASSERT(SSH_session);
    ASSERT(SFTP_session);

    // make sure file at 'source' exists
    if (!boost::filesystem::exists(source))
        throw cRuntimeError("File %s not found!", source.c_str());

    // read file contents into a string
    std::ifstream ifs(source.c_str());
    std::string content( (std::istreambuf_iterator<char>(ifs) ),
            (std::istreambuf_iterator<char>()    ) );
    int length = content.size();

    boost::filesystem::path remoteFile = dest / source.filename();
    int access_type = O_WRONLY | O_CREAT | O_TRUNC;
    sftp_file file = sftp_open(SFTP_session, remoteFile.c_str(), access_type, S_IRWXU);
    if (file == NULL)
        throw cRuntimeError("Can't open file for writing: %s", ssh_get_error(SSH_session));

    int nwritten = sftp_write(file, content.c_str(), length);
    if (nwritten != length)
        throw cRuntimeError("Can't write data to file: %s", ssh_get_error(SSH_session));

    int rc = sftp_close(file);
    if (rc != SSH_OK)
        throw cRuntimeError("Can't close the written file: %s", ssh_get_error(SSH_session));
}


// get the remote directory content --list both files and directories
std::vector<sftp_attributes> SSH::listDir(boost::filesystem::path dirpath)
{
    ASSERT(SSH_session);
    ASSERT(SFTP_session);

    sftp_dir dir = sftp_opendir(SFTP_session, dirpath.c_str());
    if (!dir)
        throw cRuntimeError("Directory not opened: %s", ssh_get_error(SSH_session));

    std::vector<sftp_attributes> dirListings;

    sftp_attributes attributes;
    while ((attributes = sftp_readdir(SFTP_session, dir)) != NULL)
        dirListings.push_back(attributes);

    if (!sftp_dir_eof(dir))
    {
        sftp_closedir(dir);
        throw cRuntimeError("Can't list directory: %s", ssh_get_error(SSH_session));
    }

    int rc = sftp_closedir(dir);
    if (rc != SSH_OK)
        throw cRuntimeError("Can't close directory: %s", ssh_get_error(SSH_session));

    // sort by name
    std::sort(dirListings.begin(), dirListings.end(),
            [] (sftp_attributes const& a, sftp_attributes const& b) { return std::string(a->name) < std::string(b->name); });

    return dirListings;
}


// copy all new/modified files in local directory to remote directory
void SSH::syncDir(boost::filesystem::path source, boost::filesystem::path destination)
{
    ASSERT(SSH_session);
    ASSERT(SFTP_session);

    // make sure source directory exists
    if (!boost::filesystem::exists(source))
        throw cRuntimeError("Directory %s not found!", source.c_str());

    // make sure source path is a directory
    if(!boost::filesystem::is_directory(source))
        throw cRuntimeError("source is not a directory!: %s", source.c_str());

    // get local directory listing
    std::vector<boost::filesystem::path> localDirListing;
    boost::filesystem::recursive_directory_iterator end;
    for (boost::filesystem::recursive_directory_iterator i(source); i != end; ++i)
        localDirListing.push_back((*i));

    // get the list of files in remote directory
    std::vector<sftp_attributes> remoteDirListing = listDir(destination);

    // iterate over local files
    std::vector<boost::filesystem::path> needCopy;
    for(auto &i : localDirListing)
    {
        if(boost::filesystem::is_directory(i))
            continue;

        // extract the file name from the full path
        std::string localFileName = i.filename().string();

        // search for file name in remote dir
        auto it = std::find_if(remoteDirListing.begin(), remoteDirListing.end(),
                [&localFileName](const sftp_attributes& obj) {return std::string(obj->name) == localFileName;});

        // the file does not exist in remote dir
        if(it == remoteDirListing.end())
            needCopy.push_back(i);
        else
        {
            // check this link http://stackoverflow.com/questions/12760574/string-size-is-different-on-windows-than-on-linux
            uint64_t size_local = boost::filesystem::file_size(i);
            uint64_t size_remote = (*it)->size;

            // check this link http://stackoverflow.com/questions/3385203/regarding-access-time-unix
            //int64_t modTime_local = boost::filesystem::last_write_time(i);
            //uint32_t modTime_remote = (*it)->mtime;

            if(size_local != size_remote /*|| modTime_local != modTime_remote*/)  // todo: comparing modification times is not correct!
                needCopy.push_back(i);
        }
    }

    // copy all new/modified files to the remote dir
    for(auto &i : needCopy)
        copyFile_SFTP(i, destination);
}


void SSH::run_command(std::string command, bool printOutput)
{
    ASSERT(SSH_session);

    ssh_channel channel = ssh_channel_new(SSH_session);
    if (channel == NULL)
        throw cRuntimeError("SSH error in run_command");

    int rc = ssh_channel_open_session(channel);
    if (rc != SSH_OK)
    {
        ssh_channel_free(channel);
        throw cRuntimeError("SSH error in run_command");
    }

    rc = ssh_channel_request_exec(channel, command.c_str());
    if (rc != SSH_OK)
    {
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        throw cRuntimeError("SSH error in run_command_thread");
    }

    int fd = 1;  /*write to standard output*/
    if(!printOutput)
    {
        fd = open("/dev/null", O_WRONLY);
        if (fd == -1)
            throw cRuntimeError("Cannot open /dev/null");
    }

    char buffer[256];
    int nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
    while (nbytes > 0)
    {
        if (write(fd, buffer, nbytes) != (unsigned int) nbytes)
        {
            ssh_channel_close(channel);
            ssh_channel_free(channel);
            throw cRuntimeError("SSH error in run_command_thread");
        }

        nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
    }

    if (nbytes < 0)
    {
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        throw cRuntimeError("SSH error in run_command_thread");
    }

    ssh_channel_send_eof(channel);
    ssh_channel_close(channel);
    ssh_channel_free(channel);
}

}

