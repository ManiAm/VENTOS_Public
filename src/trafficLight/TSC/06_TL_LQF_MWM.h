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

#include "05_TL_OJF.h"

namespace VENTOS {

class TrafficLight_LQF_MWM : public TrafficLightOJF
{
public:
    virtual ~TrafficLight_LQF_MWM();
    virtual void initialize(int);
    virtual void finish();
    virtual void handleMessage(omnetpp::cMessage *);

protected:
    void virtual initialize_withTraCI();
    void virtual executeEachTimeStep();

private:
    void chooseNextInterval();
    void chooseNextGreenInterval();

protected:
    // vehicle type should be identical to vehicle type in beacon field
    std::map<std::string /*vehicleType*/, double /*weight*/> classWeight =
    {
            {"emergency", 50},
            {"bicycle", 40},
            {"pedestrian", 30},
            {"passenger", 20},
            {"bus", 10},
            {"truck", 1}
    };

    std::string phase1_5 = "grgrGgrgrrgrgrGgrgrrrrrr";
    std::string phase2_5 = "gGgGGgrgrrgrgrrgrgrrrrrG";
    std::string phase1_6 = "grgrrgrgrrgGgGGgrgrrrGrr";
    std::string phase2_6 = "gGgGrgrgrrgGgGrgrgrrrGrG";

    std::string phase3_7 = "grgrrgrgrGgrgrrgrgrGrrrr";
    std::string phase3_8 = "grgrrgrgrrgrgrrgGgGGrrGr";
    std::string phase4_7 = "grgrrgGgGGgrgrrgrgrrGrrr";
    std::string phase4_8 = "grgrrgGgGrgrgrrgGgGrGrGr";

    std::vector<std::string> phases = {phase1_5, phase2_5, phase1_6, phase2_6, phase3_7, phase3_8, phase4_7, phase4_8};

private:
    typedef TrafficLightOJF super;
    ApplRSUMonitor *RSUptr = NULL;
    double nextGreenTime;
};

}

#endif
