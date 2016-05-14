/****************************************************************************/
/// @file    SSH_Helper.h
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

#ifndef SSHOS_H_
#define SSHOS_H_

#include "SSH.h"
#include <thread>
#include <atomic>

namespace VENTOS {

class SSH_Helper : public SSH
{
public:
    SSH_Helper(std::string, int, std::string, std::string, bool = false);
    virtual ~SSH_Helper();

    // sends sudo reboot to the remote dev
    double rebootDev(ssh_channel, int);

    // switch to sudo space
    void getSudo(ssh_channel);

    std::string run_command_nonblocking(ssh_channel, std::string, bool = false, std::string = "std::cout", std::string = "default");
    std::string run_command_blocking(ssh_channel, std::string, bool = false, std::string = "std::cout", std::string = "default");

    // active number of threads in this SSH session
    int getNumActiveThreads();

private:
    std::string run_command(ssh_channel, std::string, bool, bool, std::string, std::string);
    int isFinished(std::string);

private:
    typedef std::chrono::high_resolution_clock::time_point Htime_t;

    std::atomic<int> active_threads;  // number of active threads in this SSH session
    std::atomic<bool> terminating;
};

}

#endif
