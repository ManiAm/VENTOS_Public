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

    std::string phase1_5 = "grgrGgrgrrgrgrGgrgrrrrrr";
    std::string phase2_5 = "gGgGGgrgrrgrgrrgrgrrrrrG";
    std::string phase1_6 = "grgrrgrgrrgGgGGgrgrrrGrr";
    std::string phase2_6 = "gGgGrgrgrrgGgGrgrgrrrGrG";

    std::string phase3_7 = "grgrrgrgrGgrgrrgrgrGrrrr";
    std::string phase3_8 = "grgrrgrgrrgrgrrgGgGGrrGr";
    std::string phase4_7 = "grgrrgGgGGgrgrrgrgrrGrrr";
    std::string phase4_8 = "grgrrgGgGrgrgrrgGgGrGrGr";

    // NED variables
    double passageTime;
    bool greenExtension;
    double intervalElapseTime;

    // class variables
    std::map<std::string,double> passageTimePerLane;

public:
    virtual ~TrafficLightActuated();
    virtual void initialize(int);
    virtual void finish();
    virtual void handleMessage(omnetpp::cMessage *);

protected:
    void virtual initialize_withTraCI();
    void virtual executeEachTimeStep();

private:
    void chooseNextInterval();
    void chooseNextGreenInterval();
};

}

#endif
