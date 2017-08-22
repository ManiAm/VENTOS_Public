/****************************************************************************/
/// @file    TraCIConnection.h
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

#ifndef TRACICONNECTION_H_
#define TRACICONNECTION_H_

#include <stdint.h>
#include <mutex>

#include "mobility/Coord.h"
#include "mobility/TraCICoord.h"
#include "traci/TraCIBuffer.h"

namespace VENTOS {

class TraCIConnection
{
private:
    static void* socketPtr;
    static pid_t child_pid;
    static std::mutex lock_TraCI;

public:
    static void startSUMO(std::string SUMOexe, std::string SUMOconfig, std::string SUMOswitches, int port);
    static int getFreeEphemeralPort();
    static TraCIConnection* connect(const char* host, int port);
    ~TraCIConnection();

    /**
     * sends a single command via TraCI, checks status response, returns additional responses
     */
    TraCIBuffer query(uint8_t commandId, const TraCIBuffer& buf = TraCIBuffer());

    /**
     * sends a message via TraCI (after adding the header)
     */
    void sendMessage(std::string buf);

    /**
     * receives a message via TraCI (and strips the header)
     */
    std::string receiveMessage();

private:
    TraCIConnection(void*);
    void terminateSimulationOnError(std::string);
    static std::string getSUMOversion(std::string path);
};

/**
 * returns byte-buffer containing a TraCI command with optional parameters
 */
std::string makeTraCICommand(uint8_t commandId, const TraCIBuffer& buf = TraCIBuffer());

}

#endif /* VEINS_MOBILITY_TRACI_TRACICONNECTION_H_ */
