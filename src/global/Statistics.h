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

#include "baseAppl/03_BaseApplLayer.h"
#include "traci/TraCICommands.h"

namespace VENTOS {

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

typedef struct MAC_stat
{
    double last_stat_time;

    long statsDroppedPackets;        // packet was dropped in Mac
    long statsNumTooLittleTime;      // Too little time in this interval. Will not schedule nextMacEvent
    long statsNumInternalContention; // there was already another packet ready. we have to go increase CW and go into backoff. It's called internal contention and its wonderful
    long statsNumBackoff;
    long statsSlotsBackoff;
    double statsTotalBusyTime;
    long statsSentPackets;
    long statsReceivedPackets;     // received a data packet addressed to me
    long statsReceivedBroadcasts;  // received a broadcast data packet
} MAC_stat_t;

typedef struct PHY_stat
{
    double last_stat_time;

    long statsSentFrames;
    long statsReceivedFrames;
    long statsBiteErrorLostFrames;
    long statsCollisionLostFrames;
    long statsTXRXLostFrames;
} PHY_stat_t;

typedef struct msgTxRxStat
{
    std::string msgName;
    std::string senderNode;
    std::string receiverNode;
    int frameSize;
    double sentAt;
    double TxSpeed;
    double TxTime;
    double propagationDelay;
    double receivedAt;
    std::string receivedStatus;
} msgTxRxStat_t;


class Statistics : public BaseApplLayer
{
public:
    std::vector<BeaconStat_t> global_Beacon_stat;

    std::vector<plnManagement_t> global_plnManagement_stat;
    std::vector<plnStat_t> global_plnData_stat;

    std::map<std::string /*vehId*/, MAC_stat_t> global_MAC_stat;
    std::map<std::string /*vehId*/, PHY_stat_t> global_PHY_stat;
    std::map<std::pair<long int /*msg id*/, long int /*nicId of receiver*/>, msgTxRxStat_t> global_frameTxRx_stat;

protected:
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

protected:
    void initialize_withTraCI();
    void executeEachTimestep();

private:
    void save_plnManage_toFile();
    void save_plnStat_toFile();

    void save_beacon_stat_toFile();

    void save_MAC_stat_toFile();
    void save_PHY_stat_toFile();
    void save_FrameTxRx_stat_toFile();
};

}

#endif
