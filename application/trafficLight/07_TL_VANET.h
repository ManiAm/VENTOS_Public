/****************************************************************************/
/// @file    TL_VANET.h
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

#ifndef TRAFFICLIGHTVANET_H
#define TRAFFICLIGHTVANET_H

#include <06_TL_Adaptive_Webster.h>


namespace VENTOS {

class TrafficLightVANET : public TrafficLightWebster
{
  public:
    virtual ~TrafficLightVANET();
    virtual void finish();
    virtual void initialize(int);
    virtual void handleMessage(cMessage *);

  protected:
    void virtual executeFirstTimeStep();
    void virtual executeEachTimeStep(bool);

  private:
    void chooseNextInterval();
    void chooseNextGreenInterval();

  protected:
    // class variables
    std::vector<double> DetectedTime;
    cMessage* DetectEvt;
    double detectFreq = 0.1;

    double radius = 33;
  //  Coord outerRing = (100 - radius - 2;
  //  Coord innerRing  = radius + 2;

    // loop detectors id
    enum LDid
    {
        EC_2, EC_3, EC_4,
        NC_2, NC_3, NC_4,
        SC_2, SC_3, SC_4,
        WC_2, WC_3, WC_4,
    };

    // For VANET Controller:
    std::map<std::string,LDid> lmap =
    {
        {"EC_2", EC_2}, {"EC_3", EC_3}, {"EC_4", EC_4},
        {"NC_2", NC_2}, {"NC_3", NC_3}, {"NC_4", NC_4},
        {"SC_2", SC_2}, {"SC_3", SC_3}, {"SC_4", SC_4},
        {"WC_2", WC_2}, {"WC_3", WC_3}, {"WC_4", WC_4}
    };
};

}

#endif
