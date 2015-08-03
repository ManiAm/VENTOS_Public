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
        activeDetection = par("activeDetection").boolValue();
        debugLevel = simulation.getSystemModule()->par("debugLevel").longValue();

        // initialize RSUptr with NULL
        RSUptr = NULL;

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
    // get a pointer to the RSU module that controls this TL
    if(activeDetection)
        findRSU("C");
}


void TrafficLightBase::executeEachTimeStep(bool simulationDone)
{

}


void TrafficLightBase::findRSU(std::string TLid)
{
    // get a pointer to the RSU module that controls this intersection
    cModule *module = simulation.getSystemModule()->getSubmodule("RSU", 0);
    if(module == NULL)
        error("No RSU module was found in the network!");

    // how many RSUs are in the network?
    int RSUcount = module->getVectorSize();

    // iterate over RSUs
    bool found = false;
    for(int i = 0; i < RSUcount; ++i)
    {
        module = simulation.getSystemModule()->getSubmodule("RSU", i);
        cModule *appl =  module->getSubmodule("appl");
        std::string myTLid = appl->par("myTLid").stringValue();

        // we found our RSU
        if(myTLid == TLid)
        {
            RSUptr = static_cast<ApplRSUTLVANET *>(appl);
            if(RSUptr == NULL)
                error("Can not get a reference to our RSU!");

            found = true;
            break;
        }
    }

    if(!found)
        error("TL %s does not have any RSU!", TLid.c_str());
}


}
