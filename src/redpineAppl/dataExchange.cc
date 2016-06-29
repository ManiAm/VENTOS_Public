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

#include "vlog.h"
#include "dataExchange.h"
#include "ApplV_Manager.h"
#include "RedpineData_m.h"


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

        board_serverPort = par("board_serverPort").longValue();
        if(board_serverPort < 0)
            throw omnetpp::cRuntimeError("board_serverPort number is invalid!");

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

    for(auto i : connections_fromBoard)
        ::close(i);
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

            // check if IP address corresponds to a HIL vehicle
            std::string vehID = TraCI->ip2vehicleId(frame.ipv4);
            if(vehID != "")
            {
                // get a pointer to the vehicle
                cModule *module = omnetpp::getSimulation()->getSystemModule()->getModuleByPath(vehID.c_str());
                ASSERT(module);
                module = module->getSubmodule("appl");
                ASSERT(module);
                ApplVManager *vehPtr = static_cast<ApplVManager *>(module);
                ASSERT(vehPtr);

                // preparing a message
                redpineData* data = new redpineData("redpineData");
                data->setSrcPort(frame.port);
                data->setDataArraySize(frame.bufferLength);
                for (unsigned int ii = 0; ii < frame.bufferLength; ii++)
                    data->setData(ii, frame.buffer[ii]);

                // send the payload to the vehicle
                vehPtr->receiveDataFromBoard(data);
            }

            free(frame.buffer);
            receivedData.pop_back();
        }
    }
}


// start a local TCP server -- to receive data from boards
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
            LOG_INFO << ">>> (dataExchange) incoming connection from board " << ipv4 << " at port " << remote_port << ". \n" << std::flush;

            // save the socket fd
            connections_fromBoard.push_back(clientRecvSock);

            // create a new thread specifically for this connection
            std::thread client_thd(&dataExchange::recvDataFromBoard, this, clientRecvSock, ipv4, remote_port);
            client_thd.detach();

            // connect to the remote TCP server on this board
            std::thread server_thd(&dataExchange::connect_to_TCP_server, this, ipv4);
            server_thd.detach();
        }
    }
    catch(const std::exception& ex)
    {
        LOG_INFO << std::endl << ex.what() << std::endl << std::flush;
        return;
    }
}


void dataExchange::recvDataFromBoard(int clientRecvSock, std::string ipv4, uint16_t remote_port)
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
                receivedData.push_back(*entry);
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


// connecting to the remote TCP server on the board -- to send data to the board
void dataExchange::connect_to_TCP_server(std::string ipv4)
{
    LOG_INFO << ">>> (dataExchange) connecting to TCP server on board " << ipv4 << " at port " << board_serverPort << ". \n" << std::flush;

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
    address.sin_port = htons(board_serverPort);
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

        std::cout << boost::format("    Could not connect to the TCP server: %1% -- retry in %2% seconds. \n") % strerror(sock_errno()) % sleepDuration;
        std::cout.flush();

        std::this_thread::sleep_for(std::chrono::seconds(sleepDuration));
    }

    if(tries == 11)
        throw omnetpp::cRuntimeError("Could not connect to the TCP server after 10 retries!");

    // disable the 'Nagle' buffering algorithm. It should only be set for applications
    // that send frequent small bursts of information without getting an immediate
    // response, where timely delivery of data is required
    int x = 1;
    ::setsockopt(*socketPtr, IPPROTO_TCP, TCP_NODELAY, (const char*) &x, sizeof(x));

    // save <ipv4,socket> pair
    connections_fromVENTOS[ipv4] = socketPtr;
}


void dataExchange::sendDataToBoard(std::string id, unsigned char *data, unsigned int data_length)
{
    // check if IP address corresponds to a HIL vehicle
    std::string ipv4 = TraCI->vehicleId2ip(id);
    if(ipv4 == "")
        throw omnetpp::cRuntimeError("Vehicle id does not match any of the HIL vehicles!");

    // check for the corresponding socket
    auto conn = connections_fromVENTOS.find(ipv4);
    if(conn == connections_fromVENTOS.end())
        throw omnetpp::cRuntimeError("No connections exists to host %s", ipv4.c_str());

    try
    {
        // sending the msg size first
        uint32_t dataLength = htonl(data_length);
        int n = ::send(*(conn->second), &dataLength, sizeof(uint32_t), MSG_NOSIGNAL);
        if (n < 0)
            throw std::runtime_error("ERROR sending msg size to socket");

        // then sending the msg itself
        n = ::send(*(conn->second), data, data_length, MSG_NOSIGNAL);
        if (n < 0)
            throw std::runtime_error("ERROR sending msg to socket");
    }
    catch(const std::exception& ex)
    {
        LOG_INFO << std::endl << ex.what() << std::endl << std::flush;
        return;
    }
}

}
