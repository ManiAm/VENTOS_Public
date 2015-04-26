/****************************************************************************/
/// @file    TrafficLightControl.h
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

#ifndef TRAFFICLIGHTCONTROL_H
#define TRAFFICLIGHTCONTROL_H

#include <TrafficLightBase.h>
#include <Appl.h>
#include "TraCI_Extend.h"

using namespace std;

namespace VENTOS {

class TrafficLightControl : public TrafficLightBase
{
  public:
    virtual ~TrafficLightControl();
    virtual void finish();
    virtual void initialize(int);
    virtual void handleMessage(cMessage *);

  private:
    void virtual executeFirstTimeStep();
    void virtual executeEachTimeStep();

    void doAdaptiveTimeControl();
    void doVANETControl();

    // NED variables
    double minGreenTime;
    double maxGreenTime;
    double yellowTime;
    double redTime;
    double passageTime;
    double detectorPos;

    // class variables
    cMessage* ChangeEvt;
    cMessage* DetectEvt;

    list<string> LDList;   // list of loop-detectors in the network

    vector<double> DetectedTime;

    double offSet = 0.0;
    double detectFreq = 0.1;
    double radius = 33;
  //  Coord outerRing = (100 - radius - 2;
  //  Coord innerRing  = radius + 2;
    double intervalElapseTime = 0.0;
    string currentInterval;
    string nextGreenInterval;

    string phase1_5 = "rrrrGrrrrrrrrrGrrrrrrrrr";
    string phase2_5 = "gGgGGrrrrrrrrrrrrrrrrrrG";
    string phase1_6 = "rrrrrrrrrrgGgGGrrrrrrGrr";
    string phase2_6 = "gGgGrrrrrrgGgGrrrrrrrGrG";

    string phase3_7 = "rrrrrrrrrGrrrrrrrrrGrrrr";
    string phase3_8 = "rrrrrrrrrrrrrrrgGgGGrrGr";
    string phase4_7 = "rrrrrgGgGGrrrrrrrrrrGrrr";
    string phase4_8 = "rrrrrgGgGrrrrrrgGgGrGrGr";

    enum LDid
    {
        EC_2, EC_3, EC_4,
        NC_2, NC_3, NC_4,
        SC_2, SC_3, SC_4,
        WC_2, WC_3, WC_4,
    };

    // For VANET Controller:
    map<string,LDid> lmap =
    {
        {"EC_2", EC_2}, {"EC_3", EC_3}, {"EC_4", EC_4},
        {"NC_2", NC_2}, {"NC_3", NC_3}, {"NC_4", NC_4},
        {"SC_2", SC_2}, {"SC_3", SC_3}, {"SC_4", SC_4},
        {"WC_2", WC_2}, {"WC_3", WC_3}, {"WC_4", WC_4}
    };

};

}

#endif
