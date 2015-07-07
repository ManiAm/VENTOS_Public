/****************************************************************************/
/// @file    RSUMobility.cc
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

#include "RSUMobility.h"
#include <FWMath.h>

namespace VENTOS {

Define_Module(VENTOS::RSUMobility);


void RSUMobility::initialize(int stage)
{
    BaseMobility::initialize(stage);

    if (stage == 0)
    {
        // get the ptr of the current module
        nodePtr = FindModule<>::findHost(this);
        if(nodePtr == NULL)
            error("can not get a pointer to the module.");

        // get a pointer to the TraCI module
        cModule *module = simulation.getSystemModule()->getSubmodule("TraCI");
        TraCI = static_cast<TraCI_Extend *>(module);

        boost::filesystem::path VENTOS_FullPath = cSimulation::getActiveSimulation()->getEnvir()->getConfig()->getConfigEntry("network").getBaseDirectory();
        boost::filesystem::path SUMO_Path = simulation.getSystemModule()->par("SUMODirectory").stringValue();
        boost::filesystem::path SUMO_FullPath = VENTOS_FullPath / SUMO_Path;
        if( !boost::filesystem::exists( SUMO_FullPath ) )
            error("SUMO directory is not valid! Check it again.");

        // vehicle id in omnet++
        myId = getParentModule()->getIndex();
        myFullId = getParentModule()->getFullName();

        Coord pos = world->getRandomPosition();

        // read coordinates from parameters if available
        double myCoordX = getParentModule()->getSubmodule("appl")->par("myCoordX").doubleValue();
        double myCoordY = getParentModule()->getSubmodule("appl")->par("myCoordY").doubleValue();
        if(myCoordX == -1 || myCoordY == -1)
            error("RSU coordinates are not set correctly!");

        pos.x = myCoordX;
        pos.y = myCoordY;
        pos.z = 0;

        // set start-position and start-time (i.e. current simulation-time) of the Move
        move.setStart(pos);

        //check whether position is within the playground
        if ( !isInBoundary(move.getStartPos(), Coord::ZERO, *world->getPgs()) ) {
            error("node position specified in omnetpp.ini exceeds playgroundsize");
        }

        // set speed and direction of the Move
        move.setSpeed(0);
        move.setDirectionByVector(Coord::ZERO);
    }
}


bool RSUMobility::isInBoundary(Coord c, Coord lowerBound, Coord upperBound)
{
    return  lowerBound.x <= c.x && c.x <= upperBound.x &&
            lowerBound.y <= c.y && c.y <= upperBound.y &&
            lowerBound.z <= c.z && c.z <= upperBound.z;
}


/**
 * Calculate a new random position and the number of steps the host
 * needs to reach this position
 */
void RSUMobility::setTargetPosition()
{

}


/**
 * Move the host if the destination is not reached yet. Otherwise
 * calculate a new random position
 */
void RSUMobility::makeMove()
{

}


}

