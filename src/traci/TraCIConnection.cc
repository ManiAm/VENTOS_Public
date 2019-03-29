/****************************************************************************/
/// @file    TraCIConnection.cc
/// @author  Mani Amoozadeh   <maniam@ucdavis.edu>
/// @author  Christoph Sommer <mail@christoph-sommer.de>
/// @date    August 2013
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
#include <functional>
#include <thread>
#include <signal.h>

#include "traci/TraCIConnection.h"
#include "traci/TraCIConstants.h"
#include "traci/TraCICommands.h"
#include "logging/VENTOS_logging.h"
#include "global/utility.h"

namespace VENTOS {

pid_t TraCIConnection::child_pid = -1;
void* TraCIConnection::socketPtr = NULL;
std::mutex TraCIConnection::lock_TraCI;

SOCKET socket(void* ptr)
{
    ASSERT(ptr);
    return *static_cast<SOCKET*>(ptr);
}


TraCIConnection::TraCIConnection(void* ptr)
{
    this->socketPtr = ptr;
    ASSERT(socketPtr);
}


TraCIConnection::~TraCIConnection()
{
    if (socketPtr)
    {
        closesocket(socket(socketPtr));
        delete static_cast<SOCKET*>(socketPtr);
        socketPtr = NULL;
    }

    // Note: do not kill the SUMO process.
    // Closing the TraCI connection will automatically terminates SUMO

    // #if defined(_WIN32) || defined(__WIN32__) || defined(WIN32) || defined(__CYGWIN__) || defined(_WIN64)
    // #else
    //     // send SIGINT
    //     if (child_pid > 0)
    //         kill(child_pid, 15);
    // #endif
}


void TraCIConnection::startSUMO(std::string SUMOapplication, std::string SUMOconfig, std::string SUMOcommandLine, int port)
{
    // assemble command line options
    std::ostringstream fullOptions;
    fullOptions << " --remote-port " << port
            << " --configuration-file " << SUMOconfig
            << (" " + SUMOcommandLine);

    LOG_DEBUG << "\n>>> Starting SUMO process ... \n";
    LOG_DEBUG << boost::format("    Executable file: %1% \n") % SUMOapplication;
    LOG_DEBUG << boost::format("    Config file: %1% \n") % SUMOconfig;
    LOG_DEBUG << boost::format("    Switches: %1% \n") % SUMOcommandLine;
    LOG_DEBUG << boost::format("    TraCI server is listening on port %1% \n") % port;
    LOG_FLUSH;

    // assemble full command
    std::ostringstream fullCommand;
    fullCommand << SUMOapplication << fullOptions.str();

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32) || defined(__CYGWIN__) || defined(_WIN64)
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFOA);

    ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

    char cmdline[32768];
    strncpy(cmdline, fullCommand.c_str(), sizeof(cmdline));
    bool bSuccess = CreateProcess(0, cmdline, 0, 0, 1, NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE, 0, 0, &si, &pi);
    if (!bSuccess)
    {
        std::string msg = "undefined error";

        DWORD errorMessageID = ::GetLastError();
        if(errorMessageID != 0)
        {

            LPSTR messageBuffer = nullptr;
            size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                    NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

            std::string message(messageBuffer, size);
            LocalFree(messageBuffer);
            msg = message;
        }

        msg = std::string() + "Error launching TraCI server (\"" + fullCommand + "\"): " + msg + ". Make sure you have set $PATH correctly.";

        throw cRuntimeError(msg.c_str());
    }
    else
    {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }

#else

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

        // run SUMO server inside this child process
        // if execution is successful then child will be blocked at this line
        int r = system(fullCommand.str().c_str());

        if (r == -1)
            throw omnetpp::cRuntimeError("Running \"%s\" failed during system()", fullCommand.str().c_str());

        if (WEXITSTATUS(r) != 0)
        {
            // get a pointer to the TraCI module
            auto TraCI = TraCI_Commands::getTraCI();

            std::string sumoAppl = TraCI->par("SUMOapplication").stringValue();

            // stop the simulation
            throw omnetpp::cRuntimeError("'%s' application terminated with exit code '%d'. Check %s's output log for more information.",
                    sumoAppl.c_str(),
                    WEXITSTATUS(r),
                    sumoAppl.c_str());
        }

        exit(1);
    }
    else
    {
        LOG_DEBUG << boost::format("    SUMO has started successfully in process %1%  \n") % child_pid << std::flush;

        // show SUMO version
        LOG_DEBUG << boost::format("    %1%  \n") % getSUMOversion(SUMOapplication) << std::flush;
    }

#endif
}


// get a random Ephemeral port from the system
int TraCIConnection::getFreeEphemeralPort()
{
    if (initsocketlibonce() != 0)
        throw omnetpp::cRuntimeError("Could not init socketlib");

    SOCKET sock = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
        throw omnetpp::cRuntimeError("Failed to create socket: %s", strerror(errno));

    struct sockaddr_in serv_addr;
    struct sockaddr* serv_addr_p = (struct sockaddr*)&serv_addr;
    memset(serv_addr_p, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = 0;   // get a random Ephemeral port

    if (::bind(sock, serv_addr_p, sizeof(serv_addr)) < 0)
        throw omnetpp::cRuntimeError("Failed to bind socket: %s", strerror(errno));

    socklen_t len = sizeof(serv_addr);
    if (getsockname(sock, serv_addr_p, &len) < 0)
        throw omnetpp::cRuntimeError("Failed to get hostname: %s", strerror(errno));

    int port = ntohs(serv_addr.sin_port);

    closesocket(sock);

    return port;
}


TraCIConnection* TraCIConnection::connect(const char* host, int port)
{
    if(socketPtr)
        throw omnetpp::cRuntimeError("There is already an active connection to SUMO! Why calling 'connect' twice ?!");

    if (initsocketlibonce() != 0)
        throw omnetpp::cRuntimeError("Could not init socketlib");

    sockaddr_in address;
    sockaddr* address_p = (sockaddr*)&address;
    memset(address_p, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = utility::getIPv4ByHostName(host).ipv4_n;

    SOCKET* socketPtr = new SOCKET();
    if (*socketPtr < 0)
        throw omnetpp::cRuntimeError("Could not create socket to connect to TraCI server");

    // wait for 1 second and then try connecting to TraCI server
    std::this_thread::sleep_for(std::chrono::seconds(1));

    LOG_DEBUG << boost::format("\n>>> Connecting to TraCI server on port %1% ... \n") % port << std::flush;

    const int MAX_RETRY = 10;
    for (int tries = 1; ; ++tries)
    {
        *socketPtr = ::socket(AF_INET, SOCK_STREAM, 0);
        if (::connect(*socketPtr, address_p, sizeof(address)) >= 0)
            break;

        closesocket(socket(socketPtr));

        int sleepDuration = tries * .25 + 1;

        LOG_ERROR << boost::format("    Could not connect to the TraCI server: %1% -- retry %2%/%3% in %4% seconds. \n") % strerror(sock_errno()) % tries % MAX_RETRY % sleepDuration << std::flush;

        if(tries == MAX_RETRY)
            throw omnetpp::cRuntimeError("Could not connect to TraCI server after %d retries!", MAX_RETRY);

        std::this_thread::sleep_for(std::chrono::seconds(sleepDuration));
    }

    // TCP_NODELAY: disable the Nagle algorithm. This means that segments are always
    // sent as soon as possible, even if there is only a small amount of data.
    // When not set, data is buffered until there is a sufficient amount to send out,
    // thereby avoiding the frequent sending of small packets, which results
    // in poor utilization of the network.
    int x = 1;
    ::setsockopt(*socketPtr, IPPROTO_TCP, TCP_NODELAY, (const char*) &x, sizeof(x));

    return new TraCIConnection(socketPtr);
}


TraCIBuffer TraCIConnection::query(uint8_t commandGroupId, const TraCIBuffer& buf)
{
    // protect simultaneous access to TraCI
    std::lock_guard<std::mutex> lock(lock_TraCI);

    sendMessage(makeTraCICommand(commandGroupId, buf));

    TraCIBuffer obuf(receiveMessage());
    uint8_t cmdLength; obuf >> cmdLength;
    uint8_t commandResp; obuf >> commandResp;
    ASSERT(commandResp == commandGroupId);
    uint8_t result; obuf >> result;
    std::string description; obuf >> description;

    if (result == RTYPE_NOTIMPLEMENTED)
        throw omnetpp::cRuntimeError("TraCI server reported command 0x%2x not implemented (\"%s\"). Might need newer version.", commandGroupId, description.c_str());

    if (result == RTYPE_ERR)
        throw omnetpp::cRuntimeError("TraCI server reported throw cRuntimeError executing command 0x%2x (\"%s\").", commandGroupId, description.c_str());

    ASSERT(result == RTYPE_OK);

    return obuf;
}


std::string TraCIConnection::receiveMessage()
{
    if (!socketPtr)
        throw std::runtime_error("Cannot receive command: TraCI is disconnected");

    uint32_t msgLength = 0;

    {
        char buf2[sizeof(uint32_t)];
        uint32_t bytesRead = 0;
        while (bytesRead < sizeof(uint32_t))
        {
            int receivedBytes = ::recv(socket(socketPtr), reinterpret_cast<char*>(&buf2) + bytesRead, sizeof(uint32_t) - bytesRead, MSG_NOSIGNAL);
            if (receivedBytes > 0)
                bytesRead += receivedBytes;
            else if (receivedBytes == 0)
                terminateSimulationOnError("ERROR in receiveMessage");
            else
            {
                if (sock_errno() == EINTR) continue;
                if (sock_errno() == EAGAIN) continue;

                terminateSimulationOnError("ERROR in receiveMessage");
            }
        }

        TraCIBuffer(std::string(buf2, sizeof(uint32_t))) >> msgLength;
    }

    uint32_t bufLength = msgLength - sizeof(msgLength);
    char buf[bufLength];

    {
        uint32_t bytesRead = 0;
        while (bytesRead < bufLength)
        {
            int receivedBytes = ::recv(socket(socketPtr), reinterpret_cast<char*>(&buf) + bytesRead, bufLength - bytesRead, MSG_NOSIGNAL);
            if (receivedBytes > 0)
                bytesRead += receivedBytes;
            else if (receivedBytes == 0)
                terminateSimulationOnError("ERROR in receiveMessage");
            else
            {
                if (sock_errno() == EINTR) continue;
                if (sock_errno() == EAGAIN) continue;

                terminateSimulationOnError("ERROR in receiveMessage");
            }
        }
    }

    return std::string(buf, bufLength);
}


void TraCIConnection::sendMessage(std::string buf)
{
    if (!socketPtr)
        throw std::runtime_error("Cannot send command: TraCI is disconnected");

    {
        uint32_t msgLength = sizeof(uint32_t) + buf.length();
        TraCIBuffer buf2 = TraCIBuffer();
        buf2 << msgLength;
        uint32_t bytesWritten = 0;
        while (bytesWritten < sizeof(uint32_t))
        {
            size_t sentBytes = ::send(socket(socketPtr), buf2.str().c_str() + bytesWritten, sizeof(uint32_t) - bytesWritten, MSG_NOSIGNAL);
            if (sentBytes > 0)
                bytesWritten += sentBytes;
            else
            {
                if (sock_errno() == EINTR) continue;
                if (sock_errno() == EAGAIN) continue;

                terminateSimulationOnError("ERROR in sendMessage");
            }
        }
    }

    {
        uint32_t bytesWritten = 0;
        while (bytesWritten < buf.length())
        {
            size_t sentBytes = ::send(socket(socketPtr), buf.c_str() + bytesWritten, buf.length() - bytesWritten, MSG_NOSIGNAL);
            if (sentBytes > 0)
                bytesWritten += sentBytes;
            else
            {
                if (sock_errno() == EINTR) continue;
                if (sock_errno() == EAGAIN) continue;

                terminateSimulationOnError("ERROR in sendMessage");
            }
        }
    }
}


std::string makeTraCICommand(uint8_t commandId, const TraCIBuffer& buf)
{
    if (sizeof(uint8_t) + sizeof(uint8_t) + buf.str().length() > 0xFF)
    {
        uint32_t len = sizeof(uint8_t) + sizeof(uint32_t) + sizeof(uint8_t) + buf.str().length();
        return (TraCIBuffer() << static_cast<uint8_t>(0) << len << commandId).str() + buf.str();
    }

    uint8_t len = sizeof(uint8_t) + sizeof(uint8_t) + buf.str().length();
    return (TraCIBuffer() << len << commandId).str() + buf.str();
}


void TraCIConnection::terminateSimulationOnError(std::string err)
{
    err = "\n" + err + ": Connection to TraCI server closed unexpectedly. \n\n";
    LOG_ERROR << err << std::flush;

    // get a pointer to the TraCI module
    auto TraCI = TraCI_Commands::getTraCI();

    // end the simulation
    TraCI->simulationTerminate(true /*error?*/);
}


std::string TraCIConnection::getSUMOversion(std::string path)
{
    // get the local version
    char command[100];
    sprintf(command, "%s -V", path.c_str());

    FILE* pipe = popen(command, "r");
    if (!pipe)
        throw omnetpp::cRuntimeError("ERROR: can not open pipe");

    char buffer[128];
    std::string result = "";
    while(!feof(pipe))
    {
        if(fgets(buffer, 128, pipe) != NULL)
            result += buffer;
    }
    pclose(pipe);

    if(result == "")
        throw omnetpp::cRuntimeError("ERROR: pipe output is empty!");

    // get the first line
    std::stringstream ss(result);
    std::string to = "";
    while( std::getline(ss, to) )
    {
        break;
    }

    if(to == "")
        throw omnetpp::cRuntimeError("ERROR: first line of output is empty!");

    return to;
}

}
