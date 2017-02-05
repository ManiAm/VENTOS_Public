/****************************************************************************/
/// @file    TrafficActuated.h
/// @author  Philip Vo <foxvo@ucdavis.edu>
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
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

#ifndef TRAFFICLIGHTACTUATED_H
#define TRAFFICLIGHTACTUATED_H

#include "trafficLight/TSC/02_Adaptive_Webster.h"

namespace VENTOS {

class TrafficLightActuated : public TrafficLightWebster
{
private:
    typedef TrafficLightWebster super;

    // NED variables
    double passageTime;
    bool greenExtension;
    double intervalElapseTime;

    std::string currentInterval;
    double intervalDuration;
    std::string nextGreenInterval;

    omnetpp::cMessage* intervalChangeEVT = NULL;

    // class variables
    std::map<std::string,double> passageTimePerLane;

    // list of all traffic lights in the network
    std::vector<std::string> TLList;

    std::map<std::string /*TLid*/, std::string /*first green interval*/> firstGreen;

    // loop detector ids used for actuated-time signal control
    std::unordered_map<std::string /*lane*/, std::string /*LD id*/> LD_actuated;

public:
    virtual ~TrafficLightActuated();
    virtual void initialize(int);
    virtual void finish();
    virtual void handleMessage(omnetpp::cMessage *);

protected:
    void virtual initialize_withTraCI();
    void virtual executeEachTimeStep();

private:
    void chooseNextInterval(std::string TLid);
    void chooseNextGreenInterval(std::string TLid);
    void checkLoopDetectors();
};

}

#endif
