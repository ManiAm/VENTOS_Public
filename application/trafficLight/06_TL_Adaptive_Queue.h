/****************************************************************************/
/// @file    TL_AdaptiveQueue.h
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

#include <05_TL_Adaptive_RingAndBarrier.h>

namespace VENTOS {

class batchMovementQueueEntry
{
public:
    int oneCount;
    int totalQueue;
    std::vector<int> batchMovements;

    batchMovementQueueEntry(int i1, int i2, std::vector<int> bm)
    {
        this->oneCount = i1;
        this->totalQueue = i2;
        batchMovements.swap(bm);
    }
};


class movementCompare
{
public:
    bool operator()(batchMovementQueueEntry p1, batchMovementQueueEntry p2)
    {
        if( p1.totalQueue < p2.totalQueue )
            return true;
        else if( p1.totalQueue == p2.totalQueue && p1.oneCount < p2.oneCount)
            return true;
        else
            return false;
    }
};


class TrafficLightAdaptiveQueue : public TrafficLightAdaptive
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
    void getMovements();
    void generateAllAllowedMovements();

private:
    int LINKSIZE;
    boost::filesystem::path movementsFilePath;
    std::vector< std::vector<int> > allMovements;

    // batch of all non-conflicting movements, sorted by total queue size per batch
    std::priority_queue< batchMovementQueueEntry /*type of each element*/, std::vector<batchMovementQueueEntry> /*container*/, movementCompare > batchMovementQueue;
};

}

#endif
