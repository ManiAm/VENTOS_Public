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
        // get the ptr of the current module
        nodePtr = FindModule<>::findHost(this);
        if(nodePtr == NULL)
            error("can not get a pointer to the module.");

        // get a pointer to the TraCI module
        cModule *module = simulation.getSystemModule()->getSubmodule("TraCI");
        TraCI = static_cast<TraCI_Extend *>(module);

        Signal_executeFirstTS = registerSignal("executeFirstTS");
        simulation.getSystemModule()->subscribe("executeFirstTS", this);

        on = par("on").boolValue();

        zoom = par("zoom").doubleValue();
        if(zoom < 0)
            error("zoom value is not correct!");

        initialWindowsOffset = par("initialWindowsOffset").doubleValue();
        if(initialWindowsOffset < 0)
            error("Initial Windows Offset value is not correct!");

        trackingInterval = par("trackingInterval").doubleValue();
        if(trackingInterval <= 0)
            error("Tracking interval should be positive!");

        mode = par("mode").longValue();
        trackingV = par("trackingV").stdstringValue();
        trackingLane = par("trackingLane").stringValue();
        windowsOffset = par("windowsOffset").doubleValue();

        updataGUI = new cMessage("updataGUI", 1);
    }
}


void Tracking::finish()
{

}


void Tracking::receiveSignal(cComponent *source, simsignal_t signalID, long i)
{
    Enter_Method_Silent();

    if(signalID == Signal_executeFirstTS)
    {
        Tracking::Start();
    }
}


void Tracking::Start()
{
    // todo:
    // use TraCI command, to also check if we are really running in GUI mode

    if(!on)
        return;

    // zoom-in GUI
    TraCI->GUISetZoom(zoom);

    // adjust Windows initially
    TraCI->GUISetOffset(initialWindowsOffset, 0.);

    TrackingGUI();
}


void Tracking::handleMessage(cMessage *msg)
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
            TraCI->GUISetOffset(co.x, co.y);
    }
    else if(mode == 2)
    {
        // get a list of vehicles on this lane!
        std::list<std::string> myList = TraCI->laneGetLastStepVehicleIDs(trackingLane.c_str());

        if(!myList.empty())
        {
            // get iterator to the end
            std::list<std::string>::iterator it = myList.end();

            // iterator pointing to the last element
            --it;

            // first inserted vehicle on this lane
            std::string lastVehicleId = *it;

            Coord lastVehiclePos = TraCI->vehicleGetPosition(lastVehicleId);

            // get GUI windows boundary
            std::vector<double> windowsFrame = TraCI->GUIGetBoundry();

            // vehicle goes out of frame?
            if(lastVehiclePos.x > windowsFrame[2] || lastVehiclePos.y > windowsFrame[3])
            {
                TraCI->GUISetOffset(windowsFrame[0] + windowsOffset, 0);
            }
        }
    }
    else
    {
        error("not a valid mode!");
    }

    scheduleAt(simTime() + trackingInterval, updataGUI);
}

}

