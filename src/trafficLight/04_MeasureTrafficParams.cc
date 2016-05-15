/****************************************************************************/
/// @file    MeasureTrafficParams.cc
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

#include <04_MeasureTrafficParams.h>
#include <iomanip>

namespace VENTOS {

Define_Module(VENTOS::MeasureTrafficParams);


MeasureTrafficParams::~MeasureTrafficParams()
{

}


void MeasureTrafficParams::initialize(int stage)
{
    super::initialize(stage);

    if(stage == 0)
    {
        measureIntersectionQueue = par("measureIntersectionQueue").boolValue();
        measureTrafficDemand = par("measureTrafficDemand").boolValue();
        measureTrafficDemandMode = par("measureTrafficDemandMode").longValue();
        trafficDemandBuffSize = par("trafficDemandBuffSize").longValue();

        collectTLPhasingData = par("collectTLPhasingData").boolValue();
        collectTLQueuingData = par("collectTLQueuingData").boolValue();

        // we need to turn on measureIntersectionQueue
        if(collectTLQueuingData || collectTLPhasingData)
            measureIntersectionQueue = true;

        LD_demand.clear();
        LD_actuated.clear();
        AD_queue.clear();

        laneQueueSize.clear();
        linkQueueSize.clear();
        queueSizeTL.clear();

        laneTD.clear();
        linkTD.clear();
        laneTotalVehCount.clear();

        Vec_queueSize.clear();
    }
}


void MeasureTrafficParams::finish()
{
    super::finish();

    if(collectTLQueuingData)
        saveTLQueueingData();
}


void MeasureTrafficParams::handleMessage(omnetpp::cMessage *msg)
{
    super::handleMessage(msg);
}


void MeasureTrafficParams::initialize_withTraCI()
{
    super::initialize_withTraCI();

    // for each traffic light
    for (auto &it : TLList)
    {
        std::string TLid = it;

        // get all incoming lanes
        std::list<std::string> lan = TraCI->TLGetControlledLanes(it);

        // remove duplicate entries
        lan.unique();

        queueDataEntry *entry = new queueDataEntry(0, 0, std::numeric_limits<int>::max(), lan.size());
        queueSizeTL.insert( std::make_pair(TLid, *entry) );

        // for each incoming lane
        for(auto &it2 : lan)
        {
            std::string lane = it2;

            // initialize queue value in laneQueueSize to zero
            laneQueueSize[lane] = std::make_pair(TLid, 0);

            boost::circular_buffer<std::vector<double>> CB;   // create a circular buffer
            CB.set_capacity(trafficDemandBuffSize);         // set max capacity
            CB.clear();
            laneTD[lane] = std::make_pair(TLid, CB);  // initialize laneTD

            // initialize laneTotalVehCount
            laneVehInfo *entry = new laneVehInfo(-1, -1, 0);
            laneTotalVehCount.insert( std::make_pair(lane, std::make_pair(TLid, *entry)) );
        }

        // get all links controlled by this TL
        std::map<int,std::vector<std::string>> result = TraCI->TLGetControlledLinks(TLid);

        // for each link in this TLid
        for(auto &it2 : result)
        {
            int linkNumber = it2.first;

            boost::circular_buffer<std::vector<double>> CB;   // create a circular buffer
            CB.set_capacity(trafficDemandBuffSize);         // set max capacity
            CB.clear();
            linkTD[std::make_pair(TLid,linkNumber)] = CB;   // initialize linkTD

            // initialize queue value in linkQueueSize to zero
            linkQueueSize.insert( std::make_pair(std::make_pair(TLid,linkNumber), 0) );
        }
    }

    CheckDetectors();

    // todo: change this later
    // saturation = (3*TD) / ( 1-(35/cycle) )
    // max TD = 1900, max cycle = 120
    saturationTD = 2000.;
}


void MeasureTrafficParams::executeEachTimeStep()
{
    super::executeEachTimeStep();

    if(measureIntersectionQueue || measureTrafficDemand)
        measureTrafficParameters();
}


void MeasureTrafficParams::CheckDetectors()
{
    // get all loop detectors
    std::list<std::string> str = TraCI->LDGetIDList();

    // for each loop detector
    for (auto &it : str)
    {
        std::string lane = TraCI->LDGetLaneID(it);

        if( std::string(it).find("demand_") != std::string::npos )
            LD_demand[lane] = std::make_pair(it,-1);
        else if( std::string(it).find("actuated_") != std::string::npos )
            LD_actuated[lane] = it;
    }

    // get all area detectors
    std::list<std::string> str2 = TraCI->LADGetIDList();

    // for each area detector detector
    for (auto &it : str2)
    {
        std::string lane = TraCI->LADGetLaneID(it);

        if( std::string(it).find("queue_") != std::string::npos )
            AD_queue[lane] = it;
    }

    INFO_LOG << boost::format("\n>>> %1% loop detectors are added out of which: \n") % str.size();
    INFO_LOG << boost::format("  %1% are demand loop detectors \n") % LD_demand.size();
    INFO_LOG << boost::format("  %1% are actuated loop detectors \n") % LD_actuated.size();

    INFO_LOG << boost::format("\n>>> %1% area detectors are added out of which: \n") % str2.size();
    INFO_LOG << boost::format("  %1% are used for queue measurement \n") % AD_queue.size();

    FLUSH_LOG;

    // make sure we have all detectors we need
    for (auto &it : TLList)
    {
        std::string TLid = it;

        std::list<std::string> lan = TraCI->TLGetControlledLanes(TLid);

        // remove duplicate entries
        lan.unique();

        // for each incoming lane
        for(auto &it2 : lan)
        {
            std::string lane = it2;

            // ignore side walk
            if( std::find(sideWalkListTL[TLid].begin(), sideWalkListTL[TLid].end(), lane) != sideWalkListTL[TLid].end() )
                continue;

            // traffic-actuated TSC needs one actuated LD on each incoming lane
            if( TLControlMode == TL_TrafficActuated && LD_actuated.find(lane) == LD_actuated.end() )
                WARNING_LOG << boost::format("WARNING: no loop detector found on lane (%1%). No actuation is available for this lane. \n") % lane;

            // if we are measuring queue length then make sure we have an area detector in each lane
            if( measureIntersectionQueue && AD_queue.find(lane) == AD_queue.end() )
                WARNING_LOG << boost::format("WARNING: no area detector found on lane (%1%). No queue measurement is available for this lane. \n") % lane;

            // if we are measuring traffic demand using loop detectors then make sure we have an LD on each lane
            if( measureTrafficDemand && LD_demand.find(lane) == LD_demand.end() )
                WARNING_LOG << boost::format("WARNING: no loop detector found on lane (%1%). No traffic demand measurement is available for this lane. \n") % lane;
        }
    }
}


void MeasureTrafficParams::measureTrafficParameters()
{
    // for each 'lane i' that is controlled by traffic light j
    for(auto &y : allIncomingLanes)
    {
        std::string lane = y.first;
        std::string TLid = y.second;

        // ignore side walk
        if( std::find(sideWalkListTL[TLid].begin(), sideWalkListTL[TLid].end(), lane) != sideWalkListTL[TLid].end() )
            continue;

        if(measureIntersectionQueue)
        {
            // make sure we have an area detector in this lane
            if( AD_queue.find(lane) == AD_queue.end() )
                continue;

            std::string ADid = AD_queue[lane];
            int q = TraCI->LADGetLastStepVehicleHaltingNumber(ADid);

            // update laneQueueSize
            auto location = laneQueueSize.find(lane);
            std::pair<std::string,int> store = location->second;
            location->second = std::make_pair(store.first, q);

            if(collectTLQueuingData)
            {
                auto g = queueSizeTL.find(TLid);

                // save min/max/total queue size for each TLid temporarily
                (*g).second.totalQueueSize = (*g).second.totalQueueSize + q;
                (*g).second.maxQueueSize = std::max((*g).second.maxQueueSize, q);
            }

            // get # of outgoing links from this lane
            int NoLinks = outgoingLinks.count(lane);

            // iterate over outgoing links
            auto ppp = outgoingLinks.equal_range(lane);
            for(std::multimap<std::string, std::pair<std::string, int>>::iterator z = ppp.first; z != ppp.second; ++z)
            {
                int linkNumber = (*z).second.second;

                // each link gets a portion of the queue size (todo: turn it off by now)
                //int queuePortion = ceil( (double)q / (double)NoLinks );
                int queuePortion = q;

                // update linkQueueSize
                auto location = linkQueueSize.find( make_pair(TLid,linkNumber) );
                location->second = queuePortion;
            }
        }

        // traffic demand measurement using induction loops
        if(measureTrafficDemand)
        {
            // make sure we have a demand loop detector in this lane
            auto loc = LD_demand.find(lane);
            if( loc == LD_demand.end() )
                continue;

            std::string lane = loc->first;
            std::string LDid = loc->second.first;
            double lastDetection_old = loc->second.second;   // lastDetection_old is one step behind lastDetection
            double lastDetection = TraCI->LDGetElapsedTimeLastDetection(LDid);

            // lastDetection == 0        if a vehicle is above the LD
            // lastDetection_old != 0    if this is the first detection for this vehicle (we ignore any subsequent detections for the same vehicle)
            if(lastDetection == 0 && lastDetection_old != 0)
            {
                double TD = 0;

                // traffic measurement is done by measuring headway time between each two consecutive vehicles
                if(measureTrafficDemandMode == 1)
                {
                    double diff = omnetpp::simTime().dbl() - lastDetection_old - updateInterval;

                    // ignore the very first detection on this LD
                    if(diff > 0.0001)
                    {
                        // calculate the instantaneous traffic demand
                        TD = 3600. / lastDetection_old;

                        // bound TD
                        TD = std::min(TD, saturationTD);

                        // calculate lagT: when the measured TD will be effective?
                        // measured TD does not represent the condition in the intersection, and is effective after lagT
                        double LDPos = TraCI->laneGetLength(lane) - TraCI->LDGetPosition(LDid);  // get position of the LD from end of lane
                        double approachSpeed = TraCI->LDGetLastStepMeanVehicleSpeed(LDid);
                        double lagT = std::fabs(LDPos) / approachSpeed;

                        // push it into the circular buffer
                        auto loc = laneTD.find(lane);
                        std::vector<double> entry {TD /*traffic demand*/, omnetpp::simTime().dbl() /*time of measure*/, lagT /*time it takes to arrive at intersection*/};
                        (loc->second).second.push_back(entry);

                        // iterate over outgoing links
                        auto ppp = outgoingLinks.equal_range(lane);
                        for(std::multimap<std::string, std::pair<std::string, int>>::iterator z = ppp.first; z != ppp.second; ++z)
                        {
                            int linkNumber = (*z).second.second;

                            // push the new TD into the circular buffer
                            auto location = linkTD.find( make_pair(TLid,linkNumber) );
                            std::vector<double> entry {TD /*traffic demand*/, omnetpp::simTime().dbl() /*time of measure*/, lagT /*time it takes to arrive at intersection*/};
                            (location->second).push_back(entry);
                        }
                    }
                }
                // traffic demand measurement is done by counting total # of passed vehicles in interval
                // Note that updating laneTD and laneLinks is done at the beginning of each cycle at updateTLstate method
                else if(measureTrafficDemandMode == 2)
                {
                    auto u = laneTotalVehCount.find(lane);

                    if(u == laneTotalVehCount.end())
                        throw omnetpp::cRuntimeError("lane %s does not exist in laneTotalVehCount", lane.c_str());

                    (*u).second.second.totalVehCount = (*u).second.second.totalVehCount + 1;

                    // if this is the first vehicle on this lane
                    if((*u).second.second.totalVehCount == 1)
                        (*u).second.second.firstArrivalTime = omnetpp::simTime().dbl();

                    // last detection time
                    (*u).second.second.lastArrivalTime = omnetpp::simTime().dbl();
                }
            }

            // update lastDetection in this LD
            loc->second.second = lastDetection;
        }
    }

    if(collectTLQueuingData)
    {
        for(auto &y : queueSizeTL)
        {
            std::string TLid = y.first;
            int totalQueue = y.second.totalQueueSize;
            int maxQueue = y.second.maxQueueSize;
            int laneCount = y.second.totalLanes;

            queueDataEntryDetailed *entry = new queueDataEntryDetailed(omnetpp::simTime().dbl(), TLid, totalQueue, maxQueue, laneCount);
            Vec_queueSize.push_back(*entry);

            // reset values
            y.second.totalQueueSize = 0;
            y.second.maxQueueSize = 0;
        }
    }
}


// update traffic demand for each lane at the beginning of each cycle
// this method is called only when measureTrafficDemandMode == 2
void MeasureTrafficParams::updateTrafficDemand()
{
    if(!measureTrafficDemand)
        return;

    if(measureTrafficDemandMode != 2)
        return;

    for(auto & u : laneTotalVehCount)
    {
        std::string lane = u.first;
        std::string TLid = u.second.first;

        // ignore side walk
        if( std::find(sideWalkListTL[TLid].begin(), sideWalkListTL[TLid].end(), lane) != sideWalkListTL[TLid].end() )
            continue;

        double interval = u.second.second.lastArrivalTime - u.second.second.firstArrivalTime;

        double TD = (interval == 0) ? 0 : 3600. * ( (double)u.second.second.totalVehCount / interval );

        // bound TD
        TD = std::min(TD, saturationTD);

        // if interval is too big then clear the buffer and restart!
        if(interval >= 200)
        {
            u.second.second.totalVehCount = 0;
            u.second.second.firstArrivalTime = omnetpp::simTime().dbl();

            // clear buffer for this lane in laneTD
            const auto &loc = laneTD.find(lane);
            (loc->second).second.clear();

            // clear buffer for this lane in linkTD
            const auto &ppp = outgoingLinks.equal_range(lane);
            for(std::multimap<std::string, std::pair<std::string, int>>::iterator z = ppp.first; z != ppp.second; ++z)
            {
                int linkNumber = (*z).second.second;

                // push the new TD into the circular buffer
                const auto &location = linkTD.find( make_pair(TLid,linkNumber) );
                (location->second).clear();
            }

            DEBUG_LOG << boost::format(">>> Traffic demand measurement restarted for lane %1% \n\n ") % lane << std::flush;
        }

        if(TD != 0)
        {
            // push it into the circular buffer
            auto loc = laneTD.find(lane);
            std::vector<double> entry {TD /*traffic demand*/, omnetpp::simTime().dbl() /*time of measure*/, -1};
            (loc->second).second.push_back(entry);

            // iterate over outgoing links
            auto ppp = outgoingLinks.equal_range(lane);
            for(std::multimap<std::string, std::pair<std::string, int>>::iterator z = ppp.first; z != ppp.second; ++z)
            {
                int linkNumber = (*z).second.second;

                // push the new TD into the circular buffer
                auto location = linkTD.find( make_pair(TLid,linkNumber) );
                std::vector<double> entry {TD /*traffic demand*/, omnetpp::simTime().dbl() /*time of measure*/, -1};
                (location->second).push_back(entry);
            }
        }
    }
}


void MeasureTrafficParams::saveTLQueueingData()
{
    if(Vec_queueSize.empty())
        return;

    boost::filesystem::path filePath;

    if(omnetpp::cSimulation::getActiveEnvir()->isGUI())
    {
        filePath = "results/gui/TLqueuingData.txt";
    }
    else
    {
        // get the current run number
        int currentRun = omnetpp::getEnvir()->getConfigEx()->getActiveRunNumber();
        std::ostringstream fileName;
        fileName << std::setfill('0') << std::setw(3) << currentRun << "_TLqueuingData.txt";
        filePath = "results/cmd/" + fileName.str();
    }

    FILE *filePtr = fopen (filePath.string().c_str(), "w");

    // write simulation parameters at the beginning of the file in CMD mode
    if(!omnetpp::cSimulation::getActiveEnvir()->isGUI())
    {
        // get the current config name
        std::string configName = omnetpp::getEnvir()->getConfigEx()->getVariable("configname");

        // get number of total runs in this config
        int totalRun = omnetpp::getEnvir()->getConfigEx()->getNumRunsInConfig(configName.c_str());

        // get the current run number
        int currentRun = omnetpp::getEnvir()->getConfigEx()->getActiveRunNumber();

        // get all iteration variables
        std::vector<std::string> iterVar = omnetpp::getEnvir()->getConfigEx()->unrollConfig(configName.c_str(), false);

        // write to file
        fprintf (filePtr, "configName      %s\n", configName.c_str());
        fprintf (filePtr, "totalRun        %d\n", totalRun);
        fprintf (filePtr, "currentRun      %d\n", currentRun);
        fprintf (filePtr, "currentConfig   %s\n\n\n", iterVar[currentRun].c_str());
    }

    // write header
    fprintf (filePtr, "%-10s", "index");
    fprintf (filePtr, "%-10s", "timeStep");
    fprintf (filePtr, "%-10s", "TLid");
    fprintf (filePtr, "%-15s", "totalQueue");
    fprintf (filePtr, "%-15s", "maxQueue");
    fprintf (filePtr, "%-10s\n\n", "laneCount");

    double oldTime = -1;
    int index = 0;

    for(auto &y : Vec_queueSize)
    {
        if(oldTime != y.time)
        {
            fprintf(filePtr, "\n");
            oldTime = y.time;
            index++;
        }

        fprintf (filePtr, "%-10d", index);
        fprintf (filePtr, "%-10.2f", y.time);
        fprintf (filePtr, "%-10s", y.TLid.c_str());
        fprintf (filePtr, "%-15d", y.totalQueueSize);
        fprintf (filePtr, "%-15d", y.maxQueueSize);
        fprintf (filePtr, "%-10d\n", y.totalLanes);
    }

    fclose(filePtr);
}

}
