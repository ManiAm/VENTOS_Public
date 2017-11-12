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

#include "baseAppl/03_BaseApplLayer.h"
#include "traci/TraCICommands.h"
#include "nodes/rsu/02_Monitor.h"
#include "logging/VENTOS_logging.h"


namespace VENTOS {

class TrafficLightBase : public BaseApplLayer
{
protected:
    TraCI_Commands *TraCI;

    int TLControlMode;
    int debugLevel;
    double updateInterval;

    enum TLControlTypes {
        TL_OFF,
        TL_Fix_Time,
        TL_Adaptive_Webster,
        TL_TrafficActuated,
        TL_LQF,
        TL_OJF,
        TL_LQF_MWM,
        TL_LQF_MWM_Aging,
        TL_FMSC,
        TL_Router,
        TL_DRL,

        NUM_TL_CONTROLS
    };

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
