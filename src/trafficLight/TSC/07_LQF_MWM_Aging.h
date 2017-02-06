/****************************************************************************/
/// @file    LQF_MWM_Aging.h
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

#ifndef TRAFFICLIGHTLQFMWMAGING_H
#define TRAFFICLIGHTLQFMWMAGING_H

#include <queue>

#include "trafficLight/TSC/06_LQF_MWM.h"

namespace VENTOS {

class TrafficLight_LQF_MWM_Aging : public TrafficLight_LQF_MWM
{
private:
    typedef TrafficLight_LQF_MWM super;

    std::string currentInterval;
    double intervalDuration;
    std::string nextGreenInterval;

    omnetpp::cMessage* intervalChangeEVT = NULL;

    double nextGreenTime;

    std::vector<std::string> phases = {phase1_5, phase2_6, phase3_7, phase4_8};

    std::map<std::string /*TLid*/, std::string /*first green interval*/> firstGreen;

    std::map<std::pair<std::string /*TLid*/, int /*link*/>, std::string /*lane*/> link2Lane;

    // list of all 'incoming lanes' in each TL
    std::unordered_map< std::string /*TLid*/, std::vector<std::string> > incomingLanes_perTL;

    typedef struct sortedEntry_weight
    {
        double totalWeight;
        int oneCount;
        int maxVehCount;
        std::string phase;
    } sortedEntry_weight_t;

    typedef struct sortFunc_weight
    {
        bool operator()(sortedEntry_weight_t p1, sortedEntry_weight_t p2)
        {
            if( p1.totalWeight < p2.totalWeight )
                return true;
            else if( p1.totalWeight == p2.totalWeight && p1.oneCount < p2.oneCount)
                return true;
            else
                return false;
        }
    } sortFunc_weight_t;

    // batch of all non-conflicting movements, sorted by total weight + oneCount per batch
    typedef std::priority_queue< sortedEntry_weight_t /*type of each element*/, std::vector<sortedEntry_weight_t> /*container*/, sortFunc_weight_t > priorityQ_weight;

    typedef struct sortedEntry_delay
    {
        double totalWeight;
        int oneCount;
        int maxVehCount;
        double maxDelay;
        std::string phase;
    } sortedEntry_delay_t;

    typedef struct sortFunc_delay
    {
        bool operator()(sortedEntry_delay_t n1, sortedEntry_delay_t n2)
        {
            if (n1.maxDelay < n2.maxDelay)
                return true;
            else if (n1.maxDelay == n2.maxDelay && n1.totalWeight < n2.totalWeight)
                return true;
            else if (n1.maxDelay == n2.maxDelay && n1.totalWeight == n2.totalWeight && n1.oneCount < n2.oneCount)
                return true;
            else
                return false;
        }
    } sortFunc_delay_t;

    // second priority queue to sort the maximum delay in each phase
    typedef std::priority_queue< sortedEntry_delay_t, std::vector<sortedEntry_delay_t>, sortFunc_delay_t > priorityQ_delay;

public:
    virtual ~TrafficLight_LQF_MWM_Aging();
    virtual void initialize(int);
    virtual void finish();
    virtual void handleMessage(omnetpp::cMessage *);

protected:
    void virtual initialize_withTraCI();
    void virtual executeEachTimeStep();

private:
    void chooseNextInterval(std::string);
    void chooseNextGreenInterval(std::string);
};

}

#endif
