/****************************************************************************/
/// @file    TrafficActuated.cc
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

#include <iomanip>

#include "trafficLight/TSC/03_TrafficActuated.h"

namespace VENTOS {

Define_Module(VENTOS::TrafficLightActuated);


TrafficLightActuated::~TrafficLightActuated()
{

}


void TrafficLightActuated::initialize(int stage)
{
    super::initialize(stage);

    if(TLControlMode != TL_TrafficActuated)
        return;

    if(stage == 0)
    {
        // NED variables
        passageTime = par("passageTime").doubleValue();
        greenExtension = par("greenExtension").boolValue();

        intervalChangeEVT = new omnetpp::cMessage("intervalChangeEVT", 1);
    }
}


void TrafficLightActuated::finish()
{
    super::finish();
}


void TrafficLightActuated::handleMessage(omnetpp::cMessage *msg)
{
    if (TLControlMode == TL_TrafficActuated && msg == intervalChangeEVT)
    {
        chooseNextInterval("C");

        if(intervalDuration <= 0)
            throw omnetpp::cRuntimeError("intervalDuration is <= 0");

        // Schedule next light change event:
        scheduleAt(omnetpp::simTime().dbl() + intervalDuration, intervalChangeEVT);
    }
    else
        super::handleMessage(msg);
}


void TrafficLightActuated::initialize_withTraCI()
{
    // call parent
    super::initialize_withTraCI();

    if(TLControlMode != TL_TrafficActuated)
        return;

    LOG_INFO << "\nTraffic-actuated traffic signal control ... \n\n" << std::flush;

    // set initial values
    currentInterval = phase1_5;
    intervalDuration = minGreenTime;
    intervalElapseTime = 0;

    scheduleAt(omnetpp::simTime().dbl() + intervalDuration, intervalChangeEVT);

    TLList = TraCI->TLGetIDList();

    for (auto &TL : TLList)
    {
        TraCI->TLSetProgram(TL, "adaptive-time");
        TraCI->TLSetState(TL, currentInterval);

        firstGreen[TL] = currentInterval;

        // initialize TL status
        updateTLstate(TL, "init", currentInterval);
    }

    checkLoopDetectors();

    // passageTime calculation per incoming lane
    if(passageTime == -1)
    {
        // (*LD).first is 'lane id' and (*LD).second is detector id
        for (auto &LD : LD_actuated)
        {
            // get the max speed on this lane
            double maxV = TraCI->laneGetMaxSpeed(LD.first);
            // get position of the loop detector from end of lane
            double LDPos = TraCI->laneGetLength(LD.first) - TraCI->LDGetPosition(LD.second);
            // calculate passageTime for this lane
            double pass = std::fabs(LDPos) / maxV;
            // check if not greater than Gmin
            if(pass > minGreenTime)
            {
                LOG_WARNING << "WARNING: loop detector (" << LD.second << ") is far away! Passage time is greater than Gmin." << std::endl;
                pass = minGreenTime;
            }
            // set it for each lane
            passageTimePerLane[LD.first] = pass;
        }
    }
    // user set the passage time
    else if(passageTime >=0 && passageTime <= minGreenTime)
    {
        // (*LD).first is 'lane id' and (*LD).second is detector id
        for (auto &LD : LD_actuated)
            passageTimePerLane[LD.first] = passageTime;
    }
    else
        throw omnetpp::cRuntimeError("passageTime value is not set correctly!");

    LOG_DEBUG << boost::format("\nSimTime: %1% | Planned interval: %2% | Start time: %1% | End time: %3% \n")
    % omnetpp::simTime().dbl() % currentInterval % (omnetpp::simTime().dbl() + intervalDuration) << std::flush;
}


void TrafficLightActuated::executeEachTimeStep()
{
    // call parent
    super::executeEachTimeStep();

    if(TLControlMode != TL_TrafficActuated)
        return;

    // update passage time if necessary
    if(passageTime == -1)
    {
        // (*LD).first is 'lane id' and (*LD).second is detector id
        for (auto &LD : LD_actuated)
        {
            double approachSpeed = TraCI->LDGetLastStepMeanVehicleSpeed(LD.second);
            // update passage time for this lane
            if(approachSpeed > 0)
            {
                // get position of the loop detector from end of lane
                double LDPos = TraCI->laneGetLength(LD.first) - TraCI->LDGetPosition(LD.second);
                // calculate passageTime for this lane
                double pass = std::fabs(LDPos) / approachSpeed;
                // check if not greater than Gmin
                if(pass > minGreenTime)
                    pass = minGreenTime;

                // update passage time value in passageTimePerLane
                auto location = passageTimePerLane.find(LD.first);
                location->second = pass;
            }
        }
    }

    intervalElapseTime += updateInterval;

    // todo: change intervalElapseTime to the following code
    //    if(intervalChangeEVT->isScheduled())
    //        if(currentInterval != "yellow" && currentInterval != "red")
    //            std::cout << "### " << intervalChangeEVT->getArrivalTime() - simTime() << std::endl;
}


void TrafficLightActuated::checkLoopDetectors()
{
    // get all loop detectors
    auto str = TraCI->LDGetIDList();

    // for each loop detector
    for (auto &loopId : str)
    {
        std::string lane = TraCI->LDGetLaneID(loopId);

        if( std::string(loopId).find("actuated_") != std::string::npos )
            LD_actuated[lane] = loopId;
    }

    if(str.empty())
    {
        LOG_WARNING << "WARNING: traffic-actuated TSC needs loop detectors on incoming lanes. \n";
        return;
    }

    for (auto &TL : TLList)
    {
        // get all incoming lanes
        auto lan = TraCI->TLGetControlledLanes(TL);

        // remove duplicate entries
        sort( lan.begin(), lan.end() );
        lan.erase( unique( lan.begin(), lan.end() ), lan.end() );

        for(auto &lane : lan)
        {
            // traffic-actuated TSC needs one actuated LD on each incoming lane
            if(LD_actuated.find(lane) == LD_actuated.end())
                LOG_WARNING << boost::format("WARNING: no loop detector found on lane (%1%). No actuation is available for this lane. \n") % lane;
        }
    }
}


void TrafficLightActuated::chooseNextInterval(std::string TLid)
{
    if (currentInterval == "yellow")
    {
        currentInterval = "red";

        // change all 'y' to 'r'
        std::string str = TraCI->TLGetState(TLid);
        std::string nextInterval = "";
        for(char& c : str) {
            if (c == 'y')
                nextInterval += 'r';
            else
                nextInterval += c;
        }

        // set the new state
        TraCI->TLSetState(TLid, nextInterval);
        intervalElapseTime = 0.0;
        intervalDuration = redTime;

        // update TL status for this phase
        updateTLstate(TLid, "red");

        LOG_DEBUG << boost::format("\nSimTime: %1% | Planned interval: %2% | Start time: %1% | End time: %3% \n")
        % omnetpp::simTime().dbl() % currentInterval % (omnetpp::simTime().dbl() + intervalDuration) << std::flush;
    }
    else if (currentInterval == "red")
    {
        // update TL status for this phase
        if(nextGreenInterval == firstGreen[TLid])
            updateTLstate(TLid, "phaseEnd", nextGreenInterval, true);
        else
            updateTLstate(TLid, "phaseEnd", nextGreenInterval);

        currentInterval = nextGreenInterval;

        // set the new state
        TraCI->TLSetState(TLid, nextGreenInterval);
        intervalElapseTime = 0.0;
        intervalDuration = minGreenTime;

        LOG_DEBUG << boost::format("\nSimTime: %1% | Planned interval: %2% | Start time: %1% | End time: %3% \n")
        % omnetpp::simTime().dbl() % currentInterval % (omnetpp::simTime().dbl() + intervalDuration) << std::flush;
    }
    else
        chooseNextGreenInterval(TLid);
}


void TrafficLightActuated::chooseNextGreenInterval(std::string TLid)
{
    LOG_DEBUG << "\n------------------------------------------------------------------------------- \n";
    LOG_DEBUG << ">>> Check for actuation in the last green interval ... \n" << std::flush;

    // get loop detector information
    std::map<std::string,double> LastActuatedTime;

    // (*LD).first is 'lane id' and (*LD).second is detector id
    for (auto &LD : LD_actuated)
    {
        double elapsedT = TraCI->LDGetElapsedTimeLastDetection(LD.second);
        LastActuatedTime[LD.first] = elapsedT;
    }

    if(LOG_ACTIVE(DEBUG_LOG_VAL))
    {
        LOG_DEBUG << "\n    Passage time value per lane: ";
        for (auto &LD : passageTimePerLane)
            LOG_DEBUG << LD.first << " (" << LD.second << ") | ";

        LOG_DEBUG << "\n";

        LOG_DEBUG << "    Actuated LDs (lane, elapsed time): ";
        for (auto &LD : LD_actuated)
        {
            double elapsedT = TraCI->LDGetElapsedTimeLastDetection(LD.second);

            if(abs(omnetpp::simTime().dbl() - (elapsedT + updateInterval)) >= updateInterval)
                LOG_DEBUG << LD.first << " (" << elapsedT << ") | " << std::flush;
        }

        LOG_DEBUG << "\n" << std::flush;
    }

    bool extend = false;
    std::string nextInterval;

    // Do proper transition:
    if (currentInterval == phase1_5)
    {
        if (greenExtension && intervalElapseTime < maxGreenTime &&
                LastActuatedTime["NC_4"] < passageTimePerLane["NC_4"] &&
                LastActuatedTime["SC_4"] < passageTimePerLane["SC_4"])
        {
            intervalDuration = std::max(passageTimePerLane["NC_4"]-LastActuatedTime["NC_4"], passageTimePerLane["SC_4"]-LastActuatedTime["SC_4"]);
            extend = true;
        }
        else if (LastActuatedTime["NC_4"] < passageTimePerLane["NC_4"])
        {
            nextGreenInterval = phase2_5;
            nextInterval = "grgrGgrgrrgrgrygrgrrrrrr";
            extend = false;
        }
        else if (LastActuatedTime["SC_4"] < passageTimePerLane["SC_4"])
        {
            nextGreenInterval = phase1_6;
            nextInterval = "grgrygrgrrgrgrGgrgrrrrrr";
            extend = false;
        }
        else
        {
            nextGreenInterval = phase2_6;
            nextInterval = "grgrygrgrrgrgrygrgrrrrrr";
            extend = false;
        }
    }
    else if (currentInterval == phase2_5)
    {
        if (greenExtension && intervalElapseTime < maxGreenTime &&
                LastActuatedTime["NC_4"] < passageTimePerLane["NC_4"])
        {
            intervalDuration = passageTimePerLane["NC_4"] - LastActuatedTime["NC_4"];
            extend = true;
        }
        else
        {
            nextGreenInterval = phase2_6;
            nextInterval = "gGgGygrgrrgrgrrgrgrrrrrG";
            extend = false;
        }
    }
    else if (currentInterval == phase1_6)
    {
        if (greenExtension && intervalElapseTime < maxGreenTime &&
                LastActuatedTime["SC_4"] < passageTimePerLane["SC_4"])
        {
            intervalDuration = passageTimePerLane["SC_4"] - LastActuatedTime["SC_4"];
            extend = true;
        }
        else
        {
            nextGreenInterval = phase2_6;
            nextInterval = "grgrrgrgrrgGgGygrgrrrGrr";
            extend = false;
        }
    }
    else if (currentInterval == phase2_6)
    {
        if (greenExtension && intervalElapseTime < maxGreenTime &&
                (LastActuatedTime["NC_2"] < passageTimePerLane["NC_2"] ||
                        LastActuatedTime["NC_3"] < passageTimePerLane["NC_3"] ||
                        LastActuatedTime["SC_2"] < passageTimePerLane["SC_2"] ||
                        LastActuatedTime["SC_3"] < passageTimePerLane["SC_3"]))
        {
            double biggest1 = std::max(passageTimePerLane["NC_2"]-LastActuatedTime["NC_2"], passageTimePerLane["SC_2"]-LastActuatedTime["SC_2"]);
            double biggest2 = std::max(passageTimePerLane["NC_3"]-LastActuatedTime["NC_3"], passageTimePerLane["SC_3"]-LastActuatedTime["SC_3"]);
            intervalDuration = std::max(biggest1, biggest2);
            extend = true;
        }
        else
        {
            nextGreenInterval = phase3_7;
            nextInterval = "gygyrgrgrrgygyrgrgrrryry";
            extend = false;
        }
    }
    else if (currentInterval == phase3_7)
    {
        if (greenExtension && intervalElapseTime < maxGreenTime &&
                LastActuatedTime["WC_4"] < passageTimePerLane["WC_4"] &&
                LastActuatedTime["EC_4"] < passageTimePerLane["EC_4"])
        {
            intervalDuration = std::max(passageTimePerLane["WC_4"]-LastActuatedTime["WC_4"], passageTimePerLane["EC_4"]-LastActuatedTime["EC_4"]);
            extend = true;
        }
        else if (LastActuatedTime["WC_4"] < passageTimePerLane["WC_4"])
        {
            nextGreenInterval = phase3_8;
            nextInterval = "grgrrgrgrygrgrrgrgrGrrrr";
            extend = false;
        }
        else if (LastActuatedTime["EC_4"] < passageTimePerLane["EC_4"])
        {
            nextGreenInterval = phase4_7;
            nextInterval = "grgrrgrgrGgrgrrgrgryrrrr";
            extend = false;
        }
        else
        {
            nextGreenInterval = phase4_8;
            nextInterval = "grgrrgrgrygrgrrgrgryrrrr";
            extend = false;
        }
    }
    else if (currentInterval == phase3_8)
    {
        if (greenExtension && intervalElapseTime < maxGreenTime &&
                LastActuatedTime["WC_4"] < passageTimePerLane["WC_4"])
        {
            intervalDuration = passageTimePerLane["WC_4"] - LastActuatedTime["WC_4"];
            extend = true;
        }
        else
        {
            nextGreenInterval = phase4_8;
            nextInterval = "grgrrgrgrrgrgrrgGgGyrrGr";
            extend = false;
        }
    }
    else if (currentInterval == phase4_7)
    {
        if (greenExtension && intervalElapseTime < maxGreenTime &&
                LastActuatedTime["EC_4"] < passageTimePerLane["EC_4"])
        {
            intervalDuration = passageTimePerLane["EC_4"] - LastActuatedTime["EC_4"];
            extend = true;
        }
        else
        {
            nextGreenInterval = phase4_8;
            nextInterval = "grgrrgGgGygrgrrgrgrrGrrr";
            extend = false;
        }
    }
    else if (currentInterval == phase4_8)
    {
        if (greenExtension && intervalElapseTime < maxGreenTime &&
                (LastActuatedTime["WC_2"] < passageTimePerLane["WC_2"] ||
                        LastActuatedTime["WC_3"] < passageTimePerLane["WC_3"] ||
                        LastActuatedTime["EC_2"] < passageTimePerLane["EC_2"] ||
                        LastActuatedTime["EC_3"] < passageTimePerLane["EC_3"]))
        {
            double biggest1 = std::max(passageTimePerLane["WC_2"]-LastActuatedTime["WC_2"], passageTimePerLane["EC_2"]-LastActuatedTime["EC_2"]);
            double biggest2 = std::max(passageTimePerLane["WC_3"]-LastActuatedTime["WC_3"], passageTimePerLane["EC_3"]-LastActuatedTime["EC_3"]);
            intervalDuration = std::max(biggest1, biggest2);
            extend = true;
        }
        else
        {
            nextGreenInterval = phase1_5;
            nextInterval = "grgrrgygyrgrgrrgygyryryr";
            extend = false;
        }
    }

    // the current green interval should be extended
    if(extend)
    {
        // give a lower bound
        intervalDuration = std::max(updateInterval, intervalDuration);

        // interval duration after this offset
        double newIntervalTime = intervalElapseTime + intervalDuration;

        // never extend past maxGreenTime
        if (newIntervalTime > maxGreenTime)
            intervalDuration = intervalDuration - (newIntervalTime - maxGreenTime);

        // offset can not be too small
        if(intervalDuration < updateInterval)
        {
            intervalDuration = 0.0001;
            intervalElapseTime = maxGreenTime;

            LOG_DEBUG << "\n    Green extension offset is too small. Terminating the current phase ..." << std::flush;
        }
        else
            LOG_DEBUG << "\n    Extending green by " << intervalDuration << "s" << std::flush;
    }
    // we should terminate the current green interval
    else
    {
        currentInterval = "yellow";
        TraCI->TLSetState(TLid, nextInterval);

        intervalElapseTime = 0.0;
        intervalDuration =  yellowTime;

        // update TL status for this phase
        updateTLstate(TLid, "yellow");

        LOG_DEBUG << boost::format("\n    The next green interval is %s \n") % nextGreenInterval;
        LOG_DEBUG << "------------------------------------------------------------------------------- \n" << std::flush;

        LOG_DEBUG << boost::format("\nSimTime: %1% | Planned interval: %2% | Start time: %1% | End time: %3% \n")
        % omnetpp::simTime().dbl() % currentInterval % (omnetpp::simTime().dbl() + intervalDuration) << std::flush;
    }
}

}
