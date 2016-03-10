/****************************************************************************/
/// @file    LoopDetectors.cc
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

#include <02_LoopDetectors.h>
#include <iomanip>

namespace VENTOS {

Define_Module(VENTOS::LoopDetectors);


LoopDetectors::~LoopDetectors()
{

}


void LoopDetectors::initialize(int stage)
{
    TrafficLightBase::initialize(stage);

    if(stage == 0)
    {
        minGreenTime = par("minGreenTime").doubleValue();
        maxGreenTime = par("maxGreenTime").doubleValue();
        yellowTime = par("yellowTime").doubleValue();
        redTime = par("redTime").doubleValue();
        maxCycleLength = par("maxCycleLength").doubleValue();

        if(minGreenTime <= 0)
            error("minGreenTime value is wrong!");
        if(maxGreenTime <= 0 || maxGreenTime < minGreenTime)
            error("maxGreenTime value is wrong!");
        if(yellowTime <= 0)
            error("yellowTime value is wrong!");
        if(redTime <= 0)
            error("redTime value is wrong!");
        if( maxCycleLength < (minGreenTime + yellowTime + redTime) )
            error("maxCycleLength value is wrong!");

        measureIntersectionQueue = par("measureIntersectionQueue").boolValue();
        measureTrafficDemand = par("measureTrafficDemand").boolValue();
        measureTrafficDemandMode = par("measureTrafficDemandMode").longValue();
        trafficDemandBuffSize = par("trafficDemandBuffSize").longValue();

        collectInductionLoopData = par("collectInductionLoopData").boolValue();
        collectTLQueuingData = par("collectTLQueuingData").boolValue();
        collectTLPhasingData = par("collectTLPhasingData").boolValue();

        // we need to turn on measureIntersectionQueue
        if(collectTLQueuingData || collectTLPhasingData)
            measureIntersectionQueue = true;

        LD_demand.clear();
        LD_actuated.clear();
        AD_queue.clear();

        laneListTL.clear();
        bikeLaneListTL.clear();
        sideWalkListTL.clear();

        laneList.clear();
        laneLinks.clear();

        laneQueueSize.clear();
        linkQueueSize.clear();
        queueSizeTL.clear();

        laneTD.clear();
        linkTD.clear();
        laneTotalVehCount.clear();

        phaseTL.clear();
        statusTL.clear();

        Vec_loopDetectors.clear();
        Vec_queueSize.clear();
    }
}


void LoopDetectors::finish()
{
    TrafficLightBase::finish();

    if(collectInductionLoopData)
        saveLDsData();

    if(collectTLQueuingData)
        saveTLQueueingData();

    if(collectTLPhasingData)
        saveTLPhasingData();
}


void LoopDetectors::handleMessage(cMessage *msg)
{
    TrafficLightBase::handleMessage(msg);
}


void LoopDetectors::executeFirstTimeStep()
{
    TrafficLightBase::executeFirstTimeStep();

    TLList = TraCI->TLGetIDList();

    // for each traffic light
    for (auto &it : TLList)
    {
        std::string TLid = it;

        // get all incoming lanes
        std::list<std::string> lan = TraCI->TLGetControlledLanes(it);

        // remove duplicate entries
        lan.unique();

        laneListTL[TLid] = std::make_pair(lan.size(),lan);

        queueDataEntry *entry = new queueDataEntry(0, 0, std::numeric_limits<int>::max(), lan.size());
        queueSizeTL.insert( std::make_pair(TLid, *entry) );

        std::list<std::string> bikeLaneList;
        std::list<std::string> sideWalkList;

        // for each incoming lane
        for(auto &it2 : lan)
        {
            std::string lane = it2;

            laneList[lane] = TLid;

            // store all bike lanes and side walks
            std::list<std::string> allowedClasses = TraCI->laneGetAllowedClasses(lane);
            if(allowedClasses.size() == 1 && allowedClasses.front() == "bicycle")
                bikeLaneList.push_back(lane);
            else if(allowedClasses.size() == 1 && allowedClasses.front() == "pedestrian")
                sideWalkList.push_back(lane);

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

        bikeLaneListTL[TLid] = bikeLaneList;
        sideWalkListTL[TLid] = sideWalkList;
        bikeLaneList.clear();
        sideWalkList.clear();

        // get all links controlled by this TL
        std::map<int,std::vector<std::string>> result = TraCI->TLGetControlledLinks(TLid);

        // for each link in this TLid
        for(auto &it2 : result)
        {
            int linkNumber = it2.first;
            std::vector<std::string> link = it2.second;
            std::string incommingLane = link[0];

            laneLinks.insert( std::make_pair(incommingLane, std::make_pair(TLid,linkNumber)) );

            linkLane.insert( std::make_pair(std::make_pair(TLid,linkNumber), incommingLane) );

            boost::circular_buffer<std::vector<double>> CB;   // create a circular buffer
            CB.set_capacity(trafficDemandBuffSize);         // set max capacity
            CB.clear();
            linkTD[std::make_pair(TLid,linkNumber)] = CB;   // initialize linkTD

            // initialize queue value in linkQueueSize to zero
            linkQueueSize.insert( std::make_pair(std::make_pair(TLid,linkNumber), 0) );
        }
    }

    getAllDetectors();

    // todo: change this later
    // saturation = (3*TD) / ( 1-(35/cycle) )
    // max TD = 1900, max cycle = 120
    saturationTD = 2000.;
}


void LoopDetectors::executeEachTimeStep()
{
    TrafficLightBase::executeEachTimeStep();

    if(collectInductionLoopData)
        collectLDsData();    // collecting induction loop data in each timeStep

    if(measureIntersectionQueue || measureTrafficDemand)
        measureTrafficParameters();
}


void LoopDetectors::getAllDetectors()
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

    if(ev.isGUI() && debugLevel > 0)
    {
        std::cout << LD_demand.size() << " demand loop detectors found!" << endl;
        std::cout << LD_actuated.size() << " actuated loop detectors found!" << endl;
        std::cout << AD_queue.size() << " area detectors found!" << endl << endl;
        std::cout.flush();
    }

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
                std::cout << "WARNING: no loop detector found on lane (" << lane << "). No actuation is available for this lane." << endl;

            // if we are measuring queue length then make sure we have an area detector in each lane
            if( measureIntersectionQueue && AD_queue.find(lane) == AD_queue.end() )
                std::cout << "WARNING: no area detector found on lane (" << lane << "). No queue measurement is available for this lane." << endl;

            // if we are measuring traffic demand using loop detectors then make sure we have an LD on each lane
            if( measureTrafficDemand && LD_demand.find(lane) == LD_demand.end() )
                std::cout << "WARNING: no loop detector found on lane (" << lane << "). No traffic demand measurement is available for this lane." << endl;
        }
    }
}


void LoopDetectors::collectLDsData()
{
    // get all loop detectors
    std::list<std::string> str = TraCI->LDGetIDList();

    // for each loop detector
    for (auto &it : str)
    {
        std::vector<std::string>  st = TraCI->LDGetLastStepVehicleData(it);

        // only if this loop detector detected a vehicle
        if( st.size() > 0 )
        {
            // laneID of loop detector
            std::string lane = TraCI->LDGetLaneID(it);

            // get vehicle information
            std::string vehicleName = st.at(0);
            double entryT = atof( st.at(2).c_str() );
            double leaveT = atof( st.at(3).c_str() );
            double speed = TraCI->LDGetLastStepMeanVehicleSpeed(it);  // vehicle speed at current moment

            // save it only when collectInductionLoopData=true
            if(collectInductionLoopData)
            {
                const LoopDetectorData *searchFor = new LoopDetectorData( it.c_str(), "", vehicleName.c_str() );
                auto counter = std::find(Vec_loopDetectors.begin(), Vec_loopDetectors.end(), *searchFor);

                // its a new entry, so we add it
                if(counter == Vec_loopDetectors.end())
                {
                    LoopDetectorData *tmp = new LoopDetectorData( it.c_str(), lane.c_str(), vehicleName.c_str(), entryT, leaveT, speed, speed );
                    Vec_loopDetectors.push_back(*tmp);
                }
                // if found, just update leaveTime and leaveSpeed
                else
                {
                    counter->leaveTime = leaveT;
                    counter->leaveSpeed = speed;
                }
            }
        }
    }
}


void LoopDetectors::saveLDsData()
{
    boost::filesystem::path filePath;

    if(ev.isGUI())
    {
        filePath = "results/gui/loopDetector.txt";
    }
    else
    {
        // get the current run number
        int currentRun = ev.getConfigEx()->getActiveRunNumber();
        std::ostringstream fileName;
        fileName << std::setfill('0') << std::setw(3) << currentRun << "_loopDetector.txt";
        filePath = "results/cmd/" + fileName.str();
    }

    FILE *filePtr = fopen (filePath.string().c_str(), "w");

    // write simulation parameters at the beginning of the file in CMD mode
    if(!ev.isGUI())
    {
        // get the current config name
        std::string configName = ev.getConfigEx()->getVariable("configname");

        // get number of total runs in this config
        int totalRun = ev.getConfigEx()->getNumRunsInConfig(configName.c_str());

        // get the current run number
        int currentRun = ev.getConfigEx()->getActiveRunNumber();

        // get all iteration variables
        std::vector<std::string> iterVar = ev.getConfigEx()->unrollConfig(configName.c_str(), false);

        // write to file
        fprintf (filePtr, "configName      %s\n", configName.c_str());
        fprintf (filePtr, "totalRun        %d\n", totalRun);
        fprintf (filePtr, "currentRun      %d\n", currentRun);
        fprintf (filePtr, "currentConfig   %s\n\n\n", iterVar[currentRun].c_str());
    }

    // write header
    fprintf (filePtr, "%-30s","loopDetector");
    fprintf (filePtr, "%-20s","lane");
    fprintf (filePtr, "%-20s","vehicleName");
    fprintf (filePtr, "%-20s","vehicleEntryTime");
    fprintf (filePtr, "%-20s","vehicleLeaveTime");
    fprintf (filePtr, "%-22s","vehicleEntrySpeed");
    fprintf (filePtr, "%-22s\n\n","vehicleLeaveSpeed");

    // write body
    for(auto& y : Vec_loopDetectors)
    {
        fprintf (filePtr, "%-30s ", y.detectorName.c_str());
        fprintf (filePtr, "%-20s ", y.lane.c_str());
        fprintf (filePtr, "%-20s ", y.vehicleName.c_str());
        fprintf (filePtr, "%-20.2f ", y.entryTime);
        fprintf (filePtr, "%-20.2f ", y.leaveTime);
        fprintf (filePtr, "%-20.2f ", y.entrySpeed);
        fprintf (filePtr, "%-20.2f\n", y.leaveSpeed);
    }

    fclose(filePtr);
}


void LoopDetectors::updateTLstate(std::string TLid, std::string stage, std::string currentInterval, bool cycleStart)
{
    if(stage == "init")
    {
        // initialize phase number in this TL
        phaseTL[TLid] = 1;

        // Initialize status in this TL
        currentStatusTL *entry = new currentStatusTL(1 /*cycle number*/, currentInterval, -1, simTime().dbl(), -1, -1, -1, laneListTL[TLid].first, -1);
        statusTL.insert( std::make_pair(std::make_pair(TLid,1), *entry) );
    }
    else
    {
        // get a reference to this TL
        auto location = statusTL.find( std::make_pair(TLid,phaseTL[TLid]) );
        if(location == statusTL.end())
            error("This TLid is not found!");

        if(stage == "yellow")
        {
            (location->second).yellowStart = simTime().dbl();
            (location->second).greenLength = (location->second).yellowStart - (location->second).greenStart;

            // get green duration
            double green_duration = (location->second).yellowStart - (location->second).greenStart;

            //            // todo: make sure green interval is above G_min
            //            if(green_duration - minGreenTime < 0)
            //                error("green interval is less than minGreenTime = %0.3f", minGreenTime);
            //
            //            // make sure green interval is below G_max
            //            if(green_duration - maxGreenTime > 0)
            //                error("green interval is greater than maxGreenTime = %0.3f", maxGreenTime);
        }
        else if(stage == "red")
        {
            (location->second).redStart = simTime().dbl();

            // get yellow duration
            double yellow_duration = (location->second).redStart - (location->second).yellowStart;

            // todo:
            //            if( fabs(yellow_duration - yellowTime) < 0.0001 )
            //                error("yellow interval is not %0.3f", yellowTime);
        }
        else if(stage == "phaseEnd")
        {
            // update TL status for this phase
            (location->second).phaseEnd = simTime().dbl();

            // get red duration
            double red_duration = (location->second).phaseEnd - (location->second).redStart;

            // todo:
            //            if(red_duration - redTime != 0)
            //                error("red interval is not %0.3f", redTime);

            // get all incoming lanes for this TLid
            std::list<std::string> lan = laneListTL[TLid].second;

            // for each incoming lane
            int totalQueueSize = 0;
            for(auto &it2 : lan)
            {
                totalQueueSize = totalQueueSize + laneQueueSize[it2].second;
            }

            (location->second).totalQueueSize = totalQueueSize;

            // get the current cycle number
            int cycleNumber = (location->second).cycle;

            // on a new cycle
            if(cycleStart)
            {
                // todo: check if cycle length is ok?

                // increase cycle number by 1
                cycleNumber++;

                // update traffic demand at the begining of each cycle
                if(measureTrafficDemand && measureTrafficDemandMode == 2)
                    updateTrafficDemand();
            }

            // increase phase number by 1
            auto location2 = phaseTL.find(TLid);
            location2->second = location2->second + 1;

            // update status for the new phase
            currentStatusTL *entry = new currentStatusTL(cycleNumber, currentInterval, -1, simTime().dbl(), -1, -1, -1, lan.size(), -1);
            statusTL.insert( std::make_pair(std::make_pair(TLid,location2->second), *entry) );
        }
        else error("stage is not recognized!");
    }
}


void LoopDetectors::measureTrafficParameters()
{
    // for each 'lane i' that is controlled by traffic light j
    for(auto &y : laneList)
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
            int NoLinks = laneLinks.count(lane);

            // iterate over outgoing links
            auto ppp = laneLinks.equal_range(lane);
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
                    double diff = simTime().dbl() - lastDetection_old - updateInterval;

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
                        std::vector<double> entry {TD /*traffic demand*/, simTime().dbl() /*time of measure*/, lagT /*time it takes to arrive at intersection*/};
                        (loc->second).second.push_back(entry);

                        // iterate over outgoing links
                        auto ppp = laneLinks.equal_range(lane);
                        for(std::multimap<std::string, std::pair<std::string, int>>::iterator z = ppp.first; z != ppp.second; ++z)
                        {
                            int linkNumber = (*z).second.second;

                            // push the new TD into the circular buffer
                            auto location = linkTD.find( make_pair(TLid,linkNumber) );
                            std::vector<double> entry {TD /*traffic demand*/, simTime().dbl() /*time of measure*/, lagT /*time it takes to arrive at intersection*/};
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
                        error("lane %s does not exist in laneTotalVehCount", lane.c_str());

                    (*u).second.second.totalVehCount = (*u).second.second.totalVehCount + 1;

                    // if this is the first vehicle on this lane
                    if((*u).second.second.totalVehCount == 1)
                        (*u).second.second.firstArrivalTime = simTime().dbl();

                    // last detection time
                    (*u).second.second.lastArrivalTime = simTime().dbl();
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

            queueDataEntryDetailed *entry = new queueDataEntryDetailed(simTime().dbl(), TLid, totalQueue, maxQueue, laneCount);
            Vec_queueSize.push_back(*entry);

            // reset values
            y.second.totalQueueSize = 0;
            y.second.maxQueueSize = 0;
        }
    }
}


// update traffic demand for each lane at the beginning of each cycle
// this method is called only when measureTrafficDemandMode == 2
void LoopDetectors::updateTrafficDemand()
{
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
            u.second.second.firstArrivalTime = simTime().dbl();

            // clear buffer for this lane in laneTD
            const auto &loc = laneTD.find(lane);
            (loc->second).second.clear();

            // clear buffer for this lane in linkTD
            const auto &ppp = laneLinks.equal_range(lane);
            for(std::multimap<std::string, std::pair<std::string, int>>::iterator z = ppp.first; z != ppp.second; ++z)
            {
                int linkNumber = (*z).second.second;

                // push the new TD into the circular buffer
                const auto &location = linkTD.find( make_pair(TLid,linkNumber) );
                (location->second).clear();
            }

            if(ev.isGUI() && debugLevel > 1)
            {
                std::cout << ">>> Traffic demand measurement restarted for lane " << lane << endl << endl;
                std::cout.flush();
            }
        }

        if(TD != 0)
        {
            // push it into the circular buffer
            auto loc = laneTD.find(lane);
            std::vector<double> entry {TD /*traffic demand*/, simTime().dbl() /*time of measure*/, -1};
            (loc->second).second.push_back(entry);

            // iterate over outgoing links
            auto ppp = laneLinks.equal_range(lane);
            for(std::multimap<std::string, std::pair<std::string, int>>::iterator z = ppp.first; z != ppp.second; ++z)
            {
                int linkNumber = (*z).second.second;

                // push the new TD into the circular buffer
                auto location = linkTD.find( make_pair(TLid,linkNumber) );
                std::vector<double> entry {TD /*traffic demand*/, simTime().dbl() /*time of measure*/, -1};
                (location->second).push_back(entry);
            }
        }
    }
}


void LoopDetectors::saveTLQueueingData()
{
    boost::filesystem::path filePath;

    if(ev.isGUI())
    {
        filePath = "results/gui/TLqueuingData.txt";
    }
    else
    {
        // get the current run number
        int currentRun = ev.getConfigEx()->getActiveRunNumber();
        std::ostringstream fileName;
        fileName << std::setfill('0') << std::setw(3) << currentRun << "_TLqueuingData.txt";
        filePath = "results/cmd/" + fileName.str();
    }

    FILE *filePtr = fopen (filePath.string().c_str(), "w");

    // write simulation parameters at the beginning of the file in CMD mode
    if(!ev.isGUI())
    {
        // get the current config name
        std::string configName = ev.getConfigEx()->getVariable("configname");

        // get number of total runs in this config
        int totalRun = ev.getConfigEx()->getNumRunsInConfig(configName.c_str());

        // get the current run number
        int currentRun = ev.getConfigEx()->getActiveRunNumber();

        // get all iteration variables
        std::vector<std::string> iterVar = ev.getConfigEx()->unrollConfig(configName.c_str(), false);

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


void LoopDetectors::saveTLPhasingData()
{
    boost::filesystem::path filePath;

    if(ev.isGUI())
    {
        filePath = "results/gui/TLphasingData.txt";
    }
    else
    {
        // get the current run number
        int currentRun = ev.getConfigEx()->getActiveRunNumber();
        std::ostringstream fileName;
        fileName << std::setfill('0') << std::setw(3) << currentRun << "_TLphasingData.txt";
        filePath = "results/cmd/" + fileName.str();
    }

    FILE *filePtr = fopen (filePath.string().c_str(), "w");

    // write simulation parameters at the beginning of the file in CMD mode
    if(!ev.isGUI())
    {
        // get the current config name
        std::string configName = ev.getConfigEx()->getVariable("configname");

        // get number of total runs in this config
        int totalRun = ev.getConfigEx()->getNumRunsInConfig(configName.c_str());

        // get the current run number
        int currentRun = ev.getConfigEx()->getActiveRunNumber();

        // get all iteration variables
        std::vector<std::string> iterVar = ev.getConfigEx()->unrollConfig(configName.c_str(), false);

        // write to file
        fprintf (filePtr, "configName      %s\n", configName.c_str());
        fprintf (filePtr, "totalRun        %d\n", totalRun);
        fprintf (filePtr, "currentRun      %d\n", currentRun);
        fprintf (filePtr, "currentConfig   %s\n\n\n", iterVar[currentRun].c_str());
    }

    // write header
    fprintf (filePtr, "%-12s", "TLid");
    fprintf (filePtr, "%-12s", "phase");
    fprintf (filePtr, "%-12s", "cycle");
    fprintf (filePtr, "%-35s", "allowedMovements");
    fprintf (filePtr, "%-15s", "greenLength");
    fprintf (filePtr, "%-15s", "greenStart");
    fprintf (filePtr, "%-15s", "yellowStart");
    fprintf (filePtr, "%-15s", "redStart");
    fprintf (filePtr, "%-15s", "phaseEnd");
    fprintf (filePtr, "%-15s", "#lanes");
    fprintf (filePtr, "%-15s\n\n", "totalqueueSize");

    // write body
    for(auto & y : statusTL)
    {
        currentStatusTL status = y.second;

        fprintf (filePtr, "%-12s", y.first.first.c_str());
        fprintf (filePtr, "%-12d", y.first.second);
        fprintf (filePtr, "%-12d", status.cycle);
        fprintf (filePtr, "%-35s", status.allowedMovements.c_str());
        fprintf (filePtr, "%-15.2f", status.greenLength);
        fprintf (filePtr, "%-15.2f", status.greenStart);
        fprintf (filePtr, "%-15.2f", status.yellowStart);
        fprintf (filePtr, "%-15.2f", status.redStart);
        fprintf (filePtr, "%-15.2f", status.phaseEnd);
        fprintf (filePtr, "%-15d", status.incommingLanes);
        fprintf (filePtr, "%-15d\n", status.totalQueueSize);
    }

    fclose(filePtr);
}

}
