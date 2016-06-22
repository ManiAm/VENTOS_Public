/****************************************************************************/
/// @file    ApplV_redpine.cc
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

#include "ApplV_05_redpine.h"

namespace VENTOS {

Define_Module(VENTOS::ApplVRedpine);

ApplVRedpine::~ApplVRedpine()
{

}


void ApplVRedpine::initialize(int stage)
{
    super::initialize(stage);

    if (stage == 0)
    {
        EEBL = par("EEBL").boolValue();

    }
}


void ApplVRedpine::finish()
{
    super::finish();

    //findHost()->unsubscribe(mobilityStateChangedSignal, this);
}


void ApplVRedpine::handleSelfMsg(omnetpp::cMessage* msg)
{
    super::handleSelfMsg(msg);
}


// is called, every time the position of vehicle changes
void ApplVRedpine::handlePositionUpdate(cObject* obj)
{
    super::handlePositionUpdate(obj);

    ChannelMobilityPtrType const mobility = omnetpp::check_and_cast<ChannelMobilityPtrType>(obj);
    curPosition = mobility->getCurrentPosition();
}


void ApplVRedpine::receiveDataFromBoard(redpineData* data)
{
    Enter_Method("");

    std::cout << SUMOID << " received a EEBL msg of size " << data->getDataArraySize() << std::endl;

    u_char *payload = new u_char[data->getDataArraySize()];
    for(unsigned int i = 0; i < data->getDataArraySize(); i++)
        payload[i] = data->getData(i);

    // print the payload
    //print_dataPayload(payload, data->getDataArraySize());
    //std::cout << std::endl << std::flush;

    delete[] payload;
    delete data;

    // stopping the vehicle
    TraCI->vehicleSetSpeed(SUMOID, 0);

    // prepare a 'basic safety message'
    BSM* wsm = new BSM("BSM");
    wsm->setSender(SUMOID.c_str());
    wsm->setSenderType(SUMOType.c_str());
    Coord cord = TraCI->vehicleGetPosition(SUMOID);
    wsm->setPos(cord);

    // and broadcast it to nearby vehicles
    send(wsm, lowerLayerOut);
}


void ApplVRedpine::onBeaconVehicle(BeaconVehicle* wsm)
{
    // pass it down
    super::onBeaconVehicle(wsm);
}


void ApplVRedpine::onBeaconRSU(BeaconRSU* wsm)
{
    // pass it down
    super::onBeaconRSU(wsm);
}


void ApplVRedpine::onData(PlatoonMsg* wsm)
{
    // pass it down
    super::onData(wsm);
}


void ApplVRedpine::onHIL(BSM* wsm)
{
    // check the received data and act on it.
    // all wsm messages here are EEBL for now.
    // we stop, as soon as we receive a EEBL msg.
    // todo: we should decode the received msg

    // if msg is coming from my leading vehicle
    std::vector<std::string> leaderv = TraCI->vehicleGetLeader(SUMOID, sonarDist);
    std::string leader = leaderv[0];
    if(leader == wsm->getSender())
    {
        TraCI->vehicleSetMaxDecel(SUMOID, 4);
        TraCI->vehicleSetSpeed(SUMOID, 0);
    }
}


// print packet payload data (avoid printing binary data)
void ApplVRedpine::print_dataPayload(const u_char *payload, int len)
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
void ApplVRedpine::print_hex_ascii_line(const u_char *payload, int len, int offset)
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
