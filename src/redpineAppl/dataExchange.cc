/****************************************************************************/
/// @file    dataExchange.cc
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author  second author name
/// @date    Jun 2016
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

#include <stdexcept>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <exception>

#include <thread>
#include <fstream>

#include "dataExchange.h"
#include "ApplV_Manager.h"
#include "vlog.h"

namespace VENTOS {

Define_Module(VENTOS::dataExchange);

dataExchange::~dataExchange()
{

}


void dataExchange::initialize(int stage)
{
    super::initialize(stage);

    if(stage ==0)
    {
        active = par("active").boolValue();
        if(!active)
            return;

        VENTOS_serverPort = par("VENTOS_serverPort").longValue();
        if(VENTOS_serverPort < 0)
            throw omnetpp::cRuntimeError("VENTOS_serverPort number is invalid!");

        numBacklog = par("numBacklog").longValue();
        if(numBacklog < 0)
            throw omnetpp::cRuntimeError("numBacklog should be >= 0");

        // get a pointer to the TraCI module
        cModule *module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("TraCI");
        ASSERT(module);
        TraCI = static_cast<TraCI_Commands *>(module);
        ASSERT(TraCI);

        Signal_initialize_withTraCI = registerSignal("initialize_withTraCI");
        omnetpp::getSimulation()->getSystemModule()->subscribe("initialize_withTraCI", this);

        Signal_executeEachTS = registerSignal("executeEachTS");
        omnetpp::getSimulation()->getSystemModule()->subscribe("executeEachTS", this);
    }
}


void dataExchange::finish()
{
    // close the welcoming socket
    if(sockfd)
        ::close(sockfd);

    // close all sockets opened for incoming connections
    for(auto i : connections)
        for(auto j : i.second)
            ::close(j.incommingSocket);
}


void dataExchange::handleMessage(omnetpp::cMessage *msg)
{

}


void dataExchange::receiveSignal(omnetpp::cComponent *source, omnetpp::simsignal_t signalID, long i, cObject* details)
{
    if(!active)
        return;

    Enter_Method_Silent();

    if(signalID == Signal_initialize_withTraCI)
    {
        initialize_withTraCI();
    }
    else if(signalID == Signal_executeEachTS)
    {
        executeEachTimestep();
    }
}


void dataExchange::initialize_withTraCI()
{

}


void dataExchange::executeEachTimestep()
{
    // run this code only once
    static bool wasExecuted = false;
    if (active && !wasExecuted)
    {
        // run TCP thread in a thread
        std::thread thd(&dataExchange::start_TCP_server, this);
        thd.detach();

        wasExecuted = true;
    }

    {
        std::lock_guard<std::mutex> lock(lock_vector);

        while(!receivedData.empty())
        {
            auto &frame = receivedData.back();

            // check if IP address corresponds to an emulated vehicle
            std::string vehID = TraCI->ip2vehicleId(frame->ipv4);
            if(vehID == "")
            {
                LOG_WARNING << boost::format("\nWARNING: Emulated vehicle for OBU %1% does not exist in the simulation! \n") % frame->ipv4;
                receivedData.pop_back(); // remove it from the vector
                continue;
            }

            // get a pointer to the vehicle
            cModule *module = omnetpp::getSimulation()->getSystemModule()->getModuleByPath(vehID.c_str());
            ASSERT(module);
            module = module->getSubmodule("appl");
            ASSERT(module);
            ApplVManager *vehPtr = static_cast<ApplVManager *>(module);
            ASSERT(vehPtr);

            // should we print the received data?
            if(module->par("print_HIL2Sim"))
            {
                std::string SUMOid = TraCI->omnet2traciId(vehID);
                LOG_EVENT << boost::format("\n%1% (%2%) received the following msg of size %3% from %4%: \n") % SUMOid % vehID % frame->bufferLength % frame->ipv4;
                print_dataPayload(frame->buffer, frame->bufferLength);
                LOG_FLUSH;
            }

            // send the payload to the vehicle
            vehPtr->receiveDataFromOBU(frame);

            // remove it from the vector
            receivedData.pop_back();
        }
    }
}


// start a local TCP server -- to receive data from all OBUs
void dataExchange::start_TCP_server()
{
    // create a socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        throw omnetpp::cRuntimeError("ERROR opening socket");

    // the purpose of SO_REUSEADDR/SO_REUSEPORT is to allow to reuse the port even if the process crash or been killed
    int optval = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));

    // clear address structure
    struct sockaddr_in serv_addr;
    bzero((char *) &serv_addr, sizeof(serv_addr));

    /* setup the host_addr structure for use in bind call */
    // server byte order
    serv_addr.sin_family = AF_INET;

    // automatically be filled with current host's IP address
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    // convert short integer value for port must be converted into network byte order
    serv_addr.sin_port = htons(VENTOS_serverPort);

    // this bind() call will bind  the socket to the current IP address on port, portno
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        throw omnetpp::cRuntimeError("ERROR on binding");

    // this listen() call tells the socket to listen to the incoming connections.
    listen(sockfd, numBacklog);

    struct sockaddr_in cli_addr;
    socklen_t clilen = sizeof(cli_addr);

    try
    {
        while(true)
        {
            // The accept() returns a new socket file descriptor for the accepted connection.
            // So, the original socket file descriptor can continue to be used
            // for accepting new connections while the new socker file descriptor is used for
            // communicating with the connected client.
            int clientRecvSock = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
            if (clientRecvSock < 0)
                throw std::runtime_error("ERROR on accept");

            std::string ipv4 = inet_ntoa(cli_addr.sin_addr);
            uint16_t remote_port = ntohs(cli_addr.sin_port);
            LOG_INFO << boost::format("\n>>> Incoming connection from board %1% at port %2% \n") % ipv4 % remote_port << std::flush;

            // get all necessary parameters from the incoming connections
            recvParams(clientRecvSock, ipv4, remote_port);

            // create a new thread specifically for this connection
            std::thread client_thd(&dataExchange::recvDataFromOBU, this, clientRecvSock, ipv4, remote_port);
            client_thd.detach();
        }
    }
    catch(const std::exception& ex)
    {
        // silently ignore the exceptions
        // LOG_INFO << std::endl << ex.what() << std::endl << std::flush;
        return;
    }
}


void dataExchange::recvParams(int clientRecvSock, std::string ipv4, uint16_t remote_port)
{
    std::string role = "";
    std::string name = "";
    int remote_TCP_server_port = -1;

    try
    {
        for(int RecvData = 1; RecvData <= 3; RecvData++)
        {
            // receive the message length
            uint32_t rcvDataLength = 0;
            int n = ::recv(clientRecvSock, &rcvDataLength, sizeof(uint32_t), MSG_NOSIGNAL);
            if (n < 0)
                throw std::runtime_error("ERROR reading msg size from socket");
            else if(n == 0)
                break;

            rcvDataLength = ntohl(rcvDataLength);

            unsigned char rx_buffer[rcvDataLength+1];
            bzero(rx_buffer, rcvDataLength+1);

            n = ::recv(clientRecvSock, &rx_buffer, rcvDataLength, MSG_NOSIGNAL);
            if (n < 0)
                throw std::runtime_error("ERROR reading msg from socket");
            else if(n == 0)
                break;

            // the first received data is 'role' (driver, application)
            if(RecvData == 1)
                role = reinterpret_cast<char*>(rx_buffer);
            // the second received data is 'name'
            else if(RecvData == 2)
                name = reinterpret_cast<char*>(rx_buffer);
            // the third received data is 'TCP port #' that OBU is listening to
            else if(RecvData == 3)
                remote_TCP_server_port = std::stoi(reinterpret_cast<char*>(rx_buffer));
        }
    }
    catch(const std::exception& ex)
    {
        LOG_INFO << std::endl << ex.what() << std::endl << std::flush;
        throw omnetpp::cRuntimeError("ERROR in receiving parameters from incoming connection %s:%d", ipv4.c_str(), remote_port);
    }

    LOG_INFO << boost::format("    role: %1%, name: %2%, remote TCP server port: %3% \n") % role % name % remote_TCP_server_port << std::flush;

    if(role != "driver" && role != "application")
        throw omnetpp::cRuntimeError("Unknown role name '%s'", role.c_str());

    {
        // protect simultaneous access to connections map
        std::lock_guard<std::mutex> lock(lock_map);

        // save the new incoming connection
        ConnEntry *entry = new ConnEntry(role, name, remote_TCP_server_port, clientRecvSock, -1);
        auto it = connections.find(ipv4);
        if(it == connections.end())
        {
            std::vector<ConnEntry> vec = {*entry};
            connections[ipv4] = vec;
        }
        else
        {
            // make sure the 'name' is unique in this OBU
            for(auto &itt : it->second)
                if(itt.name == name)
                    throw omnetpp::cRuntimeError("Application name '%s' already exists on OBU %s", name.c_str(), ipv4.c_str());

            it->second.push_back(*entry);
        }

        if(role == "application")
        {
            auto it = connections.find(ipv4);
            if(it == connections.end())
                throw std::runtime_error("IP address does not exist in connections map");

            // Warn the user if the driver has not made any incoming connection so far
            bool found = false;
            for(auto &kk : it->second)
                if(kk.role == "driver") found = true;
            if(!found)
                LOG_WARNING << "\n>>> No incoming connection has been made from the driver! \n" << std::flush;

            // connect to the TCP server on OBU
            std::thread server_thd(&dataExchange::connect_to_TCP_server, this, ipv4, remote_TCP_server_port, name);
            server_thd.detach();
        }
    }  // end of lock_map protection
}


// if OBU #x is running two applications, then three threads are
// assigned to receive data from OBU #x
// thread 1: driver
// thread 2: application 1
// thread 3: application 2
void dataExchange::recvDataFromOBU(int clientRecvSock, std::string ipv4, uint16_t remote_port)
{
    try
    {
        while(true)
        {
            // receive the message length
            uint32_t rcvDataLength = 0;
            int n = ::recv(clientRecvSock, &rcvDataLength, sizeof(uint32_t), MSG_NOSIGNAL);
            if (n < 0)
                throw std::runtime_error("ERROR reading msg size from socket");
            else if(n == 0)
                break;

            rcvDataLength = ntohl(rcvDataLength);

            unsigned char rx_buffer[rcvDataLength+1];
            bzero(rx_buffer, rcvDataLength+1);

            n = ::recv(clientRecvSock, &rx_buffer, rcvDataLength, MSG_NOSIGNAL);
            if (n < 0)
                throw std::runtime_error("ERROR reading msg from socket");
            else if(n == 0)
                break;

            {
                std::lock_guard<std::mutex> lock(lock_vector);

                // save the received data
                dataEntry *entry = new dataEntry(rx_buffer, rcvDataLength, ipv4, remote_port);
                receivedData.push_back(entry);
            }
        }

        // we are done
        ::close(clientRecvSock);
    }
    catch(const std::exception& ex)
    {
        LOG_INFO << std::endl << ex.what() << std::endl << std::flush;
        return;
    }
}


// connecting to the remote TCP server running on OBU
// each application on an OBU is running a TCP server
void dataExchange::connect_to_TCP_server(std::string ipv4, int port, std::string name)
{
    LOG_INFO << boost::format("\n>>> Connecting to TCP server (%1%) on board %2% at port %3% \n") % name % ipv4 % port << std::flush;

    if (initsocketlibonce() != 0)
        throw omnetpp::cRuntimeError("Could not init socketlib");

    in_addr addr;
    struct hostent* host_ent;

    if ( (host_ent = gethostbyname(ipv4.c_str())) )
        addr = *((struct in_addr*) host_ent->h_addr_list[0]);
    else
        throw omnetpp::cRuntimeError("Invalid TCP server address");

    sockaddr_in address;
    sockaddr* address_p = (sockaddr*)&address;
    memset(address_p, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = addr.s_addr;

    int* socketPtr = new int();
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

        LOG_INFO << boost::format("    Could not connect to the TCP server: %1% -- retry in %2% seconds. \n") % strerror(sock_errno()) % sleepDuration << std::flush;

        std::this_thread::sleep_for(std::chrono::seconds(sleepDuration));
    }

    if(tries == 11)
        throw omnetpp::cRuntimeError("Could not connect to the TCP server after 10 retries!");

    // TCP_NODELAY: disable the Nagle algorithm. This means that segments are always
    // sent as soon as possible, even if there is only a small amount of data.
    // When not set, data is buffered until there is a sufficient amount to send out,
    // thereby avoiding the frequent sending of small packets, which results
    // in poor utilization of the network.
    int x = 1;
    ::setsockopt(*socketPtr, IPPROTO_TCP, TCP_NODELAY, (const char*) &x, sizeof(x));

    {
        // protect simultaneous access to connections map
        std::lock_guard<std::mutex> lock(lock_map);

        // update outgoingSocket in the connections map
        auto it = connections.find(ipv4);
        if(it == connections.end())
            throw omnetpp::cRuntimeError("No incoming connections from OBU %s", ipv4.c_str());
        // iterate over all incoming connections from this OBU
        bool found = false;
        for(auto &ii : it->second)
        {
            // look for the entry corresponding to incoming connection
            if(ii.port == port)
            {
                ii.outgoingSocket = *socketPtr;
                found = true;
                break;
            }
        }
        if(!found)
            throw omnetpp::cRuntimeError("Could not find the incoming connection for port %d", port);
    }
}


void dataExchange::sendDataToBoard(std::string SUMOid, unsigned char *data, unsigned int data_length)
{
    // check if IP address corresponds to a HIL vehicle
    std::string ipv4 = TraCI->vehicleId2ip(SUMOid);
    if(ipv4 == "")
        throw omnetpp::cRuntimeError("Vehicle id does not match any of the HIL vehicles!");

    // get a pointer to the vehicle
    std::string omnetId = TraCI->traci2omnetId(SUMOid);
    cModule *module = omnetpp::getSimulation()->getSystemModule()->getModuleByPath(omnetId.c_str());
    ASSERT(module);
    module = module->getSubmodule("appl");
    ASSERT(module);

    // should we print the sent data?
    if(module->par("print_Sim2HIL"))
    {
        LOG_EVENT << boost::format("\nThe following msg of size %1% is sent to %2% (%3%): \n") % data_length % ipv4 % SUMOid;
        print_dataPayload(data, data_length);
        LOG_FLUSH;
    }

    auto ii = connections.find(ipv4);
    if(ii == connections.end())
        throw omnetpp::cRuntimeError("No connection exists to host %s", ipv4.c_str());

    // the data is sent to all TCP servers (application 1, application 2, etc.) on the remote OBU
    std::vector<std::thread> workers;
    for(auto &conn : ii->second)
    {
        if(conn.role == "application" && conn.outgoingSocket != -1)
        {
            workers.push_back(std::thread([=]() {  // pass by value

                try
                {
                    // sending the msg size first
                    uint32_t dataLength = htonl(data_length);
                    int n = ::send(conn.outgoingSocket, &dataLength, sizeof(uint32_t), MSG_NOSIGNAL);
                    if (n < 0)
                        throw std::runtime_error("ERROR sending msg size to socket");

                    // then sending the msg itself
                    n = ::send(conn.outgoingSocket, data, data_length, MSG_NOSIGNAL);
                    if (n < 0)
                        throw std::runtime_error("ERROR sending msg to socket");
                }
                catch(const std::exception& ex)
                {
                    // ignore error message
                    // LOG_INFO << std::endl << ex.what() << std::endl << std::flush;
                    return;
                }

            }));
        }
    }

    if(workers.empty())
        throw omnetpp::cRuntimeError("No outgoing socket found to host %s", ipv4.c_str());

    // wait for all threads to finish
    std::for_each(workers.begin(), workers.end(), [](std::thread &t) {
        t.join();
    });
}


/* print packet payload data (avoid printing binary data) */
void dataExchange::print_dataPayload(const u_char *payload, int len)
{
    // double-check again
    if (len <= 0)
        return;

    int line_width = 16;   /* number of bytes per line */
    int offset = 0;        /* zero-based offset counter */
    const u_char *ch = payload;

    /* data fits on one line */
    if (len <= line_width)
    {
        print_hex_ascii_line(ch, len, offset);
        return;
    }

    int len_rem = len;
    int line_len;

    /* data spans multiple lines */
    for ( ;; )
    {
        /* compute current line length */
        line_len = line_width % len_rem;

        /* print line */
        print_hex_ascii_line(ch, line_len, offset);

        /* compute total remaining */
        len_rem = len_rem - line_len;

        /* shift pointer to remaining bytes to print */
        ch = ch + line_len;

        /* add offset */
        offset = offset + line_width;

        /* check if we have line width chars or less */
        if (len_rem <= line_width)
        {
            /* print last line and get out */
            print_hex_ascii_line(ch, len_rem, offset);
            break;
        }
    }
}


/*
 * print data in rows of 16 bytes: offset   hex   ascii
 *
 * 00000   47 45 54 20 2f 20 48 54  54 50 2f 31 2e 31 0d 0a   GET / HTTP/1.1..
 */
void dataExchange::print_hex_ascii_line(const u_char *payload, int len, int offset)
{
    int i;
    int gap;
    const u_char *ch;

    /* offset */
    printf("            %05d   ", offset);

    /* hex */
    ch = payload;
    for(i = 0; i < len; i++)
    {
        printf("%02x ", *ch);
        ch++;
        /* print extra space after 8th byte for visual aid */
        if (i == 7)
            printf(" ");
    }

    /* print space to handle line less than 8 bytes */
    if (len < 8)
        printf(" ");

    /* fill hex gap with spaces if not full line */
    if (len < 16)
    {
        gap = 16 - len;
        for (i = 0; i < gap; i++)
        {
            printf("   ");
        }
    }

    printf("   ");

    /* ascii (if printable) */
    ch = payload;
    for(i = 0; i < len; i++)
    {
        if (isprint(*ch))
            printf("%c", *ch);
        else
            printf(".");
        ch++;
    }

    printf("\n");
}

}
