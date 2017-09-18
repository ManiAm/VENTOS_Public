/****************************************************************************/
/// @file    testTCP.cc
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
#include "sockets.h"

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32) || defined(__CYGWIN__) || defined(_WIN64)
#include <ws2tcpip.h>
#else
#include <netinet/tcp.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif

#include <string.h>
#include <thread>
#include <cassert>
#include <iostream>
#include <boost/format.hpp>

SOCKET socket(void* ptr)
{
    assert(ptr);
    return *static_cast<SOCKET*>(ptr);
}

int main(int argc, char* argv[])
{
    if (initsocketlibonce() != 0)
        throw std::runtime_error("Could not init socketlib");

    in_addr addr;
    struct hostent* host_ent;
    struct in_addr saddr;

    if ((host_ent = gethostbyname("localhost")))
        addr = *((struct in_addr*) host_ent->h_addr_list[0]);
    else
        throw std::runtime_error("Invalid TraCI server address");

    sockaddr_in address;
    sockaddr* address_p = (sockaddr*)&address;
    memset(address_p, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(45676);
    address.sin_addr.s_addr = addr.s_addr;

    SOCKET* socketPtr = new SOCKET();
    if (*socketPtr < 0)
        throw std::runtime_error("Could not create socket to connect to TraCI server");

    int tries = 1;
    for (; tries <= 10; ++tries)
    {
        *socketPtr = ::socket(AF_INET, SOCK_STREAM, 0);
        if (::connect(*socketPtr, address_p, sizeof(address)) >= 0)
            break;

        closesocket(socket(socketPtr));

        int sleepDuration = tries * .25 + 1;

        std::cout << boost::format("  Could not connect to the TraCI server: %1% -- retry in %2% seconds. \n") % strerror(sock_errno()) % sleepDuration;
        std::cout.flush();

        std::this_thread::sleep_for(std::chrono::seconds(sleepDuration));
    }

    if(tries == 11)
        throw std::runtime_error("Could not connect to TraCI server after 10 retries!");

    // TCP_NODELAY: disable the Nagle algorithm. This means that segments are always
    // sent as soon as possible, even if there is only a small amount of data.
    // When not set, data is buffered until there is a sufficient amount to send out,
    // thereby avoiding the frequent sending of small packets, which results
    // in poor utilization of the network.
    int x = 1;
    ::setsockopt(*socketPtr, IPPROTO_TCP, TCP_NODELAY, (const char*) &x, sizeof(x));

    std::cout << "connected \n";
    std::cout.flush();

    std::string msg = "1||mani";

    {
        // fill the buffer
        char tx_buffer[1000];
        sprintf (tx_buffer, "%s", msg.c_str());

        // sending the msg to the TCP server
        int n = write(*socketPtr, tx_buffer, strlen(tx_buffer));
        if (n < 0)
            throw std::runtime_error("Writing error to socket");

        // receiving msg from the TCP server
        char rx_buffer[20];
        bzero(rx_buffer, 20);
        n = read(*socketPtr, rx_buffer, 19);
        if (n < 0)
            throw std::runtime_error("Reading error from socket");

        if(std::string(rx_buffer) != "ok!")
            throw std::runtime_error("Server's response msg is wrong!");
    }

    return 0;
}
