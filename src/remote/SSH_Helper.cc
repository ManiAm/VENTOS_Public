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

#define TIMEOUT_MS 200


SSH_Helper::~SSH_Helper()
{


}


// constructor
SSH_Helper::SSH_Helper(std::string host, int port, std::string username, std::string password, bool printOutput) : SSH(host, port, username, password, printOutput)
{


}


double SSH_Helper::rebootDev(ssh_channel SSH_channel, int timeOut)
{
    ASSERT(SSH_channel);

    if(timeOut <= 0)
        throw cRuntimeError("timeOut value is wrong!");

    // start measuring boot time here
    Htime_t startBoot = std::chrono::high_resolution_clock::now();

    // sending the reboot command without waiting for response
    char buffer[1000];
    int nbytes = sprintf (buffer, "%s \n", "sudo reboot");
    if (ssh_channel_write(SSH_channel, buffer, nbytes) != nbytes)
        throw cRuntimeError("SSH error in writing command to shell");

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
            throw cRuntimeError("dev reboot timeout!");
    }

    // end measuring boot time here
    Htime_t endBoot = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> fp_ms = endBoot - startBoot;

    return fp_ms.count();
}


void SSH_Helper::getSudo(ssh_channel SSH_channel)
{
    std::string output = run_command(SSH_channel, "sudo -n uptime 2>&1 | grep 'load' | wc -l");

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
            run_command(SSH_channel, "sudo su");
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
        throw cRuntimeError("Cannot get sudo access @%s", dev_hostName.c_str());
    }
}


// run a command and wait until it finishes execution
std::string SSH_Helper::run_command(ssh_channel SSH_channel, std::string command, int maxTimeOutCount, bool printOutput)
{
    ASSERT(SSH_channel);

    if(command == "")
        throw cRuntimeError("command is empty!");

    if(maxTimeOutCount <= 0)
        throw cRuntimeError("maxTimeOutCount value is invalid!");

    {
        // run the command in shell
        char buffer[1000];
        int nbytes = sprintf (buffer, "%s \n", command.c_str());
        int nwritten = ssh_channel_write(SSH_channel, buffer, nbytes);
        if (nwritten != nbytes)
            throw cRuntimeError("SSH error in writing command to shell");
    }

    // read the output from remote shell
    char buffer[1000];
    std::string command_output = "";
    int numTimeouts = 0;
    bool identFirstLine = false;
    while (true)
    {
        int nbytes = ssh_channel_read_timeout(SSH_channel, buffer, sizeof(buffer), 0, TIMEOUT_MS);

        // SSH_ERROR
        if(nbytes < 0)
        {
            ssh_channel_close(SSH_channel);
            ssh_channel_free(SSH_channel);
            throw cRuntimeError("SSH error in run_command");
        }
        // time out
        else if(nbytes == 0)
        {
            numTimeouts++;
            // did we wait long enough?
            if(numTimeouts >= maxTimeOutCount)
                break;
        }
        // buffer has data
        else if(nbytes > 0)
        {
            // reset counter
            numTimeouts = 0;

            // save output
            for (int ii = 0; ii < nbytes; ii++)
                command_output += static_cast<char>(buffer[ii]);

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
        }
    }

    // add two new lines after printing output to improve readability
    if(printOutput)
        printf("\n\n");

    int ret = last_command_failed(SSH_channel);
    // last command failed (ret=1) or we did not get any result (ret=-1)
    if(ret != 0)
    {
        // we have already printed the output if printOutput = true
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
        if(ret == 1)
            throw cRuntimeError("Command '%s' failed @%s", command.c_str(), dev_hostName.c_str());
        else if(ret == -1)
            throw cRuntimeError("Cannot check if the last command failed or not! @%s. Make sure the command is not blocking and allow enough time for the command to execute completely by increasing maxTimeOutCount", dev_hostName.c_str());
    }

    return command_output;
}


int SSH_Helper::last_command_failed(ssh_channel SSH_channel)
{
    // run echo %? to get the return value
    std::string command = "echo $?";

    {
        char buffer[1000];
        int nbytes = sprintf (buffer, "%s \n", command.c_str());
        int nwritten = ssh_channel_write(SSH_channel, buffer, nbytes);
        if (nwritten != nbytes)
            throw cRuntimeError("SSH error in writing command to shell");
    }

    // read the output from remote shell
    char buffer[1000];
    std::string command_output = "";
    while (true)
    {
        int nbytes = ssh_channel_read_timeout(SSH_channel, buffer, sizeof(buffer), 0, TIMEOUT_MS);

        // SSH_ERROR
        if(nbytes < 0)
        {
            ssh_channel_close(SSH_channel);
            ssh_channel_free(SSH_channel);
            throw cRuntimeError("SSH error in run_command");
        }
        // end of file
        else if(nbytes == 0)
            break;

        for (int ii = 0; ii < nbytes; ii++)
            command_output += static_cast<char>(buffer[ii]);
    }

    // get number of lines
    std::istringstream inputStr(command_output);
    std::string line = "";
    int numLines = 0;
    while(std::getline(inputStr, line))
        numLines++;

    // if 'echo $?' did not return any results
    // this might happen if the last command is still running
    if(numLines == 1)
        return -1;

    // get the second line
    inputStr.str(command_output);
    inputStr.clear();
    std::string secondLine = "";
    std::getline(inputStr, secondLine);
    std::getline(inputStr, secondLine);

    try
    {
        if (std::stoi(secondLine) != 0)
            return 1;
        else
            return 0;
    }
    catch (const std::exception& ex)
    {
        return -1;
    }
}


void SSH_Helper::run_command_loop(ssh_channel SSH_channel, std::string command, int waitTime, bool printOutput)
{
    ASSERT(SSH_channel);

    if(command == "")
        throw cRuntimeError("command is empty!");

    if(waitTime < 0)
        throw cRuntimeError("waitTime value is invalid!");

    {
        // run the command in shell
        char buffer[1000];
        int nbytes = sprintf (buffer, "%s \n", command.c_str());
        int nwritten = ssh_channel_write(SSH_channel, buffer, nbytes);
        if (nwritten != nbytes)
            throw cRuntimeError("SSH error in writing command to shell");
    }
    // run the loop in a child thread
    std::thread thd = std::thread([=]() {  // pass by value

        // read the output from remote shell
        char buffer[1000];
        bool identFirstLine = false;
        while (true)
        {
            int nbytes = ssh_channel_read_timeout(SSH_channel, buffer, sizeof(buffer), 0, TIMEOUT_MS);

            // SSH_ERROR
            if(nbytes < 0)
            {
                ssh_channel_close(SSH_channel);
                ssh_channel_free(SSH_channel);
                break;   // do not throw error
            }
            // buffer has data
            else if(nbytes > 0)
            {
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
            }
        }

    });

    thd.detach();

    // wait before returning
    std::this_thread::sleep_for(std::chrono::milliseconds(waitTime));

    // add two new lines for readability
    printf("\n\n");
    std::cout.flush();

    active_threads++;
}


int SSH_Helper::getNumActiveThreads()
{
    return active_threads;
}

}
