/****************************************************************************/
/// @file    ApplV_01_Base.cc
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

#include "ApplV_01_Base.h"

namespace VENTOS {

const simsignalwrap_t ApplVBase::mobilityStateChangedSignal = simsignalwrap_t(MIXIM_SIGNAL_MOBILITY_CHANGE_NAME);

Define_Module(VENTOS::ApplVBase);

ApplVBase::~ApplVBase()
{

}

void ApplVBase::initialize(int stage)
{
    BaseApplLayer::initialize(stage);

    if (stage==0)
    {
        // get the ptr of the current module
        nodePtr = this->getParentModule();
        if(nodePtr == NULL)
            error("can not get a pointer to the module.");

        // get a pointer to the TraCI module
        cModule *module = simulation.getSystemModule()->getSubmodule("TraCI");
        TraCI = static_cast<TraCI_Commands *>(module);
        ASSERT(TraCI);

        // get a pointer to traffic light module
        module = simulation.getSystemModule()->getSubmodule("TrafficLight");
        TLControlMode = module->par("TLControlMode").longValue();

        headerLength = par("headerLength").longValue();

        // vehicle id in omnet++
        myId = getParentModule()->getIndex();
        // vehicle full id in omnet++
        myFullId = getParentModule()->getFullName();
        // vehicle id in sumo
        SUMOID = par("SUMOID").stringValue();
        // vehicle type in sumo
        SUMOType = par("SUMOType").stringValue();
        // vehicle class in sumo
        vehicleClass = par("vehicleClass").stringValue();
        // vehicle class code
        vehicleClassEnum = par("vehicleClassEnum").longValue();
        // get controller type from SUMO
        SUMOControllerType = par("SUMOControllerType").longValue();
        // get controller number from SUMO
        SUMOControllerNumber = par("SUMOControllerNumber").longValue();

        VENTOS_FullPath = cSimulation::getActiveSimulation()->getEnvir()->getConfig()->getConfigEntry("network").getBaseDirectory();
        SUMO_Path = simulation.getSystemModule()->par("SUMODirectory").stringValue();
        SUMO_FullPath = VENTOS_FullPath / SUMO_Path;
        if( !boost::filesystem::exists( SUMO_FullPath ) )
            error("SUMO directory is not valid! Check it again.");

        // store the time of entry
        entryTime = simTime().dbl();

        // comment this to speed-up the simulation
        //findHost()->subscribe(mobilityStateChangedSignal, this);
    }
}


void ApplVBase::finish()
{
    //findHost()->unsubscribe(mobilityStateChangedSignal, this);
}


void ApplVBase::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj)
{
    Enter_Method_Silent();

    if (signalID == mobilityStateChangedSignal)
    {
        handlePositionUpdate(obj);
    }
}


void ApplVBase::handleSelfMsg(cMessage* msg)
{

}


void ApplVBase::handlePositionUpdate(cObject* obj)
{
    ChannelMobilityPtrType const mobility = check_and_cast<ChannelMobilityPtrType>(obj);
    curPosition = mobility->getCurrentPosition();
}

}

