/****************************************************************************/
/// @file    ApplV_02_PlatoonMg.cc
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author  second author name
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

#include "nodes/vehicle/05_PlatoonMg.h"
#include "global/SignalObj.h"

namespace VENTOS {

Define_Module(VENTOS::ApplVPlatoonMg);

// duration in seconds that platoon leader waits for response
#define  WAIT_FOR_RES       1.
#define  WAIT_FOR_MERGE     5.
#define  TARGET_GAP_GOAL    0.95
#define  SEND_DELAY_OFFSET  0.1


ApplVPlatoonMg::~ApplVPlatoonMg()
{
    cancelAndDelete(entryManeuverEvt);
    cancelAndDelete(plnTIMER0);

    cancelAndDelete(plnTIMER1);
    cancelAndDelete(plnTIMER1a);
    cancelAndDelete(plnTIMER2);
    cancelAndDelete(plnTIMER3);
    cancelAndDelete(plnTIMER3a);

    cancelAndDelete(plnTIMER4);
    cancelAndDelete(plnTIMER5);
    cancelAndDelete(plnTIMER6);
    cancelAndDelete(plnTIMER7);
    cancelAndDelete(plnTIMER8);
    cancelAndDelete(plnTIMER8a);

    cancelAndDelete(plnTIMER9);

    cancelAndDelete(plnTIMER10);
    cancelAndDelete(plnTIMER11);

    cancelAndDelete(plnTIMER12);
}


void ApplVPlatoonMg::initialize(int stage)
{
    super::initialize(stage);

    if (stage == 0)
    {
        if(plnMode != platoonManagement)
            return;

        if(!DSRCenabled)
            throw omnetpp::cRuntimeError("This vehicle is not DSRC-enabled!");

        // I am the platoon leader
        if(myPlnDepth == 0)
        {
            maxPlnSize = par("maxPlatoonSize").intValue();
            if(maxPlnSize < 1)
                throw omnetpp::cRuntimeError("maxPlnSize is invalid in vehicle '%s'", SUMOID.c_str());

            optPlnSize = par("optPlatoonSize").intValue();
            if(optPlnSize < 1)
                throw omnetpp::cRuntimeError("optPlnSize is invalid in vehicle '%s'", SUMOID.c_str());
        }

        TP = par("TP").doubleValue();
        if(TP <= 0)
            throw omnetpp::cRuntimeError("TP (inter-platoon gap) is invalid in vehicle '%s'", SUMOID.c_str());

        TG = TraCI->vehicleGetTimeGap(SUMOID);
        if(TG <= 0)
            throw omnetpp::cRuntimeError("TG (intra-platoon gap) is invalid in vehicle '%s'", SUMOID.c_str());

        if(TP < TG)
            throw omnetpp::cRuntimeError("InterGap (=%d) is smaller than intraGap (=%d) in vehicle '%s'", TP, TG, SUMOID.c_str());

        adaptiveTG = par("adaptiveTG").boolValue();

        entryEnabled = par("entryEnabled").boolValue();
        mergeEnabled = par("mergeEnabled").boolValue();
        splitEnabled = par("splitEnabled").boolValue();
        followerLeaveEnabled = par("followerLeaveEnabled").boolValue();
        leaderLeaveEnabled = par("leaderLeaveEnabled").boolValue();

        setVehicleState((myPlnDepth == 0) ? state_platoonLeader : state_platoonFollower);

        WATCH(maxPlnSize);
        WATCH(optPlnSize);
        WATCH(vehicleState);
        WATCH(busy);

        // entry maneuver
        // --------------
        if(entryEnabled)
        {
            entryManeuverEvt = new omnetpp::cMessage("EntryEvt");
            //double offset = dblrand() * 10;
            scheduleAt(omnetpp::simTime() + 4., entryManeuverEvt); // todo: no offset for now!

            plnTIMER0 = new omnetpp::cMessage("listening to beacons");
        }

        // merge maneuver
        // --------------
        plnTIMER1  = new omnetpp::cMessage("wait for merge reply");
        plnTIMER1a = new omnetpp::cMessage("wait to catchup");
        plnTIMER2  = new omnetpp::cMessage("wait for followers ack");
        plnTIMER3  = new omnetpp::cMessage("wait for merge done");
        plnTIMER3a = new omnetpp::cMessage("wait for front platoon beacon");

        // split maneuver
        // --------------
        plnTIMER4  = new omnetpp::cMessage("wait for split reply");
        plnTIMER6  = new omnetpp::cMessage("wait for free agent ACK");
        plnTIMER7  = new omnetpp::cMessage("wait for all ACKs");
        plnTIMER5  = new omnetpp::cMessage("wait for change_pl");
        plnTIMER8  = new omnetpp::cMessage("wait for split done");
        plnTIMER8a = new omnetpp::cMessage("wait for enough gap");

        // leader leave
        // ------------
        plnTIMER9 = new omnetpp::cMessage("wait for VOTE reply");

        // follower leave
        // --------------
        plnTIMER10 = new omnetpp::cMessage("wait for leave reply");
        plnTIMER11 = new omnetpp::cMessage("wait for split completion");

        // dissolve
        // --------
        plnTIMER12 = new omnetpp::cMessage("wait for DISSOLVE ACK");

        // if platoonMonitorTIMER is not scheduled earlier
        if(!platoonMonitorTIMER || (platoonMonitorTIMER && !platoonMonitorTIMER->isScheduled()))
        {
            platoonMonitorTIMER = new omnetpp::cMessage("platoon_monitor");
            scheduleAt(omnetpp::simTime(), platoonMonitorTIMER);
        }
    }
}


void ApplVPlatoonMg::finish()
{
    super::finish();
}


// is called, every time the position of vehicle changes
void ApplVPlatoonMg::handlePositionUpdate(cObject* obj)
{
    // pass it down!
    super::handlePositionUpdate(obj);
}


void ApplVPlatoonMg::handleSelfMsg(omnetpp::cMessage* msg)
{
    if(msg == platoonMonitorTIMER)
    {
        // save current platoon configuration
        reportConfigToStat();

        // check if we can split
        pltSplitMonitor();

        scheduleAt(omnetpp::simTime() + updateInterval, platoonMonitorTIMER);
    }
    else if(plnMode == platoonManagement && (msg == entryManeuverEvt || msg == plnTIMER0))
        entry_handleSelfMsg(msg);
    else if(plnMode == platoonManagement && (msg == plnTIMER1 || msg == plnTIMER1a || msg == plnTIMER2 || msg == plnTIMER3))
        merge_handleSelfMsg(msg);
    else if(plnMode == platoonManagement && (msg == plnTIMER4 || msg == plnTIMER6 || msg == plnTIMER7 || msg == plnTIMER5 || msg == plnTIMER8 || msg == plnTIMER8a))
        split_handleSelfMsg(msg);
    else if(plnMode == platoonManagement && msg == plnTIMER9)
        leaderLeave_handleSelfMsg(msg);
    else if(plnMode == platoonManagement && (msg == plnTIMER10 || msg == plnTIMER11))
        followerLeave_handleSelfMsg(msg);
    else if(plnMode == platoonManagement && msg == plnTIMER12)
        dissolve_handleSelfMsg(msg);
    else
        super::handleSelfMsg(msg);
}


void ApplVPlatoonMg::onBeaconVehicle(BeaconVehicle* wsm)
{
    // pass it down!
    super::onBeaconVehicle(wsm);

    if(plnMode == platoonManagement)
    {
        merge_BeaconFSM(wsm);
        split_BeaconFSM(wsm);
        common_BeaconFSM(wsm);
        entry_BeaconFSM(wsm);
        leaderLeave_BeaconFSM(wsm);
        followerLeave_BeaconFSM(wsm);
        dissolve_BeaconFSM(wsm);
    }
}


void ApplVPlatoonMg::onBeaconRSU(BeaconRSU* wsm)
{
    // pass it down!
    super::onBeaconRSU(wsm);
}


void ApplVPlatoonMg::onPlatoonMsg(PlatoonMsg* wsm)
{
    if(plnMode == platoonManagement)
    {
        merge_DataFSM(wsm);
        split_DataFSM(wsm);
        common_DataFSM(wsm);
        entry_DataFSM(wsm);
        leaderLeave_DataFSM(wsm);
        followerLeave_DataFSM(wsm);
        dissolve_DataFSM(wsm);
    }
}


void ApplVPlatoonMg::sendPltData(std::string receiverID, uCommand_t msgType, std::string receivingPlatoonID, value_t value)
{
    if(plnMode != platoonManagement)
        throw omnetpp::cRuntimeError("This application mode does not support platoon management!");

    PlatoonMsg* wsm = new PlatoonMsg("platoonMsg", TYPE_PLATOON_DATA);

    // add header length
    wsm->addBitLength(headerLength);

    // add payload length
    wsm->addBitLength(dataLengthBits);

    wsm->setWsmVersion(1);
    wsm->setSecurityType(1);
    wsm->setChannelNumber(Veins::Channels::CCH);
    wsm->setDataRate(1);
    wsm->setPriority(dataPriority);
    wsm->setPsid(0);

    wsm->setSenderID(SUMOID.c_str());
    wsm->setReceiverID(receiverID.c_str());
    wsm->setUCommandType(msgType);
    wsm->setSendingPlatoonID(myPlnID.c_str());
    wsm->setReceivingPlatoonID(receivingPlatoonID.c_str());
    wsm->setValue(value);

    if(record_platoon_stat)
    {
        plnDataExchange_t entry;

        entry.time = omnetpp::simTime().dbl();
        entry.senderId = wsm->getSenderID();
        entry.sendingPltId = wsm->getSendingPlatoonID();
        entry.senderState = stateToStr(this->vehicleState);
        entry.command = uCommandToStr((uCommand_t)wsm->getUCommandType());
        entry.receiverId = wsm->getReceiverID();
        entry.receivingPltId = wsm->getReceivingPlatoonID();

        STAT->global_plnDataExchange_stat.push_back(entry);
    }

    sendDelayed(wsm, uniform(0,SEND_DELAY_OFFSET), lowerLayerOut);
}


// change the blue color of the follower to show depth
// only platoon leader can call this!
void ApplVPlatoonMg::updateColorDepth()
{
    if(plnSize <= 0)
        throw omnetpp::cRuntimeError("invalid plnSize in vehicle '%s'", SUMOID.c_str());

    if(plnSize == 1)
        return;

    RGB colorRGB = Color::colorNameToRGB("blue");
    HSV colorHSV = Color::rgb2hsv(colorRGB.red, colorRGB.green, colorRGB.blue);

    // get different saturation
    std::vector<double> shades = Color::generateColorShades(plnSize - 1);

    // leader has all the followers in plnMembersList list
    for(unsigned int depth = 1; depth < plnMembersList.size(); ++depth)
    {
        HSV newColorHSV = {colorHSV.hue, shades[depth-1], colorHSV.value};
        RGB newColorRGB = Color::hsv2rgb(newColorHSV.hue, newColorHSV.saturation, newColorHSV.value);
        TraCI->vehicleSetColor(plnMembersList[depth], newColorRGB);
    }
}


void ApplVPlatoonMg::setVehicleState(states_num_t vehState, std::string maneuver)
{
    if(record_platoon_stat)
    {
        plnStateChange_t entry;

        entry.time = omnetpp::simTime().dbl();
        entry.vehId = SUMOID;
        entry.fromState = stateToStr(this->vehicleState);
        entry.toState = stateToStr(vehState);

        STAT->global_plnStateChange_stat.push_back(entry);

        if(maneuver != "")
        {
            plnManeuverDuration_t entry;

            entry.time = omnetpp::simTime().dbl();
            entry.vehId = SUMOID;
            entry.vehState = stateToStr(vehState);
            entry.maneuver = maneuver;

            STAT->global_plnManeuverDuration_stat.push_back(entry);
        }
    }

    this->vehicleState = vehState;
}


void ApplVPlatoonMg::reportConfigToStat()
{
    // record platoon configuration from the platoon leader
    // make sure the platoon is stable (busy is not set)
    if(record_platoon_stat && myPlnDepth == 0 && !this->busy)
    {
        auto members = TraCI->platoonGetMembers(SUMOID);

        for(auto &vehId : members)
        {
            std::string omnetId = TraCI->convertId_traci2omnet(vehId);

            // get a pointer to the vehicle
            cModule *mod = omnetpp::getSimulation()->getSystemModule()->getModuleByPath(omnetId.c_str());
            ASSERT(mod);

            // get the application module
            cModule *appl = mod->getSubmodule("appl");
            ASSERT(appl);

            // get a pointer to the application layer
            ApplVPlatoonMg *vehPtr = static_cast<ApplVPlatoonMg *>(appl);
            ASSERT(vehPtr);

            plnConfig_t entry;

            entry.timestamp = omnetpp::simTime().dbl();
            entry.vehId = vehId;
            entry.pltMode = vehPtr->getPlatoonMode();
            entry.pltId = vehPtr->getPlatoonId();
            entry.pltSize = vehPtr->getPlatoonSize();
            entry.pltDepth = vehPtr->getPlatoonDepth();
            entry.optSize = vehPtr->getOptSize();
            entry.maxSize = vehPtr->getMaxSize();

            STAT->global_plnConfig_stat.push_back(entry);
        }
    }
}


const std::string ApplVPlatoonMg::stateToStr(states_num_t state)
{
    auto ii = vehicleStateMap.find(state);
    if(ii == vehicleStateMap.end())
        throw omnetpp::cRuntimeError("Cannot find state '%d' in vehicleStateMap", state);

    return ii->second;
}


const std::string ApplVPlatoonMg::uCommandToStr(uCommand_t command)
{
    auto ii = uCommandMap.find(command);
    if(ii == uCommandMap.end())
        throw omnetpp::cRuntimeError("Cannot find command '%d' in uCommandMap", command);

    return ii->second;
}


// -------------------------------------------------------------------------------------------
// --------------------------------[ Public interface ]---------------------------------------
//--------------------------------------------------------------------------------------------

// asking the platoon leader to merge into the front platoon
void ApplVPlatoonMg::manualMerge()
{
    Enter_Method("inside manualMerge() method");

    if(!DSRCenabled)
        throw omnetpp::cRuntimeError("This vehicle is not VANET-enabled!");

    if(this->myPlnDepth != 0)
        throw omnetpp::cRuntimeError("only platoon leader can initiate merge!");

    if(busy || vehicleState != state_platoonLeader)
    {
        LOG_WARNING << boost::format("\nWARNING: Platoon '%s' cannot start the merge maneuver, because it is busy performing another maneuver. \n")
        % this->myPlnID << std::flush;
        return;
    }

    if(!mergeEnabled)
    {
        LOG_WARNING << boost::format("\nWARNING: Merge is disabled in platoon '%s'. \n")
        % this->myPlnID << std::flush;
        return;
    }

    setVehicleState(state_waitForBeacon);

    scheduleAt(omnetpp::simTime() + WAIT_FOR_MERGE, plnTIMER3a);
}


// asking the platoon leader to initiate a split at position 'depth'.
void ApplVPlatoonMg::splitFromPlatoon(int depth)
{
    Enter_Method("inside splitFromPlatoon() method");

    if(!DSRCenabled)
        throw omnetpp::cRuntimeError("This vehicle is not VANET-enabled!");

    if(this->myPlnDepth != 0)
        throw omnetpp::cRuntimeError("only platoon leader can initiate split!");

    if(depth <= 0 || depth >= plnSize)
        throw omnetpp::cRuntimeError("depth of splitting vehicle is invalid!");

    if(busy || vehicleState != state_platoonLeader)
    {
        LOG_WARNING << boost::format("\nWARNING: Platoon '%s' cannot start the split maneuver, because it is busy performing another maneuver. \n")
        % this->myPlnID << std::flush;
        return;
    }

    if(!splitEnabled)
    {
        LOG_WARNING << boost::format("\nWARNING: Split is disabled in platoon '%s'. \n")
        % this->myPlnID << std::flush;
        return;
    }

    splittingDepth = depth;
    splittingVehicle = plnMembersList[splittingDepth];
    splitCaller = -1;

    busy = true;
    manualSplit = true;

    setVehicleState(state_sendSplitReq);

    split_DataFSM();
}


void ApplVPlatoonMg::leavePlatoon(std::string vehLeaveDirection)
{
    Enter_Method("inside leavePlatoon() method");

    if(!DSRCenabled)
        throw omnetpp::cRuntimeError("This vehicle is not VANET-enabled!");

    if(busy)
    {
        LOG_WARNING << boost::format("\nWARNING: Platoon '%s' cannot start the leave maneuver, because it is busy performing another maneuver. \n")
        % this->myPlnID << std::flush;
        return;
    }

    // if I am leader
    if(this->myPlnDepth == 0)
    {
        if(vehicleState != state_platoonLeader)
        {
            LOG_WARNING << boost::format("\nWARNING: Platoon leader in platoon '%s' is not in the 'state_platoonLeader' \n")
            % this->myPlnID << std::flush;
            return;
        }

        if(plnSize <= 1)
        {
            LOG_WARNING << boost::format("\nWARNING: Platoon '%s' cannot start the leave maneuver, because its size is '%d' \n")
            % this->myPlnID % plnSize << std::flush;
            return;
        }

        if(!leaderLeaveEnabled)
        {
            LOG_WARNING << boost::format("\nWARNING: Leader leave is disabled in platoon '%s'. \n")
            % this->myPlnID << std::flush;
            return;
        }

        busy = true;

        setVehicleState(state_sendVoteLeader);

        leaveDirection = vehLeaveDirection;

        leaderLeave_DataFSM();
    }
    // if I am follower
    else
    {
        if(vehicleState != state_platoonFollower)
        {
            LOG_WARNING << boost::format("\nWARNING: Platoon follower in platoon '%s' is not in the 'state_platoonFollower' \n")
            % this->myPlnID << std::flush;
            return;
        }

        if(!followerLeaveEnabled)
        {
            LOG_WARNING << boost::format("\nWARNING: Follower leave is disabled in platoon '%s'. \n")
            % this->myPlnID << std::flush;
            return;
        }

        busy = true;

        setVehicleState(state_sendLeaveReq);

        leaveDirection = vehLeaveDirection;

        followerLeave_DataFSM();
    }
}


void ApplVPlatoonMg::dissolvePlatoon()
{
    Enter_Method("inside dissolvePlatoon() method");

    if(!DSRCenabled)
        throw omnetpp::cRuntimeError("This vehicle is not VANET-enabled!");

    if(vehicleState != state_platoonLeader)
        throw omnetpp::cRuntimeError("only platoon leader can break-up the platoon!");

    if(plnSize <= 1 || busy)
        return;

    busy = true;

    setVehicleState(state_sendDissolve);

    dissolve_DataFSM();
}


// -------------------------------------------------------------------------------------------
// --------------------------------------[ Common ]-------------------------------------------
//--------------------------------------------------------------------------------------------

void ApplVPlatoonMg::common_handleSelfMsg(omnetpp::cMessage* msg)
{

}


void ApplVPlatoonMg::common_BeaconFSM(BeaconVehicle* wsm)
{

}


void ApplVPlatoonMg::common_DataFSM(PlatoonMsg* wsm)
{
    if(vehicleState == state_platoonFollower)
    {
        if ( wsm->getUCommandType() == CHANGE_PL && wsm->getSenderID() == myPlnID )
        {
            if( std::string(wsm->getReceiverID()) == "multicast" || wsm->getReceiverID() == SUMOID )
            {
                // save my current platoon ID
                oldPlnID = myPlnID;

                // these should be updated after sending ACK!
                myPlnID = wsm->getValue().newPltLeader;
                myPlnDepth += wsm->getValue().newPltDepth;

                if(myPlnID == "")
                    throw omnetpp::cRuntimeError("vehicle '%s' has an empty platoon id", SUMOID.c_str());

                if(myPlnDepth < 0)
                    throw omnetpp::cRuntimeError("vehicle '%s' has invalid platoon depth of '%d'", SUMOID.c_str(), myPlnDepth);

                // send ACK
                sendPltData(wsm->getSenderID(), ACK, wsm->getSendingPlatoonID());
            }
        }
        // if my old platoon leader asks me to change my leader to the current one
        // I have done it before, so I only ACK
        else if( wsm->getUCommandType() == CHANGE_PL && wsm->getSenderID() == oldPlnID && wsm->getValue().newPltLeader == myPlnID )
        {
            if( std::string(wsm->getReceiverID()) == "multicast" || wsm->getReceiverID() == SUMOID )
            {
                // send ACK
                sendPltData(wsm->getSenderID(), ACK, wsm->getSendingPlatoonID());
            }
        }
        else if(wsm->getUCommandType() == CHANGE_Tg && wsm->getSenderID() == myPlnID)
        {
            TraCI->vehicleSetTimeGap(SUMOID, wsm->getValue().newTG);
        }
    }
}


// -------------------------------------------------------------------------------------------
// ---------------------------------[ Entry maneuver ]----------------------------------------
//--------------------------------------------------------------------------------------------

void ApplVPlatoonMg::entry_handleSelfMsg(omnetpp::cMessage* msg)
{
    if(!entryEnabled)
        return;

    // start the Entry maneuver
    if(msg == entryManeuverEvt && vehicleState == state_idle)
    {
        // check if we are at lane 0
        if(TraCI->vehicleGetLaneIndex(SUMOID) == 0)
        {
            int32_t bitset = TraCI->vehicleBuildLaneChangeMode(10, 01, 01, 01, 01);
            TraCI->vehicleSetLaneChangeMode(SUMOID, bitset);   // alter 'lane change' mode
            TraCI->vehicleChangeLane(SUMOID, 1, 5);   // change to lane 1 (special lane)

            setVehicleState(state_waitForLaneChange);

            scheduleAt(omnetpp::simTime() + updateInterval, plnTIMER0);
        }
    }
    else if(msg == plnTIMER0 && vehicleState == state_waitForLaneChange)
    {
        // check if we change lane
        if(TraCI->vehicleGetLaneIndex(SUMOID) == 1)
        {
            // make it a free agent
            myPlnID = SUMOID;
            myPlnDepth = 0;
            plnSize = 1;
            plnMembersList.push_back(SUMOID);
            TraCI->vehicleSetTimeGap(SUMOID, TP);

            busy = false;

            // get my leading vehicle
            auto leader = TraCI->vehicleGetLeader(SUMOID, sonarDist);

            // if no leading, set speed to 5 m/s
            if(leader.leaderID == "")
                TraCI->vehicleSetSpeed(SUMOID, 5.);
            // if I have leading, set speed to max
            else
                TraCI->vehicleSetSpeed(SUMOID, 30.);

            // change color to red!
            RGB newColor = Color::colorNameToRGB("red");
            TraCI->vehicleSetColor(SUMOID, newColor);

            setVehicleState(state_platoonLeader);
        }
        else
            scheduleAt(omnetpp::simTime() + updateInterval, plnTIMER0);
    }
}


void ApplVPlatoonMg::entry_BeaconFSM(BeaconVehicle* wsm)
{

}


void ApplVPlatoonMg::entry_DataFSM(PlatoonMsg *wsm)
{

}


// -------------------------------------------------------------------------------------------
// ---------------------------------[ Merge maneuver ]----------------------------------------
//--------------------------------------------------------------------------------------------

void ApplVPlatoonMg::merge_handleSelfMsg(omnetpp::cMessage* msg)
{
    if(!mergeEnabled)
        return;

    if(msg == plnTIMER1)
    {
        if(vehicleState == state_waitForMergeReply)
        {
            // leader does not response after three re-attempts!
            if(mergeReqAttempts >= 3)
            {
                mergeReqAttempts = 0;

                setVehicleState(state_platoonLeader);
            }
            else
            {
                setVehicleState(state_sendMergeReq);

                merge_BeaconFSM();
            }
        }
    }
    else if(msg == plnTIMER1a)
    {
        if(vehicleState == state_waitForCatchup)
        {
            // check gap to the last follower
            if( CatchUpDone() )
            {
                // MyCircularBufferMerge.clear();

                // free agent
                if(plnSize == 1)
                {
                    setVehicleState(state_sendMergeDone);

                    merge_DataFSM();
                }
                else
                {
                    setVehicleState(state_notifyFollowers);

                    merge_DataFSM();
                }
            }
            else
                scheduleAt(omnetpp::simTime() + updateInterval, plnTIMER1a);
        }
    }
    else if(msg == plnTIMER2)
    {
        if(vehicleState == state_waitForAllAcks)
        {
            setVehicleState(state_notifyFollowers);

            merge_DataFSM();
        }
    }
    else if(msg == plnTIMER3)
    {
        if(vehicleState == state_waitForMergeDone)
        {
            setVehicleState(state_sendMergeAccept);

            merge_DataFSM();
        }
    }
    else if(msg == plnTIMER3a)
    {
        // we have waited long enough for a beacon from
        // the front vehicle
        if(vehicleState == state_waitForBeacon)
        {
            setVehicleState(state_platoonLeader);
        }
    }
}


void ApplVPlatoonMg::merge_BeaconFSM(BeaconVehicle* wsm)
{
    if(!mergeEnabled)
        return;

    if(vehicleState == state_platoonLeader)
    {
        // can we merge?
        if(!busy && plnSize < optPlnSize)
        {
            if(isBeaconFromFrontVehicle(wsm))
            {
                int finalPlnSize = wsm->getPlatoonDepth() + 1 + plnSize;

                if(finalPlnSize <= optPlnSize)
                {
                    setVehicleState(state_sendMergeReq);

                    merge_BeaconFSM(wsm);
                }
            }
        }
    }
    else if(vehicleState == state_waitForBeacon)
    {
        if(isBeaconFromFrontVehicle(wsm))
        {
            setVehicleState(state_sendMergeReq);

            merge_BeaconFSM(wsm);
        }
    }
    else if(vehicleState == state_sendMergeReq)
    {
        // save these values from the received beacon from my preceding vehicle
        if(wsm != NULL)
        {
            leadingPlnID = wsm->getPlatoonID();
            leadingPlnDepth = wsm->getPlatoonDepth();
        }

        // if its not a valid leader id or depth!
        if(leadingPlnID == "" || leadingPlnDepth == -1)
        {
            setVehicleState(state_platoonLeader);

            return;
        }

        if(!plnTIMER3a->isScheduled())
        {
            // send a unicast MERGE_REQ to its platoon leader
            value_t value;
            value.myPltMembers = plnMembersList;
            sendPltData(leadingPlnID, MERGE_REQ, leadingPlnID, value);
        }
        else
        {
            cancelEvent(plnTIMER3a);

            // send a unicast MERGE_REQ to its platoon leader and
            // we set 'manualMerge' flag to true
            value_t value;
            value.myPltMembers = plnMembersList;
            value.manualMerge = true;
            sendPltData(leadingPlnID, MERGE_REQ, leadingPlnID, value);
        }

        setVehicleState(state_waitForMergeReply, "Merge_Request");

        // start plnTIMER1
        scheduleAt(omnetpp::simTime() + WAIT_FOR_RES, plnTIMER1);

        mergeReqAttempts++;
    }
}


void ApplVPlatoonMg::merge_DataFSM(PlatoonMsg* wsm)
{
    if(!mergeEnabled)
        return;

    if(vehicleState == state_waitForMergeReply)
    {
        if (wsm->getUCommandType() == MERGE_REJECT && wsm->getSenderID() == leadingPlnID)
        {
            mergeReqAttempts = 0;

            cancelEvent(plnTIMER1);

            setVehicleState(state_platoonLeader, "Merge_Reject");
        }
        else if(wsm->getUCommandType() == MERGE_ACCEPT && wsm->getSenderID() == leadingPlnID)
        {
            mergeReqAttempts = 0;

            setVehicleState(state_mergeAccepted, "Merge_Start");

            merge_DataFSM();
        }
    }
    else if(vehicleState == state_mergeAccepted)
    {
        cancelEvent(plnTIMER1);
        busy = true;

        TraCI->vehicleSetTimeGap(SUMOID, TG);
        TraCI->vehicleSetSpeed(SUMOID, 30.);  // catch-up

        // now we should wait until we catch-up completely
        setVehicleState(state_waitForCatchup);

        // MyCircularBufferMerge.clear();

        scheduleAt(omnetpp::simTime() + updateInterval, plnTIMER1a);
    }
    else if(vehicleState == state_waitForCatchup)
    {
        // if we are in waitForCatchup and receive a
        // MERGE_REQ then we should reject it!
        if (wsm->getUCommandType() == MERGE_REQ && wsm->getReceiverID() == myPlnID)
        {
            // send MERGE_REJECT
            sendPltData(wsm->getSenderID(), MERGE_REJECT, wsm->getSendingPlatoonID());
        }
    }
    else if(vehicleState == state_sendMergeDone)
    {
        myPlnID = leadingPlnID;
        myPlnDepth = leadingPlnDepth + 1;
        plnSize = -1;
        optPlnSize = -1;
        maxPlnSize = -1;
        plnMembersList.clear();
        busy = false;

        // send unicast MERGE_DONE
        sendPltData(myPlnID, MERGE_DONE, myPlnID);

        setVehicleState(state_platoonFollower, "Merge_End");
    }
    else if(vehicleState == state_notifyFollowers)
    {
        // send CHANGE_PL to all my followers (last two parameters are data attached to this ucommand)
        value_t entry;
        entry.newPltDepth = leadingPlnDepth+1;
        entry.newPltLeader = leadingPlnID;
        sendPltData("multicast", CHANGE_PL, leadingPlnID, entry);

        setVehicleState(state_waitForAllAcks);

        // start plnTIMER2
        scheduleAt(omnetpp::simTime() + WAIT_FOR_RES, plnTIMER2);
    }
    else if(vehicleState == state_waitForAllAcks)
    {
        if ( wsm->getUCommandType() == ACK && (wsm->getSendingPlatoonID() == myPlnID || wsm->getSendingPlatoonID() == leadingPlnID) )
        {
            std::string followerID = wsm->getSenderID();
            RemoveFollowerFromList_Merge(followerID);

            // all followers ACK-ed
            if(plnSize == 1)
            {
                cancelEvent(plnTIMER2);

                setVehicleState(state_sendMergeDone);

                merge_DataFSM();
            }
        }
    }
    else if(vehicleState == state_platoonLeader)
    {
        if (wsm->getUCommandType() == MERGE_REQ && wsm->getReceiverID() == myPlnID)
        {
            if(busy)
            {
                // send MERGE_REJECT
                sendPltData(wsm->getSenderID(), MERGE_REJECT, wsm->getSendingPlatoonID());

                return;
            }

            if(!wsm->getValue().manualMerge)
            {
                int finalPlnSize =  wsm->getValue().myPltMembers.size() + plnSize;
                if(finalPlnSize > optPlnSize)
                {
                    // send MERGE_REJECT
                    sendPltData(wsm->getSenderID(), MERGE_REJECT, wsm->getSendingPlatoonID());

                    return;
                }
            }
            // in manual merge, we need to update the optimal platoon size in leader
            else
            {
                int finalPlnSize = plnSize + wsm->getValue().myPltMembers.size();
                this->optPlnSize = finalPlnSize;
            }

            // save followers list for future use
            secondPlnMembersList = wsm->getValue().myPltMembers;

            setVehicleState(state_sendMergeAccept);

            merge_DataFSM(wsm);
        }
    }
    else if(vehicleState == state_sendMergeAccept)
    {
        // send MERGE_ACCEPT
        sendPltData(secondPlnMembersList.front().c_str(), MERGE_ACCEPT, secondPlnMembersList.front().c_str());

        setVehicleState(state_waitForMergeDone);

        // start plnTIMER3 (we wait)
        scheduleAt(omnetpp::simTime() + WAIT_FOR_MERGE, plnTIMER3);

        // we start the merge maneuver
        busy = true;
    }
    else if(vehicleState == state_waitForMergeDone)
    {
        if(wsm->getUCommandType() == MERGE_DONE && wsm->getReceiverID() == myPlnID)
        {
            setVehicleState(state_mergeDone);

            merge_DataFSM();
        }
    }
    else if(vehicleState == state_mergeDone)
    {
        cancelEvent(plnTIMER3);
        plnSize = plnSize + secondPlnMembersList.size();
        plnMembersList.insert(plnMembersList.end(), secondPlnMembersList.begin(), secondPlnMembersList.end());

        if( adaptiveTG && plnSize > (maxPlnSize / 2) )
        {
            // increase Tg
            double newTG = TG + 0.05;
            value_t entry;
            entry.newTG = newTG;
            sendPltData("multicast", CHANGE_Tg, myPlnID, entry);
        }

        setVehicleState(state_platoonLeader);

        // now that plnSize is changed, we should
        // change the color of the followers
        updateColorDepth();

        TraCI->vehiclePlatoonInit(SUMOID, plnMembersList);

        busy = false;
    }
}


void ApplVPlatoonMg::RemoveFollowerFromList_Merge(std::string followerID)
{
    bool found = false;
    unsigned int i = 0;

    for(i = 0; i < plnMembersList.size(); i++)
    {
        if(plnMembersList[i] == followerID)
        {
            found = true;
            break;
        }
    }

    if(found)
    {
        plnMembersList.erase(plnMembersList.begin() + i);
        plnSize--;
    }
}


bool ApplVPlatoonMg::CatchUpDone()
{
    // we use our sonar to check the gap
    auto leader = TraCI->vehicleGetLeader(SUMOID, sonarDist);

    if(leader.leaderID == "")
        return true;

    // get the timeGap setting
    double timeGapSetting = TraCI->vehicleGetTimeGap(SUMOID);

    // get speed
    double speed = TraCI->vehicleGetSpeed(SUMOID);

    // get minGap
    double minGap = TraCI->vehicleGetMinGap(SUMOID);

    double targetGap = (speed * timeGapSetting) + minGap;

    if( (TARGET_GAP_GOAL * leader.distance2Leader) <= targetGap )
        return true;
    else
        return false;
}


// -------------------------------------------------------------------------------------------
// ---------------------------------[ Split maneuver ]----------------------------------------
//--------------------------------------------------------------------------------------------

void ApplVPlatoonMg::pltSplitMonitor()
{
    // platoon leader checks constantly if we can split
    if(vehicleState == state_platoonLeader)
    {
        if(plnSize < 1)
            throw omnetpp::cRuntimeError("vehicle '%s' has invalid plnSize of '%d'", SUMOID.c_str(), plnSize);

        if(optPlnSize < 1)
            throw omnetpp::cRuntimeError("vehicle '%s' has invalid optPlnSize of '%d'", SUMOID.c_str(), optPlnSize);

        if(!busy && splitEnabled && plnSize > optPlnSize)
        {
            splittingDepth = optPlnSize;
            splittingVehicle = plnMembersList[splittingDepth];
            splitCaller = -1;

            busy = true;

            setVehicleState(state_sendSplitReq);

            split_DataFSM();
        }
    }
}


void ApplVPlatoonMg::split_handleSelfMsg(omnetpp::cMessage* msg)
{
    if(!splitEnabled)
        return;

    if(msg == plnTIMER4)
    {
        setVehicleState(state_sendSplitReq);

        split_DataFSM();
    }
    else if(msg == plnTIMER6)
    {
        if(vehicleState == state_waitForAck)
        {
            setVehicleState(state_makeItFreeAgent);

            split_DataFSM();
        }
    }
    else if(msg == plnTIMER7)
    {
        if(vehicleState == state_waitForAllAcks2)
        {
            TotalACKsRx = 0;
            TotalPLSent = 0;

            setVehicleState(state_changePL);

            split_DataFSM();
        }
    }
    else if(msg == plnTIMER5)
    {
        if(vehicleState == state_waitForCHANGEPL)
        {
            setVehicleState(state_platoonFollower);
        }
    }
    else if(msg == plnTIMER8)
    {
        if(vehicleState == state_waitForSplitDone)
        {
            setVehicleState(state_sendingACK);

            split_DataFSM();
        }
    }
    else if(msg == plnTIMER8a)
    {
        if( GapCreated() )
        {
            // send a unicast GAP_CREATED to the old leader
            // we need this in follower/leader leave only. Other maneuvers ignore this
            sendPltData(oldPlnID, GAP_CREATED, oldPlnID);

            setVehicleState(state_platoonLeader, "Split_End");
        }
        else
            scheduleAt(omnetpp::simTime() + updateInterval, plnTIMER8a);
    }
}


void ApplVPlatoonMg::split_BeaconFSM(BeaconVehicle *wsm)
{
    if(!splitEnabled)
        return;
}


void ApplVPlatoonMg::split_DataFSM(PlatoonMsg *wsm)
{
    if(!splitEnabled)
        return;

    if(vehicleState == state_sendSplitReq)
    {
        if(plnSize < 1)
            throw omnetpp::cRuntimeError("vehicle '%s' has invalid plnSize of '%d'", SUMOID.c_str(), plnSize);

        if(splittingDepth <= 0 || splittingDepth >= plnSize)
            throw omnetpp::cRuntimeError("vehicle '%s' has invalid splittingDepth of '%d'", SUMOID.c_str(), splittingDepth);

        if(splittingVehicle == "")
            throw omnetpp::cRuntimeError("vehicle '%s' has an empty splittingVehicle", SUMOID.c_str());

        // send a unicast SPLIT_REQ to the follower
        sendPltData(splittingVehicle, SPLIT_REQ, myPlnID);

        setVehicleState(state_waitForSplitReply);

        scheduleAt(omnetpp::simTime() + WAIT_FOR_RES, plnTIMER4);
    }
    else if(vehicleState == state_waitForSplitReply)
    {
        if(wsm->getUCommandType() == SPLIT_ACCEPT && wsm->getSenderID() == splittingVehicle)
        {
            cancelEvent(plnTIMER4);

            setVehicleState(state_makeItFreeAgent, "Split_Start");

            split_DataFSM();
        }
    }
    else if(vehicleState == state_makeItFreeAgent)
    {
        // send CHANGE_PL to the splitting vehicle (last two parameters are data attached to this ucommand)
        value_t entry;
        entry.newPltDepth = -splittingDepth;
        entry.newPltLeader = splittingVehicle;
        sendPltData(splittingVehicle, CHANGE_PL, myPlnID, entry);

        setVehicleState(state_waitForAck);

        scheduleAt(omnetpp::simTime() + WAIT_FOR_RES, plnTIMER6);
    }
    else if(vehicleState == state_waitForAck)
    {
        if (wsm->getUCommandType() == ACK && wsm->getSenderID() == splittingVehicle)
        {
            cancelEvent(plnTIMER6);

            // store the elements of second platoon before erasing
            secondPlnMembersList.clear();
            for (std::deque<std::string>::iterator it=plnMembersList.begin()+splittingDepth; it!=plnMembersList.end(); ++it)
                secondPlnMembersList.push_back(*it);

            // if splitting vehicle is the last follower
            // i.e: no follower exists after the splitting vehicle
            if(splittingDepth + 1 == plnSize)
            {
                setVehicleState(state_splitDone);
            }
            else
            {
                setVehicleState(state_changePL);
            }

            split_DataFSM();
        }
    }
    else if(vehicleState == state_splitDone)
    {
        plnSize = splittingDepth;
        plnMembersList.pop_back();

        // optimal platoon size is equal to current platoon size
        if(manualSplit)
            optPlnSize = plnSize;

        updateColorDepth();

        TraCI->vehiclePlatoonInit(SUMOID, plnMembersList);

        if( adaptiveTG && plnSize < (maxPlnSize / 2) )
        {
            // decrease Tg
            value_t entry;
            entry.newTG = TG;
            sendPltData("multicast", CHANGE_Tg, myPlnID, entry);
        }

        // send unicast SPLIT_DONE
        value_t entry;
        entry.caller = splitCaller;
        entry.myNewPltMembers = secondPlnMembersList;
        entry.maxSize = maxPlnSize;
        entry.optPlnSize = optPlnSize;
        entry.manualSplit = manualSplit;
        sendPltData(splittingVehicle, SPLIT_DONE, myPlnID, entry);

        // reset manualSplit after sending it in SPLIT_DONE
        manualSplit = false;

        if(splitCaller == -1)
        {
            busy = false;

            setVehicleState(state_platoonLeader);
        }
        // leader leave had called split
        // we should not set busy to false; leader leave is on-going
        else if(splitCaller == 0)
        {
            setVehicleState(state_splitCompleted);
        }
        // follower leave had called split
        // we should not set busy to false; follower leave is on-going
        else if(splitCaller == 1)
        {
            setVehicleState(state_platoonLeader);;
        }
        else
            throw omnetpp::cRuntimeError("invalid splitCaller '%d' in vehicle '%s'", splitCaller, SUMOID.c_str());
    }
    else if(vehicleState == state_changePL)
    {
        // send CHANGE_PL to followers after the splitting vehicle
        for (std::deque<std::string>::iterator it=plnMembersList.begin()+splittingDepth+1; it!=plnMembersList.end(); ++it)
        {
            value_t entry;
            entry.newPltDepth = -splittingDepth;
            entry.newPltLeader = splittingVehicle;
            sendPltData(*it /*targetVeh*/, CHANGE_PL, myPlnID, entry);

            TotalPLSent++;
        }

        setVehicleState(state_waitForAllAcks2);

        scheduleAt(omnetpp::simTime() + WAIT_FOR_RES, plnTIMER7);
    }
    else if(vehicleState == state_waitForAllAcks2)
    {
        if (wsm->getUCommandType() == ACK && wsm->getReceivingPlatoonID() == myPlnID)
        {
            std::string followerID = wsm->getSenderID();
            RemoveFollowerFromList_Split(followerID);

            TotalACKsRx++;

            // all followers ACK-ed
            if(TotalACKsRx == TotalPLSent)
            {
                cancelEvent(plnTIMER7);
                TotalACKsRx = 0;
                TotalPLSent = 0;

                setVehicleState(state_splitDone);

                split_DataFSM();
            }
        }
    }
    else if(vehicleState == state_platoonFollower)
    {
        // splitting vehicle receives a SPLIT_REQ from the leader
        if (wsm->getUCommandType() == SPLIT_REQ && wsm->getReceiverID() == SUMOID)
        {
            // send SPLIT_ACCEPT
            sendPltData(myPlnID, SPLIT_ACCEPT, myPlnID);

            setVehicleState(state_waitForCHANGEPL);

            scheduleAt(omnetpp::simTime() + WAIT_FOR_RES, plnTIMER5);
        }
    }
    else if(vehicleState == state_waitForCHANGEPL)
    {
        if (wsm->getUCommandType() == CHANGE_PL && wsm->getSenderID() == myPlnID && wsm->getReceiverID() == SUMOID)
        {
            cancelEvent(plnTIMER5);

            // save my old platoon leader id for future use
            oldPlnID = myPlnID;

            // I am a free agent now!
            myPlnID = wsm->getValue().newPltLeader;
            myPlnDepth += wsm->getValue().newPltDepth;
            plnSize = 1;

            if(myPlnID == "")
                throw omnetpp::cRuntimeError("vehicle '%s' has an empty platoon id", SUMOID.c_str());

            if(myPlnDepth != 0)
                throw omnetpp::cRuntimeError("vehicle '%s' has a platoon depth other than zero", SUMOID.c_str());

            // announce it to the SUMO ASAP
            TraCI->vehiclePlatoonInit(SUMOID, std::deque<std::string> () = {SUMOID});

            // change color to red!
            RGB newColor = Color::colorNameToRGB("red");
            TraCI->vehicleSetColor(SUMOID, newColor);

            setVehicleState(state_sendingACK);

            split_DataFSM();
        }
    }
    else if(vehicleState == state_sendingACK)
    {
        // send ACK
        sendPltData(oldPlnID, ACK, myPlnID);

        setVehicleState(state_waitForSplitDone);

        scheduleAt(omnetpp::simTime() + WAIT_FOR_RES, plnTIMER8);
    }
    else if(vehicleState == state_waitForSplitDone)
    {
        if(wsm->getUCommandType() == SPLIT_DONE && wsm->getSenderID() == oldPlnID && wsm->getReceiverID() == SUMOID)
        {
            cancelEvent(plnTIMER8);

            plnMembersList.clear();
            plnMembersList = wsm->getValue().myNewPltMembers;

            plnSize = plnMembersList.size();

            if(wsm->getValue().manualSplit)
                optPlnSize = plnSize;
            else
                optPlnSize = wsm->getValue().optPlnSize;

            maxPlnSize = wsm->getValue().maxSize;

            if(optPlnSize < 1)
                throw omnetpp::cRuntimeError("vehicle '%s' has an invalid optPlnSize of '%d'", SUMOID.c_str(), optPlnSize);

            if(maxPlnSize < 1)
                throw omnetpp::cRuntimeError("vehicle '%s' has invalid maxPlnSize of '%d'", SUMOID.c_str(), maxPlnSize);

            updateColorDepth();

            TraCI->vehicleSetTimeGap(SUMOID, TP);

            TraCI->vehiclePlatoonInit(SUMOID, plnMembersList);

            // check splitCaller. If 'leader leave' is on-going
            if(wsm->getValue().caller == 0)
            {
                // then check if there is any leading vehicle after my leader
                auto leader = TraCI->vehicleGetLeader(oldPlnID, sonarDist);

                if(leader.leaderID == "")
                    TraCI->vehicleSetSpeed(SUMOID, 20.);
                else
                    TraCI->vehicleSetSpeed(SUMOID, 30.); // set max speed
            }
            else
                TraCI->vehicleSetSpeed(SUMOID, 30.); // set max speed

            setVehicleState(state_waitForGap);

            // check each updateInterval to see if the gap is big enough
            scheduleAt(omnetpp::simTime() + updateInterval, plnTIMER8a);
        }
    }
}


void ApplVPlatoonMg::RemoveFollowerFromList_Split(std::string followerID)
{
    bool found = false;
    unsigned int i = 0;

    for(i = 0; i < plnMembersList.size(); i++)
    {
        if(plnMembersList[i] == followerID)
        {
            found = true;
            break;
        }
    }

    if(found)
    {
        plnMembersList.erase(plnMembersList.begin() + i);
        plnSize--;
    }
}


bool ApplVPlatoonMg::GapCreated()
{
    // we use our sonar to check the gap
    auto leader = TraCI->vehicleGetLeader(SUMOID, sonarDist);

    if(leader.leaderID == "")
        return true;

    // get the timeGap setting
    double timeGapSetting = TraCI->vehicleGetTimeGap(SUMOID);

    // get speed
    double speed = TraCI->vehicleGetSpeed(SUMOID);

    // get minGap
    double minGap = TraCI->vehicleGetMinGap(SUMOID);

    double targetGap = (speed * timeGapSetting) + minGap;

    //cout << simTime().dbl() << ": " << SUMOID << ", speed = " << speed << ", targetGap = " << targetGap << ", gap = " << gap << endl;
    //if( gap >= targetGap ) cout << " BOOOM! " << endl;

    if( leader.distance2Leader >= targetGap )
        return true;
    else
        return false;
}


// -------------------------------------------------------------------------------------------
// --------------------------[ Leader Leave maneuver ]----------------------------------------
//--------------------------------------------------------------------------------------------

void ApplVPlatoonMg::leaderLeave_handleSelfMsg(omnetpp::cMessage* msg)
{
    if(!leaderLeaveEnabled)
        return;

    if(msg == plnTIMER9)
    {
        if(vehicleState == state_waitForVoteReply)
        {
            setVehicleState(state_sendVoteLeader);

            leaderLeave_DataFSM();
        }
    }
}


void ApplVPlatoonMg::leaderLeave_BeaconFSM(BeaconVehicle *wsm)
{
    if(!leaderLeaveEnabled)
        return;
}


void ApplVPlatoonMg::leaderLeave_DataFSM(PlatoonMsg *wsm)
{
    if(!leaderLeaveEnabled)
        return;

    if(vehicleState == state_sendVoteLeader)
    {
        // send a multicast VOTE_LEADER to all followers
        sendPltData("multicast", VOTE_LEADER, myPlnID);

        setVehicleState(state_waitForVoteReply, "LLeave_Start");

        scheduleAt(omnetpp::simTime() + WAIT_FOR_RES, plnTIMER9);
    }
    else if(vehicleState == state_waitForVoteReply)
    {
        if(wsm->getUCommandType() == ELECTED_LEADER && wsm->getReceiverID() == SUMOID)
        {
            cancelEvent(plnTIMER9);

            // todo: it is always 1 for now!
            splittingDepth = 1;
            splittingVehicle = plnMembersList[splittingDepth];
            splitCaller = 0;  // Notifying split that leader leave is the caller

            setVehicleState(state_sendSplitReq);

            split_DataFSM();
        }
    }
    else if(vehicleState == state_splitCompleted)
    {
        // now we can leave the platoon
        if(wsm->getUCommandType() == GAP_CREATED && wsm->getReceiverID() == SUMOID)
        {
            int32_t bitset = TraCI->vehicleBuildLaneChangeMode(10, 01, 01, 01, 01);
            TraCI->vehicleSetLaneChangeMode(SUMOID, bitset);  // alter 'lane change' mode

            performLaneChange();

            TraCI->vehicleSetSpeed(SUMOID, 30.);

            // change color to yellow!
            RGB newColor = Color::colorNameToRGB("yellow");
            TraCI->vehicleSetColor(SUMOID, newColor);

            myPlnID = "";
            myPlnDepth = -1;
            plnSize = -1;
            plnMembersList.clear();
            busy = false;
            leaveDirection = "";

            setVehicleState(state_idle, "LLeave_End");
        }
    }
    else if(vehicleState == state_platoonFollower)
    {
        if ( wsm->getUCommandType() == VOTE_LEADER && wsm->getSenderID() == myPlnID )
        {
            // todo:
            // we assume the second vehicle in the platoon always replies
            if(myPlnDepth == 1)
            {
                // send ELECTED_LEADER
                value_t entry;
                entry.myPltDepth = myPlnDepth;
                sendPltData(myPlnID, ELECTED_LEADER, myPlnID, entry);
            }
        }
    }
}


// -------------------------------------------------------------------------------------------
// ---------------------------[ Follower Leave maneuver ]-------------------------------------
//--------------------------------------------------------------------------------------------

void ApplVPlatoonMg::followerLeave_handleSelfMsg(omnetpp::cMessage* msg)
{
    if(!followerLeaveEnabled)
        return;

    if(msg == plnTIMER10)
    {
        if(vehicleState == state_waitForLeaveReply)
        {
            setVehicleState(state_sendLeaveReq);

            followerLeave_DataFSM();
        }
    }
    else if(msg == plnTIMER11)
    {
        // check if we are free agent?
        if(vehicleState == state_platoonLeader && plnSize == 1)
        {
            int32_t bitset = TraCI->vehicleBuildLaneChangeMode(10, 01, 01, 01, 01);
            TraCI->vehicleSetLaneChangeMode(SUMOID, bitset);  // alter 'lane change' mode

            performLaneChange();

            TraCI->vehicleSetSpeed(SUMOID, 30.);

            // change color to yellow!
            RGB newColor = Color::colorNameToRGB("yellow");
            TraCI->vehicleSetColor(SUMOID, newColor);

            myPlnID = "";
            myPlnDepth = -1;
            busy = false;
            leaveDirection = "";

            setVehicleState(state_idle, "FLeave_End");
        }
        else
            scheduleAt(omnetpp::simTime() + updateInterval, plnTIMER11);
    }
}


void ApplVPlatoonMg::followerLeave_BeaconFSM(BeaconVehicle *wsm)
{
    if(!followerLeaveEnabled)
        return;
}


void ApplVPlatoonMg::followerLeave_DataFSM(PlatoonMsg *wsm)
{
    if(!followerLeaveEnabled)
        return;

    if(vehicleState == state_sendLeaveReq)
    {
        // send a unicast LEAVE_REQ to the leader
        value_t entry;
        entry.myPltDepth = myPlnDepth;
        sendPltData(myPlnID, LEAVE_REQ, myPlnID, entry);

        setVehicleState(state_waitForLeaveReply, "FLeave_Request");

        scheduleAt(omnetpp::simTime() + WAIT_FOR_RES, plnTIMER10);
    }
    else if(vehicleState == state_waitForLeaveReply)
    {
        if(wsm->getUCommandType() == LEAVE_REJECT && wsm->getSenderID() == myPlnID)
        {
            cancelEvent(plnTIMER10);

            leaveDirection = "";

            setVehicleState(state_platoonFollower, "FLeave_Reject");
        }
        else if(wsm->getUCommandType() == LEAVE_ACCEPT && wsm->getSenderID() == myPlnID)
        {
            cancelEvent(plnTIMER10);

            if(wsm->getValue().lastFollower)
                setVehicleState(state_platoonFollower, "LFLeave_Start");
            else
                setVehicleState(state_platoonFollower, "MFLeave_Start");

            // now we should wait for the leader to do the split(s), and make us a free agent.
            // we check every updateInterval to see if we are free agent
            scheduleAt(omnetpp::simTime() + updateInterval, plnTIMER11);
        }
    }
    else if(vehicleState == state_platoonLeader)
    {
        // leader receives a LEAVE_REQ
        if (wsm->getUCommandType() == LEAVE_REQ && wsm->getReceiverID() == SUMOID)
        {
            if(wsm->getValue().myPltDepth <= 0 || wsm->getValue().myPltDepth >= plnSize)
                throw omnetpp::cRuntimeError("depth of the follower is not right!");

            busy = true;

            // send LEAVE_ACCEPT
            // lastFollower notifies the leaving vehicle if it is the last follower or not!
            value_t entry;
            entry.lastFollower = (wsm->getValue().myPltDepth + 1 == plnSize) ? true : false;
            sendPltData(wsm->getSenderID(), LEAVE_ACCEPT, myPlnID, entry);

            // last follower wants to leave (one split is enough)
            if(wsm->getValue().myPltDepth + 1 == plnSize)
            {
                RemainingSplits = 1;

                splittingDepth = wsm->getValue().myPltDepth;
                splittingVehicle = plnMembersList[splittingDepth];
                splitCaller = 1;  // Notifying split that follower leave is the caller

                setVehicleState(state_sendSplitReq);

                split_DataFSM();
            }
            // middle follower wants to leave (we need two splits)
            else
            {
                RemainingSplits = 2;

                // start the first split
                splittingDepth = wsm->getValue().myPltDepth + 1;
                splittingVehicle = plnMembersList[splittingDepth];
                splitCaller = 1;  // Notifying split that follower leave is the caller

                setVehicleState(state_sendSplitReq);

                split_DataFSM();
            }
        }
        // leader receives a GAP_CREATED
        else if(wsm->getUCommandType() == GAP_CREATED && wsm->getReceiverID() == SUMOID)
        {
            RemainingSplits--;

            if(RemainingSplits == 0)
            {
                // no more splits are needed. We are done!
                busy = false;
            }
            else if(RemainingSplits == 1)
            {
                // start the second split
                splittingDepth = plnSize - 1;
                splittingVehicle = plnMembersList[splittingDepth];
                splitCaller = 1;  // Notifying split that follower leave is the caller

                setVehicleState(state_sendSplitReq);

                split_DataFSM();
            }
        }
    }
}


void ApplVPlatoonMg::performLaneChange()
{
    // change lane to the right
    if(leaveDirection == "right")
    {
        bool can = TraCI->vehicleCouldChangeLane(SUMOID, -1);
        if(!can)
            throw omnetpp::cRuntimeError("Vehicle '%s' cannot change lane to right!", SUMOID.c_str());

        int lane = TraCI->vehicleGetLaneIndex(SUMOID);
        TraCI->vehicleChangeLane(SUMOID, lane-1, 5);
    }
    // change lane to the left
    else if(leaveDirection == "left")
    {
        bool can = TraCI->vehicleCouldChangeLane(SUMOID, 1);
        if(!can)
            throw omnetpp::cRuntimeError("Vehicle '%s' cannot change lane to left!", SUMOID.c_str());

        int lane = TraCI->vehicleGetLaneIndex(SUMOID);
        TraCI->vehicleChangeLane(SUMOID, lane+1, 5);
    }
    // change lane to whatever lane that is free
    else if(leaveDirection == "free")
    {
        bool can = TraCI->vehicleCouldChangeLane(SUMOID, -1);
        // we can change lane to right
        if(can)
        {
            int lane = TraCI->vehicleGetLaneIndex(SUMOID);
            TraCI->vehicleChangeLane(SUMOID, lane-1, 5);
        }
        else
        {
            bool can = TraCI->vehicleCouldChangeLane(SUMOID, 1);
            // we can change lane to left
            if(can)
            {
                int lane = TraCI->vehicleGetLaneIndex(SUMOID);
                TraCI->vehicleChangeLane(SUMOID, lane+1, 5);
            }
            else
                throw omnetpp::cRuntimeError("Vehicle '%s' is not allowed to change lane to right or left!", SUMOID.c_str());
        }
    }
    else
        throw omnetpp::cRuntimeError("Vehicle '%s' has an invalid leave direction '%s'", SUMOID.c_str(), leaveDirection.c_str());
}


// -------------------------------------------------------------------------------------------
// -----------------------------------[ Dissolve ]--------------------------------------------
//--------------------------------------------------------------------------------------------

void ApplVPlatoonMg::dissolve_handleSelfMsg(omnetpp::cMessage* msg)
{
    if(msg == plnTIMER12)
    {
        if(vehicleState == state_waitForDissolveAck)
        {
            setVehicleState(state_sendDissolve);

            dissolve_DataFSM();
        }
    }


}


void ApplVPlatoonMg::dissolve_BeaconFSM(BeaconVehicle* wsm)
{


}


void ApplVPlatoonMg::dissolve_DataFSM(PlatoonMsg* wsm)
{
    if(vehicleState == state_sendDissolve)
    {
        // get the last vehicle
        std::deque<std::string>::iterator it = plnMembersList.end() - 1;
        lastVeh = *it;

        // send a unicast DISSOLVE message my follower
        sendPltData(lastVeh, DISSOLVE, myPlnID);

        setVehicleState(state_waitForDissolveAck);

        scheduleAt(omnetpp::simTime() + WAIT_FOR_RES, plnTIMER12);
    }
    else if(vehicleState == state_waitForDissolveAck)
    {
        if (wsm->getUCommandType() == ACK && wsm->getSenderID() == lastVeh)
        {
            cancelEvent(plnTIMER12);

            plnMembersList.pop_back();

            if(plnMembersList.size() == 1)
                setVehicleState(state_platoonLeader);

            setVehicleState(state_sendDissolve);

            dissolve_DataFSM();
        }
    }
    else if(vehicleState == state_platoonFollower)
    {
        if ( wsm->getUCommandType() == DISSOLVE && wsm->getSenderID() == myPlnID && wsm->getReceiverID() == SUMOID )
        {
            // send ACK
            sendPltData(wsm->getSenderID(), ACK, wsm->getSendingPlatoonID());

            // make it a free agent
            myPlnID = SUMOID;
            myPlnDepth = 0;
            plnSize = 1;
            plnMembersList.push_back(SUMOID);
            TraCI->vehicleSetTimeGap(SUMOID, TP);

            busy = false;

            // change color to red!
            RGB newColor = Color::colorNameToRGB("red");
            TraCI->vehicleSetColor(SUMOID, newColor);

            setVehicleState(state_platoonLeader);

            TraCI->vehiclePlatoonInit(SUMOID, plnMembersList);
        }
    }
}

}
