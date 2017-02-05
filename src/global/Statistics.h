/****************************************************************************/
/// @file    Statistics.h
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

#ifndef STATISTICS_H
#define STATISTICS_H

#include "veins/modules/BaseApplLayer.h"
#include "veins/modules/ChannelAccess.h"
#include "veins/WaveAppToMac1609_4Interface.h"
#include "traci/TraCICommands.h"

namespace VENTOS {


typedef struct MAC_stat_entry
{
    double last_stat_time;

    long statsDroppedPackets;        // packet was dropped in Mac
    long statsNumTooLittleTime;      // Too little time in this interval. Will not schedule nextMacEvent
    long statsNumInternalContention; // there was already another packet ready.
    // we have to go increase CW and go into backoff.
    // It's called internal contention and its wonderful
    long statsNumBackoff;
    long statsSlotsBackoff;
    double statsTotalBusyTime;

    long statsSentPackets;

    long statsSNIRLostPackets;     // A packet was not received due to bit-errors
    long statsTXRXLostPackets;     // A packet was not received because we were sending while receiving

    long statsReceivedPackets;     // Received a data packet addressed to me
    long statsReceivedBroadcasts;  // Received a broadcast data packet

} MAC_stat_entry_t;

typedef struct BeaconStat
{
    double time;
    std::string senderID;
    std::string receiverID;
    bool dropped;
} BeaconStat_t;

typedef struct plnManagement
{
    double time;
    std::string sender;
    std::string receiver;
    std::string type;
    std::string sendingPlnID;
    std::string receivingPlnID;
} plnManagement_t;

typedef struct plnStat
{
public:
    double time;
    std::string from;
    std::string to;
    std::string maneuver;
} plnStat_t;


class Statistics : public BaseApplLayer
{
public:
    std::map<std::string, MAC_stat_entry_t> global_MAC_stat;
    std::vector<BeaconStat_t> global_Beacon_stat;

    std::vector<plnManagement_t> global_plnManagement_stat;
    std::vector<plnStat_t> global_plnData_stat;

private:
    // NED variables
    TraCI_Commands *TraCI;

    // class variables (signals)
    omnetpp::simsignal_t Signal_initialize_withTraCI;

public:
    virtual ~Statistics();
    virtual void finish();
    virtual void initialize(int);
    virtual void handleMessage(omnetpp::cMessage *);
    virtual void receiveSignal(omnetpp::cComponent *, omnetpp::simsignal_t, long, cObject* details);

private:
    void initialize_withTraCI();
    void executeEachTimestep();

    void save_plnManage_toFile();
    void save_plnStat_toFile();

    void save_MAC_stat_toFile();
    void save_beacon_stat_toFile();
};

}

#endif
