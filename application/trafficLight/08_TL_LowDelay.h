/****************************************************************************/
/// @file    TL_LowDelay.h
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

#ifndef TRAFFICLIGHTLOWDELAY_H
#define TRAFFICLIGHTLOWDELAY_H

#include <07_TL_TrafficActuated.h>

namespace VENTOS {

class batchMovementDelayEntry
{
public:
    int oneCount;
    int maxVehCount;
    double totalDelay;
    std::vector<int> batchMovements;

    batchMovementDelayEntry(int i1, int i2, double d1, std::vector<int> bm)
    {
        this->oneCount = i1;
        this->maxVehCount = i2;
        this->totalDelay = d1;
        batchMovements.swap(bm);
    }
};


class movementCompareDelay
{
public:
    bool operator()(batchMovementDelayEntry p1, batchMovementDelayEntry p2)
    {
        if( p1.totalDelay < p2.totalDelay )
            return true;
        else if( p1.totalDelay == p2.totalDelay && p1.oneCount < p2.oneCount)
            return true;
        else
            return false;
    }
};


class TrafficLightLowDelay : public TrafficLightActuated
{
  public:
    virtual ~TrafficLightLowDelay();
    virtual void finish();
    virtual void initialize(int);
    virtual void handleMessage(cMessage *);

  protected:
    void virtual executeFirstTimeStep();
    void virtual executeEachTimeStep(bool);

  private:
    void chooseNextInterval();
    void chooseNextGreenInterval();

  private:
    double nextGreenTime;

    // batch of all non-conflicting movements, sorted by total vehicle delay per batch
    std::priority_queue< batchMovementDelayEntry /*type of each element*/, std::vector<batchMovementDelayEntry> /*container*/, movementCompareDelay > batchMovementDelay;
};

}

#endif
