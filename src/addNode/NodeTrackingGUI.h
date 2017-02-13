/****************************************************************************/
/// @file    NodeTrackingGUI.h
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

#ifndef TRACKING_H
#define TRACKING_H

#include "baseAppl/03_BaseApplLayer.h"
#include "traci/TraCICommands.h"

namespace VENTOS {

class Tracking : public BaseApplLayer
{
private:
    // NED variables
    TraCI_Commands *TraCI;  // pointer to the TraCI module
    omnetpp::simsignal_t Signal_initialize_withTraCI;

    // NED variables (GUI)
    int mode;
    double zoom;
    double winOffsetX;
    double winOffsetY;
    double trackingInterval;
    std::string trackingV;
    std::string trackingLane;
    double windowsOffset;

    // class variables
    omnetpp::cMessage* updataGUI = NULL;

public:
    virtual ~Tracking();
    virtual void initialize(int stage);
    virtual void handleMessage(omnetpp::cMessage *msg);
    virtual void finish();
    virtual void receiveSignal(omnetpp::cComponent *, omnetpp::simsignal_t, long, cObject* details);

private:
    void TrackingGUI();
};

}

#endif
