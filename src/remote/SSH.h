/****************************************************************************/
/// @file    SSH.h
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

#ifndef SSHCONNECT_H_
#define SSHCONNECT_H_

#include <string>

#include <libssh/libsshpp.hpp>
#include <libssh/sftp.h>
#include <fcntl.h>   // def of  O_WRONLY | O_CREAT | O_TRUNC
#include <stdlib.h>
#include <mutex>

#undef ev
#include "boost/filesystem.hpp"

namespace VENTOS {

#define TIMEOUT_MS 100
#define EOCMD "done109enod"

class SSH
{
public:
    SSH(std::string, int, std::string, std::string, bool = false, std::string = "std::cout", std::string = "default");
    virtual ~SSH();

    void copyFile_SFTP(boost::filesystem::path, boost::filesystem::path);
    void copyFileStr_SFTP(std::string, std::string, boost::filesystem::path);
    std::vector<sftp_attributes> listDir(boost::filesystem::path dirpath);
    void createDir(boost::filesystem::path newDirpath);
    void syncDir(boost::filesystem::path, boost::filesystem::path);

    ssh_channel openShell(std::string, bool = true, bool = false);
    void closeShell(ssh_channel);

    std::string getHostName();
    std::string getHostAddress();
    int getPort();
    std::string getUsername();
    std::string getPassword();

private:
    void checkHost(std::string host, bool);
    void authenticate();
    int authenticate_kbdint();
    int verify_knownhost();
    void createSession_SFTP();

protected:
    std::string dev_hostName = "";
    std::string dev_hostIP = "";
    int dev_port = -1;
    std::string dev_username = "";
    std::string dev_password = "";

    bool printOutput = false;
    std::string category = "";
    std::string subcategory = "";

    ssh_session SSH_session;
    sftp_session SFTP_session;

    std::mutex lock_SSH_Session;  // control access to SSH session
    static std::mutex lock_prompt;  // all SSH connections share the same lock
};

}

#endif
