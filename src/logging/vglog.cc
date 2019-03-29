/****************************************************************************/
/// @file    vglog.cc
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

#ifdef __APPLE__
#define MSG_NOSIGNAL 0x4000
#endif

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32) || defined(__CYGWIN__) || defined(_WIN64)
#include <ws2tcpip.h>
#else
#include <netinet/tcp.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif

#include <algorithm>
#include <thread>
#include <sys/poll.h>
#include <signal.h>

#include "logging/vglog.h"
#include "global/utility.h"


namespace VENTOS {

vglog * vglog::objPtr = NULL;

Define_Module(VENTOS::vglog);

vglog::~vglog()
{
    std::lock_guard<std::mutex> lock(lock_socket);

    if(socketPtr)
    {
        closesocket(*static_cast<int*>(socketPtr));
        delete(socketPtr);
        socketPtr = NULL;
    }

    // Note: do not kill the log window process.

    // #if defined(_WIN32) || defined(__WIN32__) || defined(WIN32) || defined(__CYGWIN__) || defined(_WIN64)
    // #else
    //     // send SIGINT
    //     if (child_pid > 0)
    //         kill(child_pid, 15);
    // #endif
}


void vglog::initialize(int stage)
{
    super::initialize(stage);

    if(stage == 0)
    {
        loggingWindowPath = par("loggingWindowPath").stringValue();
        if(loggingWindowPath == "")
            throw omnetpp::cRuntimeError("loggingWindowPath parameter is invalid");

        loggingWindowTitle = par("loggingWindowTitle").stringValue();
        if(loggingWindowTitle == "")
            throw omnetpp::cRuntimeError("loggingWindowTitle parameter is invalid");

        syntaxHighlighting = par("syntaxHighlighting").boolValue();

        if(syntaxHighlighting)
        {
            syntaxHighlightingExpression = par("syntaxHighlightingExpression").stringValue();

            // adding a delimiter to the end
            if(syntaxHighlightingExpression != "")
                syntaxHighlightingExpression += " | ";

            // adding our default syntax highlighting
            syntaxHighlightingExpression += "text='ok' caseSensitive='false' fcolor='green' | ";
            syntaxHighlightingExpression += "text='warning' caseSensitive='false' fcolor='orange' | ";
            syntaxHighlightingExpression += "text='error' caseSensitive='false' fcolor='red'";
        }

        glogRecordCMD = par("glogRecordCMD").boolValue();
        glogActive = par("glogActive").boolValue();

        // store the pointer to class object
        objPtr = this;
    }
}


void vglog::finish()
{
    super::finish();

    // making sure to flush the remaining data in buffer
    if(socketPtr)
        GFLUSHALL();
}


void vglog::handleMessage(omnetpp::cMessage *msg)
{
    super::handleMessage(msg);
}


vglog& vglog::GLOGF(std::string tab, std::string pane)
{
    if(tab == "")
        throw omnetpp::cRuntimeError("tab name can't be empty!");

    if(pane == "")
        throw omnetpp::cRuntimeError("pane name can't be empty!");

    return objPtr->setGLog(tab, pane);
}


void vglog::GFLUSH(std::string tab, std::string pane)
{
    if(!objPtr->logActive())
        return;

    if(tab == "")
        throw omnetpp::cRuntimeError("tab name can't be empty!");

    if(pane == "")
        throw omnetpp::cRuntimeError("pane name can't be empty!");

    objPtr->sendToLogWindow(std::to_string(CMD_FLUSH) + objPtr->delimiter + tab + objPtr->delimiter + pane);
}


void vglog::GFLUSHALL()
{
    if(!objPtr->logActive())
        return;

    // iterate over all tabs
    for(auto &ii : objPtr->allCategories)
    {
        // iterate over all panes in this tab and flush each
        for(auto &jj : ii.second)
            objPtr->sendToLogWindow(std::to_string(CMD_FLUSH) + objPtr->delimiter + ii.first + objPtr->delimiter + jj);
    }
}


bool vglog::logActive()
{
    if( glogRecordCMD || omnetpp::cSimulation::getActiveEnvir()->isGUI() )
    {
        if(glogActive)
            return true;
    }

    return false;
}


vglog& vglog::setGLog(std::string tab, std::string pane)
{
    if(!logActive())
        return *this;

    // open logWindow if not open
    static bool initLogWindow = false;
    if(!initLogWindow)
    {
        openLogWindow();
        connect_to_TCP_server();

        // sending syntax highlighting
        sendToLogWindow(std::to_string(CMD_SYNTAX_HIGHLIGHTING) + objPtr->delimiter + syntaxHighlightingExpression);
        initLogWindow = true;
    }

    auto it = allCategories.find(tab);
    // this is a new tab
    if(it == allCategories.end())
    {
        // adding a new tab with pane
        sendToLogWindow(std::to_string(CMD_ADD_TAB) + objPtr->delimiter + tab + objPtr->delimiter + pane);

        // inserting the pane
        std::vector <std::string> vec;
        vec.push_back(pane);
        allCategories[tab] = vec;

        this->lastTab = tab;
        this->lastPane = pane;
        return *this;
    }

    // look for pane name
    auto ii = find(it->second.begin(), it->second.end(), pane);
    // no such pane exists
    if(ii == it->second.end())
    {
        // adding a new pane inside this tab
        sendToLogWindow(std::to_string(CMD_ADD_SUB_TEXTVIEW) + objPtr->delimiter + tab + objPtr->delimiter + pane);
        it->second.push_back(pane);
    }

    this->lastTab = tab;
    this->lastPane = pane;
    return *this;
}


void vglog::openLogWindow()
{
    DEBUG(__FILE__, __LINE__) << "\n>>> Starting logWindow process ... \n";
    DEBUG(__FILE__, __LINE__) << "    Executable file: " << loggingWindowPath << " \n";
    FLUSH();

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

        std::ostringstream cmd;
        cmd << boost::format("%s \"%s\" \"%s\" ") % loggingWindowPath % loggingWindowTitle % std::to_string(systemLogLevel);

        // run 'logWindow' inside this child process
        // if execution is successful then child will be blocked at this line
        int r = system(cmd.str().c_str());

        if (r == -1)
            throw omnetpp::cRuntimeError("Running logWindow failed during system()");

        if (WEXITSTATUS(r) != 0)
            throw omnetpp::cRuntimeError("logWindow exited with code %d", WEXITSTATUS(r));

        _exit(0);
    }
    else
    {
        DEBUG(__FILE__, __LINE__) << "    logWindow has started successfully in process " << child_pid << " \n" << std::flush;
    }
}


void vglog::connect_to_TCP_server()
{
    if (initsocketlibonce() != 0)
        throw omnetpp::cRuntimeError("Could not init socketlib");

    sockaddr_in address;
    sockaddr* address_p = (sockaddr*)&address;
    memset(address_p, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(45676);
    address.sin_addr.s_addr = utility::getIPv4ByHostName("localhost").ipv4_n;

    socketPtr = new int();
    if (*socketPtr < 0)
        throw omnetpp::cRuntimeError("Could not create socket to connect to TCP server");

    // wait for 1 second and then try connecting to TCP server
    std::this_thread::sleep_for(std::chrono::seconds(1));

    int tries = 1;
    for (; tries <= 10; ++tries)
    {
        *socketPtr = ::socket(AF_INET, SOCK_STREAM, 0);
        if (::connect(*socketPtr, address_p, sizeof(address)) >= 0)
            break;

        assert(socketPtr);
        closesocket(*static_cast<int*>(socketPtr));

        int sleepDuration = tries * .25 + 1;

        INFO(__FILE__, __LINE__) << boost::format("    Could not connect to the logWindow server: %1% -- retry in %2% seconds. \n") % strerror(sock_errno()) % sleepDuration << std::flush;

        std::this_thread::sleep_for(std::chrono::seconds(sleepDuration));
    }

    if(tries == 11)
        throw omnetpp::cRuntimeError("Could not connect to the logWindow server after 10 retries!");

    // TCP_NODELAY: disable the Nagle algorithm. This means that segments are always
    // sent as soon as possible, even if there is only a small amount of data.
    // When not set, data is buffered until there is a sufficient amount to send out,
    // thereby avoiding the frequent sending of small packets, which results
    // in poor utilization of the network.
    int x = 1;
    ::setsockopt(*socketPtr, IPPROTO_TCP, TCP_NODELAY, (const char*) &x, sizeof(x));
}


void vglog::sendToLogWindow(std::string msg)
{
    // protect socket from the destructor
    std::lock_guard<std::mutex> lock(lock_socket);

    // if socketPtr is closed in the destructor, then WARN the user instead of throwing error
    if(!socketPtr)
    {
        std::cout << "WARNING: Cannot send the message to GLOG window: socket to window is closed. \n" << std::flush;
        return;
    }

    try
    {
        // sending the msg size first
        uint32_t dataLength = htonl(msg.size());
        int n = ::send(*socketPtr, &dataLength, sizeof(uint32_t), MSG_NOSIGNAL);
        if (n < 0)
            throw std::runtime_error("ERROR sending msg size to socket");

        // then sending the msg itself
        n = ::send(*socketPtr, msg.c_str(), msg.size(), MSG_NOSIGNAL);
        if (n < 0)
            throw std::runtime_error("ERROR sending msg to socket");

        // wait for response from logWindow
        struct pollfd poll[1];
        poll[0].fd = *socketPtr;
        poll[0].events = POLLIN | POLLPRI;
        int rv = ::poll(poll, 1, 1000);

        if (rv == -1)
            throw std::runtime_error("ERROR reading response from socket");
        else if (rv == 0)
            throw std::runtime_error("No response from the logWindow after 1 second! Is logWindow closed?");
        else // we receive data from the TCP server
        {
            char rx_buffer[100];
            bzero(rx_buffer, 100);

            n = ::recv(*socketPtr, rx_buffer, 99, MSG_NOSIGNAL);
            if (n < 0)
                throw std::runtime_error("ERROR reading response from socket");
            else if (n == 0)
                throw omnetpp::cRuntimeError("No response from the logWindow! Is logWindow closed?");

            std::string res = rx_buffer;
            if(res != "ok!")
                throw omnetpp::cRuntimeError("Command execution error in logWindow: %s", res.c_str());
        }
    }
    catch(const std::runtime_error& ex)
    {
        // ignore
        // LOG_INFO << std::endl << ex.what() << std::endl << std::flush;
        return;
    }
}

}
