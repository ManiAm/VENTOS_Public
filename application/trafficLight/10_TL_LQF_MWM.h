/****************************************************************************/
/// @file    TL_LQF_MWM.h
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

// This algorithm is implemented according to the paper, but suffers from starvation.
// It performs the scheduling per phase (and not per cycle)

#ifndef TRAFFICLIGHTLQFMWM_H
#define TRAFFICLIGHTLQFMWM_H

#include <09_TL_OJF.h>

namespace VENTOS {

class greenIntervalInfo_LQF
{
public:
    int maxVehCount;
    double totalWeight;
    int oneCount;
    double greenTime;
    std::string greenString;

    greenIntervalInfo_LQF(int i1, double d0, int i2, double d1, std::string str)
    {
        this->maxVehCount = i1;
        this->totalWeight = d0;
        this->oneCount = i2;
        this->greenTime = d1;
        this->greenString = str;
    }
};


class TrafficLight_LQF_MWM : public TrafficLightOJF
{
public:
    virtual ~TrafficLight_LQF_MWM();
    virtual void finish();
    virtual void initialize(int);
    virtual void handleMessage(cMessage *);

protected:
    void virtual executeFirstTimeStep();
    void virtual executeEachTimeStep();

private:
    void chooseNextInterval();
    void chooseNextGreenInterval();

protected:
    std::map<std::string /*className*/, double /*weight*/> classWeight =
    {
            {"emergency", 50},
            {"bicycle", 40},
            {"pedestrian", 30},
            {"passenger", 20},
            {"bus", 10},
            {"truck", 1}
    };

    std::vector<std::string> phases = {phase1_5, phase2_5, phase1_6, phase2_6, phase3_7, phase3_8, phase4_7, phase4_8};

private:
    double nextGreenTime;
};

}

#endif
