/****************************************************************************/
/// @file    TrafficLightBase.cc
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

#include <01_TL_Base.h>

namespace VENTOS {

Define_Module(VENTOS::TrafficLightBase);


TrafficLightBase::~TrafficLightBase()
{

}


void TrafficLightBase::initialize(int stage)
{
    if(stage == 0)
	{
        // get a pointer to the TraCI module
        cModule *module = simulation.getSystemModule()->getSubmodule("TraCI");
        TraCI = static_cast<TraCI_Extend *>(module);

        Signal_executeFirstTS = registerSignal("executeFirstTS");
        simulation.getSystemModule()->subscribe("executeFirstTS", this);

        Signal_executeEachTS = registerSignal("executeEachTS");
        simulation.getSystemModule()->subscribe("executeEachTS", this);

        updateInterval = TraCI->par("updateInterval").doubleValue();
        TLControlMode = par("TLControlMode").longValue();

        // get the file paths
        VENTOS_FullPath = cSimulation::getActiveSimulation()->getEnvir()->getConfig()->getConfigEntry("network").getBaseDirectory();
        SUMO_Path = simulation.getSystemModule()->par("SUMODirectory").stringValue();
        SUMO_FullPath = VENTOS_FullPath / SUMO_Path;
        if( !boost::filesystem::exists( SUMO_FullPath ) )
            error("SUMO directory is not valid! Check it again.");
    }
}


void TrafficLightBase::finish()
{

}


void TrafficLightBase::handleMessage(cMessage *msg)
{

}


void TrafficLightBase::receiveSignal(cComponent *source, simsignal_t signalID, long i)
{
    Enter_Method_Silent();

    if(signalID == Signal_executeFirstTS)
    {
        executeFirstTimeStep();
    }
    else if(signalID == Signal_executeEachTS)
    {
        executeEachTimeStep((bool)i);
    }
}


void TrafficLightBase::executeFirstTimeStep()
{

}


void TrafficLightBase::executeEachTimeStep(bool simulationDone)
{

}


}
