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
SSH_Helper::SSH_Helper(std::string host, int port, std::string username, std::string password, bool printOutput) : SSH(host, port, username, password, printOutput)
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


std::string SSH_Helper::run_command_nonblocking(ssh_channel SSH_channel, std::string command, bool printOutput)
{
    return run_command(SSH_channel, command, false, printOutput);
}


std::string SSH_Helper::run_command_blocking(ssh_channel SSH_channel, std::string command, bool printOutput)
{
    return run_command(SSH_channel, command, true, printOutput);
}


std::string SSH_Helper::run_command(ssh_channel SSH_channel, std::string command, bool blocking, bool printOutput)
{
    ASSERT(SSH_channel);

    if(command == "")
        throw omnetpp::cRuntimeError("command is empty!");

    {
        std::lock_guard<std::mutex> lock(lock_SSH_Session);

        // run the command in shell
        char buffer[1000];
        int nbytes = sprintf (buffer, "%s ; echo $? ; echo %s \n", command.c_str(), EOCMD);
        int nwritten = ssh_channel_write(SSH_channel, buffer, nbytes);
        if (nwritten != nbytes)
            throw omnetpp::cRuntimeError("SSH error in writing command to shell");
    }

    std::string command_output = "";

    // run the loop in a child thread
    std::thread thd = std::thread([=]() mutable {  // pass by value

        // read the output from remote shell
        char buffer[1000];
        bool identFirstLine = false;
        int returnCode = -1;
        long int numEOF = 0;
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
                numEOF++;

                // keep yielding to other threads as long as we do not print anything!
                if(numEOF > 20)
                    std::this_thread::yield();
            }
            // buffer has data
            else if(nbytes > 0)
            {
                // reset numEOF
                numEOF = 0;

                // print what we have received so far in buffer
                if(printOutput)
                {
                    std::string cOutput = "";
                    for (int ii = 0; ii < nbytes; ii++)
                        cOutput += static_cast<char>(buffer[ii]);

                    // add indentation to the first line
                    if(!identFirstLine)
                    {
                        cOutput = "    " + cOutput;
                        identFirstLine = true;
                    }

                    // substituting all \r\n with \n
                    // Windows = CR LF, Linux = LF, MAC < 0SX = CR
                    boost::replace_all(cOutput, "\r\n", "\n");
                    boost::replace_all(cOutput, "\n", "\n    ");  // add indentation to the rest of the lines

                    std::cout << cOutput;
                    std::cout.flush();
                }

                // save output
                for (int ii = 0; ii < nbytes; ii++)
                    command_output += static_cast<char>(buffer[ii]);

                // monitor command output and look for EOCMD
                returnCode = isFinished(command_output);  // todo: make it more efficient? we test the whole command_output every timeh;

                if(returnCode == 1 || returnCode == 2)
                    break;
            }
        }

        // add two new lines for readability
        if(printOutput)
        {
            printf("\n\n");
            std::cout.flush();
        }

        // last command failed
        if(returnCode == 2)
        {
            // if we have not already printed the output
            if(!printOutput)
            {
                // format output
                command_output = "    " + command_output;
                boost::replace_all(command_output, "\r\n", "\n");
                boost::replace_all(command_output, "\n", "\n    ");

                printf("%s \n\n", command_output.c_str());
                std::cout.flush();
            }

            // let the user know
            throw omnetpp::cRuntimeError("Command '%s' failed @%s", command.c_str(), dev_hostName.c_str());
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


// 0: command is still running
// 1: command execution is finished without error
// 2: command execution is finished with error
int SSH_Helper::isFinished(std::string multiLineStr)
{
    // tokenize all lines
    std::istringstream inputStr(multiLineStr);
    std::string line = "";
    std::vector<std::string> tokens;
    while(std::getline(inputStr, line))
        tokens.push_back(line);

    if(tokens.size() <= 1)
        return 0;

    // note: start parsing tokens vector from the second entry!
    for(unsigned int ii = 1; ii < tokens.size(); ii++)
    {
        std::size_t pos = tokens[ii].find(EOCMD);
        // EOCMD is found
        if(pos != std::string::npos)
        {
            try
            {
                if (std::stoi(tokens[ii-1]) != 0)
                    return 2;
                else
                    return 1;
            }
            catch (const std::exception& ex)
            {
                return 2;
            }
        }
    }

    return 0;
}


int SSH_Helper::getNumActiveThreads()
{
    return active_threads;
}

}
