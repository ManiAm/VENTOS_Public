/****************************************************************************/
/// @file    SumoServer.cc
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author  second author name
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

#include "SumoServer.h"

namespace VENTOS {

Define_Module(VENTOS::SumoServer);

void SumoServer::initialize(int stage)
{
    if(stage == 2)
    {
        // get a pointer to the TraCI module
        cModule *module = simulation.getSystemModule()->getSubmodule("SumoBinary");

        SUMO_GUI_FileName = module->par("SUMO_GUI_FileName").stringValue();
        SUMO_CMD_FileName = module->par("SUMO_CMD_FileName").stringValue();

        VENTOS_FullPath = cSimulation::getActiveSimulation()->getEnvir()->getConfig()->getConfigEntry("network").getBaseDirectory();
        SUMO_Binary_FullPath = VENTOS_FullPath / "sumoBinary";

        SUMO_GUI_Binary_FullPath = SUMO_Binary_FullPath / SUMO_GUI_FileName;
        SUMO_CMD_Binary_FullPath = SUMO_Binary_FullPath / SUMO_CMD_FileName;

        createServerSocket();
    }
}


void SumoServer::finish()
{

}


void SumoServer::handleMessage(cMessage *msg)
{

}


void SumoServer::createServerSocket()
{

}

} // namespace


