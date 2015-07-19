/****************************************************************************/
/// @file    TL_MaxQueue.h
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

#ifndef TRAFFICLIGHTADAPTIVEQUEUE_H
#define TRAFFICLIGHTADAPTIVEQUEUE_H

#include <07_TL_TrafficActuated.h>

namespace VENTOS {

class greenIntervalInfo
{
public:
    int maxVehCount;
    double greenTime;
    std::string greenString;

    greenIntervalInfo(int i1, double d1, std::string str)
    {
        this->maxVehCount = i1;
        this->greenTime = d1;
        this->greenString = str;
    }
};


class TrafficLightAdaptiveQueue : public TrafficLightActuated
{
public:
    virtual ~TrafficLightAdaptiveQueue();
    virtual void finish();
    virtual void initialize(int);
    virtual void handleMessage(cMessage *);

protected:
    void virtual executeFirstTimeStep();
    void virtual executeEachTimeStep(bool);

private:
    void chooseNextInterval();
    void chooseNextGreenInterval();
    void calculatePhases(std::string);

private:
    int maxQueueSize;
    bool nextGreenIsNewCycle;
    std::vector<greenIntervalInfo> greenInterval;
};

}

#endif
