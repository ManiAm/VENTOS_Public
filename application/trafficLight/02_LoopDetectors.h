/****************************************************************************/
/// @file    LoopDetectors.h
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author
/// @date    April 2015
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

#ifndef LOOPDETECTORS_H
#define LOOPDETECTORS_H

#include <01_TL_Base.h>
#include <Appl.h>
#include "TraCI_Extend.h"

using namespace std;

namespace VENTOS {

class LoopDetectorData
{
  public:
    char detectorName[20];
    char vehicleName[20];
    char lane[20];
    double entryTime;
    double leaveTime;
    double entrySpeed;
    double leaveSpeed;

    LoopDetectorData( const char *str1, const char *str2, const char *str3, double entryT, double leaveT, double entryS, double leaveS )
    {
        strcpy(this->detectorName, str1);
        strcpy(this->lane, str2);
        strcpy(this->vehicleName, str3);

        this->entryTime = entryT;
        this->leaveTime = leaveT;

        this->entrySpeed = entryS;
        this->leaveSpeed = leaveS;
    }
};


class LaneQueueEntry
{
public:
    char vehicleName[20];
    double entryT;
    double exitT;

    LaneQueueEntry(const char *str1, double t1, double t2)
    {
        strcpy(this->vehicleName, str1);
        entryT = t1;
        exitT = t2;
    }
};


class LoopDetectors : public TrafficLightBase
{
  public:
    virtual ~LoopDetectors();
    virtual void finish();
    virtual void initialize(int);
    virtual void handleMessage(cMessage *);

  protected:
    void virtual executeFirstTimeStep();
    void virtual executeEachTimeStep(bool);

  protected:
    list<string> LDList;   // list of loop-detectors in the network

    deque<LaneQueueEntry *> queue_C_NC_2;
    deque<LaneQueueEntry *> queue_C_NC_3;
    deque<LaneQueueEntry *> queue_C_NC_4;

    deque<LaneQueueEntry *> queue_C_SC_2;
    deque<LaneQueueEntry *> queue_C_SC_3;
    deque<LaneQueueEntry *> queue_C_SC_4;

    deque<LaneQueueEntry *> queue_C_WC_2;
    deque<LaneQueueEntry *> queue_C_WC_3;
    deque<LaneQueueEntry *> queue_C_WC_4;

    deque<LaneQueueEntry *> queue_C_EC_2;
    deque<LaneQueueEntry *> queue_C_EC_3;
    deque<LaneQueueEntry *> queue_C_EC_4;

  private:
    void inductionLoops();
    void inductionLoopToFile();
    int findInVector(vector<LoopDetectorData *> , const char *, const char *);

    bool collectInductionLoopData;
    bool collectIntersectionQueue;
    vector<LoopDetectorData *> Vec_loopDetectors;
};

}

#endif
