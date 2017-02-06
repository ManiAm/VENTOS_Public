/****************************************************************************/
/// @file    LQF_NoStarv.h
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

#ifndef TRAFFICLIGHTLQFNOSTARV_H
#define TRAFFICLIGHTLQFNOSTARV_H

#include "trafficLight/TSC/03_TrafficActuated.h"

namespace VENTOS {

class TrafficLightLQF_NoStarv : public TrafficLightActuated
{
private:
    typedef TrafficLightActuated super;

    std::string currentInterval;
    double intervalDuration;
    std::string nextGreenInterval;

    omnetpp::cMessage* intervalChangeEVT = NULL;

    bool nextGreenIsNewCycle;

    typedef struct greenIntervalEntry
    {
        int maxVehCount;
        double greenTime;
        std::string greenString;
    } greenIntervalEntry_t;

    std::vector<greenIntervalEntry_t> greenInterval;

    // list of all 'incoming lanes' in each TL
    std::unordered_map< std::string /*TLid*/, std::vector<std::string> > incomingLanes_perTL;

    std::map<std::pair<std::string /*TLid*/, int /*link number*/>, std::string /*lane*/> link2Lane;

    std::vector< std::vector<int> > allMovements;

    typedef struct sortedEntry
    {
        int oneCount;
        int totalQueue;
        int maxVehCount;
        std::vector<int> batchMovements;
    } sortedEntry_t;

    typedef struct sortFunc
    {
        bool operator()(sortedEntry_t p1, sortedEntry_t p2)
        {
            if(p1.totalQueue < p2.totalQueue)
                return true;
            else if(p1.totalQueue == p2.totalQueue && p1.oneCount < p2.oneCount)
                return true;
            else
                return false;
        }
    } sortFunc_t;

    // batch of all non-conflicting movements, sorted by total queue size per batch
    typedef std::priority_queue< sortedEntry_t /*type of each element*/, std::vector<sortedEntry_t> /*container*/, sortFunc_t > priorityQ;

public:
    virtual ~TrafficLightLQF_NoStarv();
    virtual void initialize(int);
    virtual void finish();
    virtual void handleMessage(omnetpp::cMessage *);

protected:
    void virtual initialize_withTraCI();
    void virtual executeEachTimeStep();

private:
    void chooseNextInterval(std::string);
    void chooseNextGreenInterval(std::string);
    void calculatePhases(std::string);
};

}

#endif
