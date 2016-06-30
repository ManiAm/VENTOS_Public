/****************************************************************************/
/// @file    dataExchange.h
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

#ifndef DATAEXCHANGE_H_
#define DATAEXCHANGE_H_

#include <BaseApplLayer.h>
#include <mutex>
#include "TraCICommands.h"

namespace VENTOS {

// class for holding incoming data from a board
class dataEntry
{
public:
    unsigned char *buffer;
    uint32_t bufferLength;
    std::string ipv4;
    uint16_t port;

    dataEntry(unsigned char *buffer, uint32_t bufferLength, std::string ipv4, uint16_t port)
    {
        // copy buffer
        this->buffer = new unsigned char[bufferLength + 1];
        std::copy(buffer, buffer + bufferLength, this->buffer);

        this->bufferLength = bufferLength;
        this->ipv4 = ipv4;
        this->port = port;
    }
};


class dataExchange : public BaseApplLayer
{
public:
    virtual ~dataExchange();
    virtual void initialize(int stage);
    virtual void finish();
    virtual void handleMessage(omnetpp::cMessage *msg);
    virtual void receiveSignal(omnetpp::cComponent *, omnetpp::simsignal_t, long, cObject* details);

    void sendDataToBoard(std::string, unsigned char *, unsigned int);

private:
    void initialize_withTraCI();
    void executeEachTimestep();

    void start_TCP_server();
    void recvDataFromBoard(int, std::string, uint16_t);
    void connect_to_TCP_server(std::string);

private:
    typedef BaseApplLayer super;

    TraCI_Commands *TraCI;  // pointer to the TraCI module
    bool active;
    int VENTOS_serverPort;
    int board_serverPort;
    int numBacklog;

    omnetpp::simsignal_t Signal_executeEachTS;
    omnetpp::simsignal_t Signal_initialize_withTraCI;

    int sockfd;  // welcoming socket
    std::vector<int> connections_fromBoard;
    std::vector<dataEntry *> receivedData;
    std::mutex lock_vector;

    std::map<std::string, int *> connections_fromVENTOS;
};

}

#endif
