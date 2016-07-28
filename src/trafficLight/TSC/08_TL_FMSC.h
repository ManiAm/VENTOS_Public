/****************************************************************************/
/// @file    TL_FMSC.h
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @date    Jul 2015
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

#ifndef TRAFFICLIGHTFMSC_H
#define TRAFFICLIGHTFMSC_H

#include "07_TL_LQF_MWM_Aging.h"

namespace VENTOS {

class greenInterval_FMSC
{
public:
    int maxVehCount;
    double maxWeight;
    int oneCount;
    double greenTime;
    std::string greenString;

    greenInterval_FMSC(int i1, double d0, int i2, double d1, std::string str)
    {
        this->maxVehCount = i1;
        this->maxWeight = d0;
        this->oneCount = i2;
        this->greenTime = d1;
        this->greenString = str;
    }
};


class TrafficLight_FMSC : public TrafficLight_LQF_MWM_Aging
{
public:
    virtual ~TrafficLight_FMSC();
    virtual void initialize(int);
    virtual void finish();
    virtual void handleMessage(omnetpp::cMessage *);

protected:
    void virtual initialize_withTraCI();
    void virtual executeEachTimeStep();

private:
    void chooseNextInterval();
    void chooseNextGreenInterval();
    void calculatePhases(std::string);

private:
    typedef TrafficLight_LQF_MWM_Aging super;
    ApplRSUMonitor *RSUptr;
    std::vector<greenInterval_FMSC> greenInterval;
    bool nextGreenIsNewCycle;
};

}

#endif
