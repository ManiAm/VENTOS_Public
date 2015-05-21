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
#include <boost/graph/adjacency_list.hpp>

using namespace std;
using namespace boost;

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

//    bool operator== (const LoopDetectorData &v1, const LoopDetectorData &v2)
//    {
//        return (strcmp(v1.detectorName,v2.detectorName) == 0 && strcmp(v1.vehicleName, v2.vehicleName) == 0);
//    }
};


class IntersectionQueueData
{
  public:
    double time;
    char TLid[20];
    char lane[20];
    int qSize;

    IntersectionQueueData( double t, const char *str1, const char *str2, int q )
    {
        this->time = t;
        this->qSize = q;
        strcpy(this->TLid, str1);
        strcpy(this->lane, str2);
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

  private:
    void getAllDetectors();
    void collectLDsData();
    void saveLDsData();
    int findInVector(vector<LoopDetectorData *> , const char *, const char *);
    void measureTD();
    void measureQ();
    void saveQueueData();

  protected:
    bool collectInductionLoopData;
    bool measureIntersectionQueue;
    bool measureTrafficDemand;

    map<string, string> LD_demand;       // ids of loop detectors for measuring incoming traffic demand
    map<string, string> LD_actuated;     // ids of loop detectors for actuated-time signal control
    map<string, string> AD_queue;        // ids of area detectors for measuring queue length

    map<string /*lane*/, pair<string /*TLid*/,int /*queue size*/>> laneQueueSize;
    map<pair<string /*TLid*/,int /*link*/>, int /*queue size*/> linkQueueSize;

    vector<LoopDetectorData *> Vec_loopDetectors;
    vector<IntersectionQueueData *> Vec_queueSize;

  private:
    bool freeze = false;
    double lastDetectionT_old = 0;
    double total = 0;
    int passedVeh = 0;
};

}

#endif
