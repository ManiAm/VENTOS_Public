/****************************************************************************/
/// @file    IntersectionQueue.h
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

#ifndef INTERSECTIONQUEUE_H
#define INTERSECTIONQUEUE_H

#include <boost/circular_buffer.hpp>
#include <unordered_map>

#include "trafficLight/01_Base.h"

namespace VENTOS {

class IntersectionQueue : public TrafficLightBase
{
protected:
    typedef struct queueInfoRealTime
    {
        std::string TLid;
        int queueSize;
        std::vector<std::string> vehs;
    } queueInfoRealTime_t;

private:
    typedef TrafficLightBase super;

    // NED variables
    bool record_intersectionQueue_stat;
    double speedThreshold_veh;
    double speedThreshold_bike;

    // list of all traffic lights in the network
    std::vector<std::string> TLList;

    // list of all 'incoming lanes' in each TL
    std::unordered_map< std::string /*TLid*/, std::vector<std::string> > incomingLanes_perTL;

    // list of all 'side walks' in each TL
    std::unordered_map< std::string /*TLid*/, std::vector<std::string> > sideWalks_perTL;

    // all incoming lanes in all traffic lights
    std::unordered_map<std::string /*lane*/, std::string /*TLid*/> incomingLanes;

    // real-time queue size for each incoming lane in each intersection
    std::unordered_map<std::string /*lane*/, queueInfoRealTime_t> queueSize_perLane;

    typedef struct queueInfo
    {
        double time;
        int totalQueueSize;
        int maxQueueSize;
        int totalLanes;
    } queueInfo_t;

    std::map<std::string /*TLid*/, std::vector<queueInfo_t>> queueInfo_perTL;

public:
    virtual ~IntersectionQueue();
    virtual void initialize(int);
    virtual void finish();
    virtual void handleMessage(omnetpp::cMessage *);

protected:
    void virtual initialize_withTraCI();
    void virtual executeEachTimeStep();
    queueInfoRealTime_t laneGetQueueSize(std::string);

private:
    void initVariables();
    void measureQueue();
    void saveTLQueueingData();
};

}

#endif
