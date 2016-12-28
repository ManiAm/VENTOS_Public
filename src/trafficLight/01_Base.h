/****************************************************************************/
/// @file    Base.h
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

#ifndef TRAFFICLIGHTBASE_H
#define TRAFFICLIGHTBASE_H

#undef ev
#include "boost/filesystem.hpp"

#include "MIXIM/modules/BaseApplLayer.h"
#include "traci/TraCICommands.h"
#include "rsu/02_Monitor.h"
#include "logging/vlog.h"


namespace VENTOS {

class TrafficLightBase : public BaseApplLayer
{
protected:
    TraCI_Commands *TraCI;

    int TLControlMode;
    int debugLevel;
    double updateInterval;

private:
    typedef BaseApplLayer super;

public:
    virtual ~TrafficLightBase();
    virtual void initialize(int);
    virtual void finish();
    virtual void handleMessage(omnetpp::cMessage *);

protected:
    virtual void initialize_withTraCI();
    virtual void executeEachTimeStep();
    ApplRSUMonitor * findRSU(std::string);
};

}

#endif
