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
#include <thread>
#include "utf8.h"
#include <omnetpp.h>

namespace VENTOS {

std::mutex SSH::lock_prompt;
std::mutex SSH::lock_verify;

SSH::~SSH()
{
    if(SFTP_session)
        sftp_free(SFTP_session);

    // free SSH session at the end
    if(SSH_session)
    {
        ssh_disconnect(SSH_session);
        ssh_free(SSH_session);
    }
}


// constructor
SSH::SSH(std::string host, int port, std::string username, std::string password, bool printOutput)
{
    if(host == "")
        throw omnetpp::cRuntimeError("host is empty!");

    if(port <= 0)
        throw omnetpp::cRuntimeError("port number is invalid!");

    if(username == "")
        throw omnetpp::cRuntimeError("username is empty!");

    {
        std::lock_guard<std::mutex> lock(vlog::lock_log);
        vlog::EVENT(host) << boost::format(">>> Connecting to %1% ... \n\n") % host;
        vlog::flush();
    }

    this->dev_hostName = host;
    this->dev_port = port;
    this->dev_username = username;
    this->dev_password = password;

    checkHost(host, printOutput);

    SSH_session = ssh_new();
    if (SSH_session == NULL)
        throw omnetpp::cRuntimeError("SSH session error!");

    long timeout = 10;  // timeout for the connection in seconds
    int verbosity = SSH_LOG_NOLOG;

    ssh_options_set(SSH_session, SSH_OPTIONS_HOST, host.c_str());
    ssh_options_set(SSH_session, SSH_OPTIONS_PORT, &port);
    ssh_options_set(SSH_session, SSH_OPTIONS_USER, username.c_str());
    ssh_options_set(SSH_session, SSH_OPTIONS_TIMEOUT, &timeout);
    ssh_options_set(SSH_session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);

    if(printOutput)
    {
        std::lock_guard<std::mutex> lock(vlog::lock_log);
        vlog::EVENT(host) << boost::format("    SSH to %1%@%2% at port %3% \n") % username % host % port;
        vlog::flush();
    }

    int rc = ssh_connect(SSH_session);
    if (rc != SSH_OK)
        throw omnetpp::cRuntimeError("%s", ssh_get_error(SSH_session));

    {
        // one SSH connection at a time -- because this might prompt the user
        std::lock_guard<std::mutex> lock(lock_verify);

        // verify the server's identity
        if (verify_knownhost() < 0)
        {
            ssh_disconnect(SSH_session);
            ssh_free(SSH_session);
            throw omnetpp::cRuntimeError("Cannot verify the host!");
        }
    }

    if(printOutput)
    {
        std::lock_guard<std::mutex> lock(vlog::lock_log);

        // get the protocol version of the session
        vlog::EVENT(dev_hostName) << boost::format("    SSH version @%1% --> %2% \n") % host % ssh_get_version(SSH_session);

        // get the server banner
        vlog::EVENT(dev_hostName) << boost::format("    Server banner @%1%: \n        %2% \n") % host % ssh_get_serverbanner(SSH_session);

        // get issue banner
        char *str = ssh_get_issue_banner(SSH_session);
        if(str)
            vlog::EVENT(dev_hostName) << boost::format("    Issue banner: %1% \n") % str % ssh_get_serverbanner(SSH_session);

        vlog::flush();
    }

    if(printOutput)
    {
        std::lock_guard<std::mutex> lock(vlog::lock_log);
        vlog::EVENT(dev_hostName) << boost::format("    Authenticating to %1% ... Please wait \n") % host;
        vlog::flush();
    }

    authenticate(password);

    // create a new SFTP session for file transfer
    createSession_SFTP();
}


void SSH::checkHost(std::string host, bool printOutput)
{
    struct hostent *he = gethostbyname(host.c_str());  // needs Internet connection to resolve DNS names
    if (he == NULL)
        throw omnetpp::cRuntimeError("hostname %s is invalid!", host.c_str());

    struct in_addr **addr_list = (struct in_addr **) he->h_addr_list;

    char IPAddress[100];
    for(int i = 0; addr_list[i] != NULL; i++)
        strcpy(IPAddress, inet_ntoa(*addr_list[i]));

    this->dev_hostIP = IPAddress;

    if(printOutput)
    {
        std::lock_guard<std::mutex> lock(vlog::lock_log);
        vlog::EVENT(dev_hostName) << "    Pinging " << IPAddress << "\n";
        vlog::flush();
    }

    // test if IPAdd is alive?
    std::string cmd = "ping -c 1 -s 1 " + std::string(IPAddress) + " > /dev/null 2>&1";
    int result = system(cmd.c_str());
    if(result != 0)
        throw omnetpp::cRuntimeError("host at %s is not responding!", IPAddress);
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
        std::cout << "Host key for server changed: it is now: \n";
        ssh_print_hexa("Public key hash", hash, hlen);
        std::cout << "For security reasons, connection will be stopped. \n";
        std::cout.flush();
        free(hash);
        return -1;

    case SSH_SERVER_FOUND_OTHER:
        std::cout << "The host key for this server was not found but an other type of key exists. \n";
        std::cout << "An attacker might change the default server key to confuse your client into thinking the key does not exist. \n";
        std::cout.flush();
        free(hash);
        return -1;

    case SSH_SERVER_FILE_NOT_FOUND:
        std::cout << "Could not find known host file. \n";
        std::cout << "If you accept the host key here, the file will be automatically created. \n";
        std::cout.flush();
        /* fallback to SSH_SERVER_NOT_KNOWN behavior */

    case SSH_SERVER_NOT_KNOWN:
    {
        char *hexa = ssh_get_hexa(hash, hlen);
        std::cout << boost::format("The authenticity of host '%1% (%2%)' can't be established. \n") % dev_hostName % dev_hostIP;
        std::cout << "Public key hash is " << hexa << " \n";
        free(hexa);

        std::cout << "Are you sure you want to continue connecting (yes/no)? ";
        std::cout.flush();

        while(true)
        {
            std::string answer;
            getline(std::cin, answer);

            if(answer == "no")
            {
                free(hash);
                return -1;
            }
            else if(answer == "yes")
            {
                if (ssh_write_knownhost(SSH_session) < 0)
                {
                    free(hash);
                    return -1;
                }

                // insert a new line for improving readability
                std::cout << "\n";
                std::cout.flush();

                break; // break out of loop
            }
            else
            {
                std::cout << "Please type 'yes' or 'no': ";
                std::cout.flush();
            }
        }

        break;
    }

    case SSH_SERVER_ERROR:
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
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        rc = ssh_userauth_none(SSH_session, NULL);
    } while (rc == SSH_AUTH_AGAIN);  // In nonblocking mode, you've got to call this again later.

    if (rc == SSH_AUTH_ERROR)
        throw omnetpp::cRuntimeError("Authentication failed.");

    // this requires the function ssh_userauth_none() to be called before the methods are available.
    int method = ssh_userauth_list(SSH_session, NULL);

    // Try to authenticate with public key first
    if (method & SSH_AUTH_METHOD_PUBLICKEY)
    {
        rc = ssh_userauth_publickey_auto(SSH_session, NULL, NULL);
        if (rc == SSH_AUTH_ERROR)
            throw omnetpp::cRuntimeError("Authentication failed.");
        else if (rc == SSH_AUTH_SUCCESS)
            return;
    }

    // Try to authenticate with keyboard interactive"
    if (method & SSH_AUTH_METHOD_INTERACTIVE)
    {
        rc = authenticate_kbdint();
        if (rc == SSH_AUTH_ERROR)
            throw omnetpp::cRuntimeError("Authentication failed.");
        else if (rc == SSH_AUTH_SUCCESS)
            return;
    }

    // Try to authenticate with password
    if (method & SSH_AUTH_METHOD_PASSWORD)
    {
        // if password is provided then try it
        if(password != "")
        {
            // make sure the password is in UFT-8
            std::string temp;
            utf8::replace_invalid(password.begin(), password.end(), back_inserter(temp));
            password = temp;

            // Authenticate ourselves
            rc = ssh_userauth_password(SSH_session, NULL, password.c_str());
            if (rc == SSH_AUTH_ERROR)
                throw omnetpp::cRuntimeError("Authentication failed.");
            else if (rc == SSH_AUTH_SUCCESS)
                return;

            printf("    Username/password combination is not correct. Try again! \n");
            std::cout.flush();
        }

        // if we are here then the password did not work!

        while(true)
        {
            // only one SSH connection should access this
            std::lock_guard<std::mutex> lock(lock_prompt);

            std::cout << "    Password @" + dev_hostName + ": ";
            getline(std::cin, password);

            // make sure the password is in UFT-8
            std::string temp;
            utf8::replace_invalid(password.begin(), password.end(), back_inserter(temp));
            password = temp;

            // Authenticate ourselves
            rc = ssh_userauth_password(SSH_session, NULL, password.c_str());
            if (rc == SSH_AUTH_ERROR)
                throw omnetpp::cRuntimeError("Authentication failed.");
            else if (rc == SSH_AUTH_SUCCESS)
                return;

            printf("    Username/password combination is not correct. Try again! \n");
            std::cout.flush();
        }
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


void SSH::createSession_SFTP()
{
    ASSERT(SSH_session);

    SFTP_session = sftp_new(SSH_session);
    if (SFTP_session == NULL)
        throw omnetpp::cRuntimeError("Error allocating SFTP session: %s", ssh_get_error(SSH_session));

    int rc = sftp_init(SFTP_session);
    if (rc != SSH_OK)
    {
        sftp_free(SFTP_session);
        throw omnetpp::cRuntimeError("Error initializing SFTP session: %s.", ssh_get_error(SFTP_session));
    }
}


void SSH::copyFile_SFTP(boost::filesystem::path source, boost::filesystem::path remote_dir)
{
    ASSERT(SSH_session);
    ASSERT(SFTP_session);

    // make sure file at 'source' exists
    if (!boost::filesystem::exists(source))
        throw omnetpp::cRuntimeError("File %s not found!", source.c_str());

    // read file contents into a string
    std::ifstream ifs(source.c_str());
    std::string content( (std::istreambuf_iterator<char>(ifs) ),
            (std::istreambuf_iterator<char>()    ) );
    int length = content.size();

    boost::filesystem::path remoteFile = remote_dir / source.filename();
    int access_type = O_WRONLY | O_CREAT | O_TRUNC;
    sftp_file file = sftp_open(SFTP_session, remoteFile.c_str(), access_type, S_IRWXU);
    if (file == NULL)
        throw omnetpp::cRuntimeError("Can't open file for writing: %s", ssh_get_error(SSH_session));

    int nwritten = sftp_write(file, content.c_str(), length);
    if (nwritten != length)
        throw omnetpp::cRuntimeError("Can't write data to file: %s", ssh_get_error(SSH_session));

    int rc = sftp_close(file);
    if (rc != SSH_OK)
        throw omnetpp::cRuntimeError("Can't close the written file: %s", ssh_get_error(SSH_session));
}


void SSH::copyFileStr_SFTP(std::string fileName, std::string content, boost::filesystem::path remote_dir)
{
    ASSERT(SSH_session);
    ASSERT(SFTP_session);

    int length = content.size();
    if(length <= 0)
        throw omnetpp::cRuntimeError("content length should be > 0");

    if(fileName == "")
        throw omnetpp::cRuntimeError("fileName is empty!");

    boost::filesystem::path remoteFile = remote_dir / fileName;
    int access_type = O_WRONLY | O_CREAT | O_TRUNC;
    sftp_file file = sftp_open(SFTP_session, remoteFile.c_str(), access_type, S_IRWXU);
    if (file == NULL)
        throw omnetpp::cRuntimeError("Can't open file for writing: %s", ssh_get_error(SSH_session));

    int nwritten = sftp_write(file, content.c_str(), length);
    if (nwritten != length)
        throw omnetpp::cRuntimeError("Can't write data to file: %s", ssh_get_error(SSH_session));

    int rc = sftp_close(file);
    if (rc != SSH_OK)
        throw omnetpp::cRuntimeError("Can't close the written file: %s", ssh_get_error(SSH_session));
}


// get the remote directory content --list both files and directories
std::vector<sftp_attributes> SSH::listDir(boost::filesystem::path remote_dir)
{
    ASSERT(SSH_session);
    ASSERT(SFTP_session);

    sftp_dir dir = sftp_opendir(SFTP_session, remote_dir.c_str());
    if (!dir)
        throw omnetpp::cRuntimeError("Directory not opened: %s", ssh_get_error(SSH_session));

    std::vector<sftp_attributes> dirListings;

    sftp_attributes attributes;
    while ((attributes = sftp_readdir(SFTP_session, dir)) != NULL)
        dirListings.push_back(attributes);

    if (!sftp_dir_eof(dir))
    {
        sftp_closedir(dir);
        throw omnetpp::cRuntimeError("Can't list directory: %s", ssh_get_error(SSH_session));
    }

    int rc = sftp_closedir(dir);
    if (rc != SSH_OK)
        throw omnetpp::cRuntimeError("Can't close directory: %s", ssh_get_error(SSH_session));

    // sort by name
    std::sort(dirListings.begin(), dirListings.end(),
            [] (sftp_attributes const& a, sftp_attributes const& b) { return std::string(a->name) < std::string(b->name); });

    return dirListings;
}


void SSH::createDir(boost::filesystem::path newDirpath)
{
    ASSERT(SSH_session);
    ASSERT(SFTP_session);

    int rc = sftp_mkdir(SFTP_session, newDirpath.c_str(), S_IRWXU);
    if (rc != SSH_OK)
    {
        if (sftp_get_error(SFTP_session) != SSH_FX_FILE_ALREADY_EXISTS)
            throw omnetpp::cRuntimeError("Can't create directory: %s", ssh_get_error(SSH_session));
    }
}


// copy all new/modified files/folders in local directory to remote directory
void SSH::syncDir(boost::filesystem::path source, boost::filesystem::path remote_dir)
{
    ASSERT(SSH_session);
    ASSERT(SFTP_session);

    // make sure source directory exists
    if (!boost::filesystem::exists(source))
        throw omnetpp::cRuntimeError("Directory %s not found!", source.c_str());

    // make sure source path is a directory
    if(!boost::filesystem::is_directory(source))
        throw omnetpp::cRuntimeError("source is not a directory!: %s", source.c_str());

    std::vector<boost::filesystem::path> directories;
    directories.push_back(source);

    while(!directories.empty())
    {
        boost::filesystem::path currentDir = directories.back();
        directories.pop_back();

        // get local directory listing
        std::vector<boost::filesystem::path> localDirListing;
        boost::filesystem::directory_iterator end;
        for (boost::filesystem::directory_iterator i(currentDir); i != end; ++i)
            localDirListing.push_back((*i));

        std::string relativePath = currentDir.string();
        relativePath.erase(0, source.string().size());
        boost::filesystem::path newRemoteDir = remote_dir / source.filename().string() / relativePath;

        createDir(newRemoteDir);  // create directory with the same name in the remote device

        // get the list of files in remote directory
        std::vector<sftp_attributes> remoteDirListing = listDir(newRemoteDir);  // todo: list  only files

        // iterate over local files/directories
        for(auto &i : localDirListing)
        {
            if(boost::filesystem::is_directory(i))
            {
                directories.push_back(i);
                continue;
            }

            // extract the file name from the full path
            std::string localFileName = i.filename().string();

            // search for file name in remote dir
            auto it = std::find_if(remoteDirListing.begin(), remoteDirListing.end(),
                    [&localFileName](const sftp_attributes& obj) {return std::string(obj->name) == localFileName;});

            // the file does not exist in remote dir
            if(it == remoteDirListing.end())
                copyFile_SFTP(i, newRemoteDir);
            else
            {
                // check this link http://stackoverflow.com/questions/12760574/string-size-is-different-on-windows-than-on-linux
                //uint64_t size_local = boost::filesystem::file_size(i);
                //uint64_t size_remote = (*it)->size;

                // check this link http://stackoverflow.com/questions/3385203/regarding-access-time-unix
                //int64_t modTime_local = boost::filesystem::last_write_time(i);
                //uint32_t modTime_remote = (*it)->mtime;

                //if(size_local != size_remote /*|| modTime_local != modTime_remote*/)  // todo: comparing modification times is not correct!
                copyFile_SFTP(i, newRemoteDir);
            }
        }
    }
}


ssh_channel SSH::openShell(std::string shellName, bool interactive, bool keepAlive)
{
    ASSERT(SSH_session);

    ssh_channel SSH_channel = NULL;

    {
        std::lock_guard<std::mutex> lock(lock_SSH_Session);

        SSH_channel = ssh_channel_new(SSH_session);
        if (SSH_channel == NULL)
            throw omnetpp::cRuntimeError("SSH error in openShell");

        int rc = ssh_channel_open_session(SSH_channel);
        if (rc != SSH_OK)
        {
            ssh_channel_free(SSH_channel);
            throw omnetpp::cRuntimeError("SSH error in openShell");
        }

        /*
         * Interactive: any shell process that you use to type commands,
         * and get back output from those commands. The shell
         * can prompt the user to enter input.
         *
         * Non-interactive: the shell is probably run from an automated process so
         * it can't assume if can request input or that someone will see the output.
         */
        if(interactive)
        {
            rc = ssh_channel_request_pty(SSH_channel);
            if (rc != SSH_OK)
            {
                ssh_channel_free(SSH_channel);
                throw omnetpp::cRuntimeError("SSH error in openShell");
            }

            rc = ssh_channel_change_pty_size(SSH_channel, 80 /*cols*/, 30 /*rows*/);
            if (rc != SSH_OK)
            {
                ssh_channel_free(SSH_channel);
                throw omnetpp::cRuntimeError("SSH error in openShell");
            }
        }

        rc = ssh_channel_request_shell(SSH_channel);
        if (rc != SSH_OK)
        {
            ssh_channel_free(SSH_channel);
            throw omnetpp::cRuntimeError("SSH error in openShell");
        }
    }

    std::string shell_mode = interactive ? "interactive" : "non-interactive";
    std::string keepAlive_mode = keepAlive ? "with" : "without";

    {
        std::lock_guard<std::mutex> lock(vlog::lock_log);
        vlog::EVENT(dev_hostName) << boost::format(">>> Opening %1% shell '%2%' %3% keepAlive. \n\n") % shell_mode % shellName % keepAlive_mode;
        vlog::flush();
    }

    // read the greeting message from remote shell and redirect it to /dev/null
    char buffer[1000];
    while (ssh_channel_is_open(SSH_channel) && !ssh_channel_is_eof(SSH_channel))
    {
        int nbytes = 0;

        {
            std::lock_guard<std::mutex> lock(lock_SSH_Session);
            nbytes = ssh_channel_read_timeout(SSH_channel, buffer, sizeof(buffer), 0, TIMEOUT_MS);
        }

        // SSH_ERROR
        if(nbytes < 0)
        {
            ssh_channel_close(SSH_channel);
            ssh_channel_free(SSH_channel);
            throw omnetpp::cRuntimeError("SSH error in run_command");
        }
        // time out
        else if(nbytes == 0)
            break;

        //for (int ii = 0; ii < nbytes; ii++)
        //    std::cout << static_cast<char>(buffer[ii]) << std::flush;
    }

    return SSH_channel;
}


void SSH::closeShell(ssh_channel SSH_channel)
{
    if(SSH_channel)
    {
        ssh_channel_send_eof(SSH_channel);
        ssh_channel_close(SSH_channel);
        ssh_channel_free(SSH_channel);

        SSH_channel = NULL;
    }
}


std::string SSH::getHostName()
{
    return this->dev_hostName;
}


std::string SSH::getHostAddress()
{
    return this->dev_hostIP;
}


int SSH::getPort()
{
    return this->dev_port;
}


std::string SSH::getUsername()
{
    return this->dev_username;
}


std::string SSH::getPassword()
{
    return this->dev_password;
}

}
