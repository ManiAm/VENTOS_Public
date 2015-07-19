/****************************************************************************/
/// @file    TL_MultiClass.h
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

#ifndef TRAFFICLIGHTMULTICLASS_H
#define TRAFFICLIGHTMULTICLASS_H

#include <09_TL_MaxQueue.h>

namespace VENTOS {

class TrafficLightMultiClass : public TrafficLightAdaptiveQueue
{
  public:
    virtual ~TrafficLightMultiClass();
    virtual void finish();
    virtual void initialize(int);
    virtual void handleMessage(cMessage *);

  protected:
    void virtual executeFirstTimeStep();
    void virtual executeEachTimeStep(bool);

  private:
    void findRSU(std::string);
    void chooseNextInterval();
    void chooseNextGreenInterval();

  private:
    bool greenExtension;

    enum vehTypeWeight
    {
        weight_emergency = 50,
        weight_pedestrian = 40,
        weight_bicycle = 30,
        weight_passenger = 20,
        weight_bus = 10,
        weight_truck = 1,
    };
};

}

#endif
