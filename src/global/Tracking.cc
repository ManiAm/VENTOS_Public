/****************************************************************************/
/// @file    Tracking.cc
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

#include "Tracking.h"

Define_Module(VENTOS::Tracking);

namespace VENTOS {

Tracking::~Tracking()
{

}


void Tracking::initialize(int stage)
{
    if(stage ==0)
    {
        mode = par("mode").longValue();

        if(mode < 0)
            return;

        // get a pointer to the TraCI module
        cModule *module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("TraCI");
        TraCI = static_cast<TraCI_Commands *>(module);
        ASSERT(TraCI);

        Signal_initialize_withTraCI = registerSignal("initialize_withTraCI");
        omnetpp::getSimulation()->getSystemModule()->subscribe("initialize_withTraCI", this);

        zoom = par("zoom").doubleValue();
        if(zoom < 0)
            throw omnetpp::cRuntimeError("zoom value is not correct!");

        initialWindowsOffset = par("initialWindowsOffset").doubleValue();
        if(initialWindowsOffset < 0)
            throw omnetpp::cRuntimeError("Initial Windows Offset value is not correct!");

        trackingInterval = par("trackingInterval").doubleValue();
        if(trackingInterval <= 0)
            throw omnetpp::cRuntimeError("Tracking interval should be positive!");

        trackingV = par("trackingV").stdstringValue();
        trackingLane = par("trackingLane").stringValue();
        windowsOffset = par("windowsOffset").doubleValue();

        updataGUI = new omnetpp::cMessage("updataGUI", 1);
    }
}


void Tracking::finish()
{
    // unsubscribe
    omnetpp::getSimulation()->getSystemModule()->unsubscribe("initialize_withTraCI", this);
    omnetpp::getSimulation()->getSystemModule()->unsubscribe("executeEachTS", this);
}


void Tracking::receiveSignal(omnetpp::cComponent *source, omnetpp::simsignal_t signalID, long i, cObject* details)
{
    Enter_Method_Silent();

    if(signalID == Signal_initialize_withTraCI)
    {
        // todo:
        // first check if we are really running in GUI mode

        if(mode < 0)
            return;

        if(mode == 0)
        {
            TraCI->GUISetZoom("View #0", zoom);
            return;
        }

        if(mode == 1 || mode == 2)
        {
            // zoom-in GUI
            TraCI->GUISetZoom("View #0", zoom);

            // adjust Windows initially
            TraCI->GUISetOffset("View #0", initialWindowsOffset, 0.);

            TrackingGUI();
        }
    }
}


void Tracking::handleMessage(omnetpp::cMessage *msg)
{
    if (msg == updataGUI)
    {
        TrackingGUI();
    }
}


void Tracking::TrackingGUI()
{
    if(mode == 1)
    {
        // get vehicle position in SUMO coordinates
        Coord co = TraCI->vehicleGetPosition(trackingV);

        if(co.x > 0)
            TraCI->GUISetOffset("View #0", co.x, co.y);
    }
    else if(mode == 2)
    {
        // get a list of vehicles on this lane!
        std::list<std::string> myList = TraCI->laneGetLastStepVehicleIDs(trackingLane.c_str());

        if(!myList.empty())
        {
            // iterator pointing to the last element
            std::list<std::string>::iterator it = myList.end();
            --it;

            // get the first inserted vehicle on this lane
            std::string lastVehicleId = *it;

            Coord lastVehiclePos = TraCI->vehicleGetPosition(lastVehicleId);

            // get GUI windows boundary
            std::vector<double> windowsFrame = TraCI->GUIGetBoundry("View #0");

            // vehicle goes out of frame?
            if(lastVehiclePos.x > windowsFrame[2] || lastVehiclePos.y > windowsFrame[3])
                TraCI->GUISetOffset("View #0", windowsFrame[0] + windowsOffset, 0);
        }
    }
    else
        throw omnetpp::cRuntimeError("not a valid mode!");

    scheduleAt(omnetpp::simTime() + trackingInterval, updataGUI);
}

}

