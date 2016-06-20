/****************************************************************************/
/// @file    SSH_Helper.cc
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

#include "SSH_Helper.h"
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <omnetpp.h>
#include <vlog.h>

namespace VENTOS {


SSH_Helper::~SSH_Helper()
{
    // set the terminating flag
    terminating = true;

    // wait until all threads terminate
    while(active_threads > 0)
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
}


// constructor
SSH_Helper::SSH_Helper(std::string host, int port, std::string username, std::string password,
        bool printOutput, std::string cat, std::string sub) : SSH(host, port, username, password, printOutput, cat, sub)
{
    active_threads = 0;
    terminating = false;
}


double SSH_Helper::rebootDev(ssh_channel SSH_channel, int timeOut)
{
    ASSERT(SSH_channel);

    if(timeOut <= 0)
        throw omnetpp::cRuntimeError("timeOut value is wrong!");

    // start measuring boot time here
    Htime_t startBoot = std::chrono::high_resolution_clock::now();

    // sending the reboot command without waiting for response
    {
        std::lock_guard<std::mutex> lock(lock_SSH_Session);

        char buffer[1000];
        int nbytes = sprintf (buffer, "%s \n", "sudo reboot");
        if (ssh_channel_write(SSH_channel, buffer, nbytes) != nbytes)
            throw omnetpp::cRuntimeError("SSH error in writing command to shell");
    }

    Htime_t startPing = std::chrono::high_resolution_clock::now();

    // keep pinging dev
    bool disconnected = false;
    while(true)
    {
        // wait for 100 ms
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        std::string cmd = "ping -c 1 -s 1 " + dev_hostIP + " > /dev/null 2>&1";
        int result = system(cmd.c_str());
        if(!disconnected && result != 0)
            disconnected = true;

        if(disconnected && result == 0)
            break;

        // waiting too long for dev to boot?
        Htime_t currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> fp_ms = currentTime - startPing;
        if(fp_ms.count() > timeOut)
            throw omnetpp::cRuntimeError("dev reboot timeout!");
    }

    // end measuring boot time here
    Htime_t endBoot = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> fp_ms = endBoot - startBoot;

    return fp_ms.count();
}


void SSH_Helper::getSudo(ssh_channel SSH_channel)
{
    std::string output = run_command_blocking(SSH_channel, "sudo -n uptime 2>&1 | grep 'load' | wc -l");

    // get the second line of the output
    std::istringstream inputStr(output);
    std::string secondLine = "";
    std::getline(inputStr, secondLine);
    std::getline(inputStr, secondLine);

    try
    {
        // we are either in sudo mode or sudo doesn't require pass
        if (std::stoi(secondLine) > 0)
        {
            //run_command_blocking(SSH_channel, "sudo su");  // todo: this loops forever cause after sudo su, echo EOCMD is never executed!
        }
        // we need password
        else
        {
            std::string password;
            std::cout << "sudo password @" + dev_hostName + ": ";
            getline(std::cin, password);

            // echo $sudoPass | sudo -S apt-get  todo
        }
    }
    catch (const std::exception& ex)
    {
        throw omnetpp::cRuntimeError("Cannot get sudo access @%s", dev_hostName.c_str());
    }
}


std::string SSH_Helper::run_command_nonblocking(ssh_channel SSH_channel, std::string command, bool printOutput, std::string category, std::string subcategory)
{
    return run_command(SSH_channel, command, false, printOutput, category, subcategory);
}


std::string SSH_Helper::run_command_blocking(ssh_channel SSH_channel, std::string command, bool printOutput, std::string category, std::string subcategory)
{
    return run_command(SSH_channel, command, true, printOutput, category, subcategory);
}


std::string SSH_Helper::run_command(ssh_channel SSH_channel, std::string command, bool blocking, bool printOutput, std::string category, std::string subcategory)
{
    ASSERT(SSH_channel);

    if(command == "")
        throw omnetpp::cRuntimeError("command is empty!");

    {
        std::lock_guard<std::mutex> lock(lock_SSH_Session);

        // run the command
        char buffer[1000];
        int nbytes = sprintf (buffer, "%s ; ret=$? ; echo "" ; echo %s ; echo $ret \n", command.c_str(), EOCMD);
        int nwritten = ssh_channel_write(SSH_channel, buffer, nbytes);
        if (nwritten != nbytes)
            throw omnetpp::cRuntimeError("SSH error in writing command to shell");
    }

    std::string command_output = "";

    // run the loop in a child thread
    std::thread thd = std::thread([=]() mutable {  // pass by value

        // read the output from remote shell
        char buffer[1000];
        int numEOF = 0;
        int indexCmdBegin = -1;
        int indexCmdEnd = -1;
        bool endFound = false;
        int indexPrintStart = -1;
        while (ssh_channel_is_open(SSH_channel) && !ssh_channel_is_eof(SSH_channel) && !terminating)
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
                break;   // do not throw error
            }
            // time-out in non-blocking commands
            else if(nbytes == 0 && !blocking)
            {
                if(numEOF < 10)
                    numEOF++;
                // keep yielding to other threads if we
                // do not print anything for 10 * 100ms = 1 second
                else
                    std::this_thread::yield();
            }
            // buffer has data
            else if(nbytes > 0)
            {
                // reset numEOF
                numEOF = 0;

                // iterate over characters that we have received in the last SSH read
                for (int ii = 0; ii < nbytes; ii++)
                {
                    char ch = static_cast<char>(buffer[ii]);
                    command_output += ch;
                }

                // look for the beginning of command output
                if(indexCmdBegin == -1)
                {
                    std::size_t pos = command_output.find("\n");
                    if(pos != std::string::npos && pos != command_output.size() - 1)
                    {
                        indexCmdBegin = pos + 1;
                        indexCmdEnd = indexCmdBegin;
                        indexPrintStart = indexCmdBegin;
                    }
                }

                // look for the end of command output
                if(!endFound && indexCmdEnd != -1)
                {
                    std::size_t pos = command_output.find(EOCMD, indexCmdEnd);
                    if(pos != std::string::npos)
                    {
                        indexCmdEnd = pos - 1;
                        endFound = true;
                    }
                    else
                        indexCmdEnd = command_output.size()- 1; // points to the last character
                }

                ASSERT(indexCmdEnd >= indexCmdBegin);

                // print what we have received so far in buffer
                if(printOutput)
                {
                    if(indexPrintStart != -1 && indexCmdEnd != -1)
                    {
                        std::string cOutput = "";
                        for(int h = indexPrintStart; h <= indexCmdEnd; h++)
                            cOutput += command_output[h];

                        LOG_EVENT_C(category, subcategory) << cOutput;
                        LOG_FLUSH_C(category, subcategory);

                        indexPrintStart = indexCmdEnd + 1;
                    }
                }

                if(endFound)
                {
                    // look for return code
                    std::string remain = command_output.substr(indexCmdEnd+1);

                    // tokenize
                    std::istringstream inputStr(remain);
                    std::string line = "";
                    std::vector<std::string> tokens;
                    while(std::getline(inputStr, line))
                        tokens.push_back(line);

                    // 0: command is still running
                    // 1: command execution is finished without error
                    // 2: command execution is finished with error
                    int returnCode = 0;
                    if(tokens.size() >= 2)
                    {
                        try
                        {
                            if (std::stoi(tokens[1]) != 0)
                                returnCode = 2;
                            else
                                returnCode = 1;
                        }
                        catch (const std::exception& ex)
                        {
                            returnCode = 2;
                        }
                    }

                    if(returnCode == 1)
                        break;
                    // last command failed
                    if(returnCode == 2)
                    {
                        // if we have not already printed the output
                        if(!printOutput)
                        {
                            if(indexCmdBegin != -1 && indexCmdEnd != -1)
                            {
                                std::string cOutput = "";
                                for(int h = indexCmdBegin; h <= indexCmdEnd; h++)
                                    cOutput += command_output[h];

                                LOG_EVENT_C(category, subcategory) << cOutput;
                                LOG_FLUSH_C(category, subcategory);
                            }
                        }

                        // let the user know
                        throw omnetpp::cRuntimeError("Command '%s' failed @%s", command.c_str(), dev_hostName.c_str());
                    }
                }
            }
        }

        if(!blocking && active_threads > 0)
            active_threads--;
    });

    if(!blocking)
    {
        thd.detach();
        active_threads++;
    }
    else
        thd.join();

    return command_output;
}


int SSH_Helper::getNumActiveThreads()
{
    return active_threads;
}

}
