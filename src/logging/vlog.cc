/****************************************************************************/
/// @file    vlog.cc
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author  second author name
/// @date    May 2016
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

#define WANT_WINSOCK2
#include <platdep/sockets.h>

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32) || defined(__CYGWIN__) || defined(_WIN64)
#include <ws2tcpip.h>
#else
#include <netinet/tcp.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif

#include <vlog.h>
#include <algorithm>
#include "thread"

namespace VENTOS {

vlog * vlog::objPtr = NULL;
std::mutex vlog::lock_log;

Define_Module(VENTOS::vlog);

vlog::~vlog()
{
    // making sure to flush the remaining data in buffer
    if(socketPtr)
        objPtr->flush();

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32) || defined(__CYGWIN__) || defined(_WIN64)
#else
    // send SIGINT
    if (child_pid > 0)
        kill(child_pid, 15);
#endif

    if(socketPtr)
        closesocket(*static_cast<int*>(socketPtr));
}


void vlog::initialize(int stage)
{
    super::initialize(stage);

    if(stage == 0)
    {
        systemLogLevel = par("systemLogLevel").longValue();
        logRecordCMD = par("logRecordCMD").boolValue();

        // store the pointer to class object
        objPtr = this;
    }
}


void vlog::finish()
{

}


void vlog::handleMessage(omnetpp::cMessage *msg)
{

}


vlog& vlog::WARNING(std::string category, std::string subcategory)
{
    return objPtr->setLog(WARNING_LOG, category, subcategory);
}


vlog& vlog::INFO(std::string category, std::string subcategory)
{
    return objPtr->setLog(INFO_LOG, category, subcategory);
}


vlog& vlog::ERROR(std::string category, std::string subcategory)
{
    return objPtr->setLog(ERROR_LOG, category, subcategory);
}


vlog& vlog::DEBUG(std::string category, std::string subcategory)
{
    return objPtr->setLog(DEBUG_LOG, category, subcategory);
}


vlog& vlog::EVENT(std::string category, std::string subcategory)
{
    return objPtr->setLog(EVENT_LOG, category, subcategory);
}


void vlog::flush()
{
    objPtr->sendToLogWindow(std::string("3||") + "-");
}


vlog& vlog::setLog(uint8_t logLevel, std::string category, std::string subcategory)
{
    if(category == "")
        throw omnetpp::cRuntimeError("category name can't be empty!");

    if(category == "std::cout")
    {
        lastLogLevel = logLevel;
        lastCategory = category;

        return *this;
    }

    static bool initLogWindow = false;

    if(!initLogWindow)
    {
        // create a child process
        // forking creates an exact copy of the parent process at the time of forking.
        child_pid = fork();

        // fork failed
        if(child_pid < 0)
            throw omnetpp::cRuntimeError("fork failed!");

        // in child process
        if(child_pid == 0)
        {
            // make the child process ignore the SIGINT signal
            signal(SIGINT, SIG_IGN);

            // run 'logWindow' inside this child process
            // if execution is successful then child will be blocked at this line
            int r = system("src/loggingWindow/logWindow");  // relative path to logWindow

            if (r == -1)
                throw omnetpp::cRuntimeError("Running logWindow failed during system()");

            if (WEXITSTATUS(r) != 0)
                throw omnetpp::cRuntimeError("logWindow exited with code %d", WEXITSTATUS(r));

            exit(0);
        }
        else
        {
            std::cout << "\n>>> Opening the 'log window' process ... \n";
            std::cout << boost::format("  Parent PID %1% \n") % getpid();
            std::cout << boost::format("  Child  PID %1% \n") % child_pid;
            std::cout.flush();

            start_TCP_client();
        }

        initLogWindow = true;
    }

    auto it = find(allCategories.begin(), allCategories.end(), category);
    if(it == allCategories.end())
    {
        // adding a new tab with name 'category'
        sendToLogWindow(std::string("1||") + category);

        allCategories.push_back(category);
    }

    lastLogLevel = logLevel;
    lastCategory = category;

    return *this;
}


void vlog::start_TCP_client()
{
    if (initsocketlibonce() != 0)
        throw omnetpp::cRuntimeError("Could not init socketlib");

    in_addr addr;
    struct hostent* host_ent;

    if ((host_ent = gethostbyname("localhost")))
        addr = *((struct in_addr*) host_ent->h_addr_list[0]);
    else
        throw omnetpp::cRuntimeError("Invalid TCP server address");

    sockaddr_in address;
    sockaddr* address_p = (sockaddr*)&address;
    memset(address_p, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(45676);
    address.sin_addr.s_addr = addr.s_addr;

    socketPtr = new int();
    if (*socketPtr < 0)
        throw omnetpp::cRuntimeError("Could not create socket to connect to TCP server");

    int tries = 1;
    for (; tries <= 10; ++tries)
    {
        *socketPtr = ::socket(AF_INET, SOCK_STREAM, 0);
        if (::connect(*socketPtr, address_p, sizeof(address)) >= 0)
            break;

        assert(socketPtr);
        closesocket(*static_cast<int*>(socketPtr));

        int sleepDuration = tries * .25 + 1;

        std::cout << boost::format("  Could not connect to the TCP server: %1% -- retry in %2% seconds. \n") % strerror(sock_errno()) % sleepDuration;
        std::cout.flush();

        std::this_thread::sleep_for(std::chrono::seconds(sleepDuration));
    }

    if(tries == 11)
        throw omnetpp::cRuntimeError("Could not connect to the TCP server after 10 retries!");

    {
        int x = 1;
        ::setsockopt(*socketPtr, IPPROTO_TCP, TCP_NODELAY, (const char*) &x, sizeof(x));
    }

    std::cout << "  Successfully connected. \n\n";
    std::cout.flush();
}


void vlog::sendToLogWindow(std::string msg)
{
    // fill the buffer
    char tx_buffer[1000];
    sprintf (tx_buffer, "%s", msg.c_str());

    // sending the msg to the TCP server
    int n = write(*socketPtr, tx_buffer, strlen(tx_buffer));
    if (n < 0)
    {
        std::cout << "\n\nWriting error to socket \n";
        std::cout.flush();
        return;
    }

    // receiving msg from the TCP server
    char rx_buffer[20];
    bzero(rx_buffer, 20);
    n = read(*socketPtr, rx_buffer, 19);
    if (n < 0)
    {
        std::cout << "\n\nReading error from socket \n";
        std::cout.flush();
        return;
    }

    if(std::string(rx_buffer) != "got it!")
    {
        std::cout << "\n\nServer's response msg is wrong! \n";
        std::cout.flush();
        return;
    }
}

}
