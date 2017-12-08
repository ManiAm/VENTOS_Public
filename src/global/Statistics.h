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

typedef struct plnConfig
{
    double timestamp = -1;
    std::string vehId = "";
    int pltMode = -1;
    std::string pltId = "";
    int pltSize = -1;
    int pltDepth = -1;
    int optSize = -1;
    int maxSize = -1;
} plnConfig_t;

typedef struct plnStateChange
{
    double time;
    std::string vehId;
    std::string fromState;
    std::string toState;
} plnStateChange_t;

typedef struct plnDataExchange
{
    double time;
    std::string senderId;
    std::string sendingPltId;
    std::string senderState;
    std::string command;
    std::string receiverId;
    std::string receivingPltId;
} plnDataExchange_t;

typedef struct plnManeuverDuration
{
public:
    double time;
    std::string vehId;
    std::string vehState;
    std::string maneuver;
} plnManeuverDuration_t;

typedef struct BeaconStat
{
    double time;
    std::string senderID;
    std::string receiverID;
    bool dropped;
} BeaconStat_t;

typedef struct MAC_stat
{
    double last_stat_time;

    long NumDroppedFrames;      // packet was dropped from the EDCA queue
    long NumTooLittleTime;      // too little time in this interval. Will not schedule nextMacEvent
    long NumInternalContention; // there was already another packet ready. we have to go increase CW and go into backoff. It's called internal contention and its wonderful
    long NumBackoff;
    long SlotsBackoff;
    double TotalBusyTime;
} MAC_stat_t;

typedef struct PHY_stat
{
    double last_stat_time;

    long NumSentFrames;
    long NumReceivedFrames;
    long NumLostFrames_BiteError;
    long NumLostFrames_Collision;
    long NumLostFrames_TXRX;
} PHY_stat_t;

typedef struct msgTxRxStat
{
    std::string MsgName;
    std::string SenderNode;
    std::string ReceiverNode;
    double SentAt;
    int FrameSize;
    int channelNum;
    double TransmissionPower;
    double TransmissionSpeed;
    double TransmissionTime;
    double DistanceToReceiver;
    double PropagationDelay;
    double ReceivedAt;
    std::string FrameRxStatus;
} msgTxRxStat_t;


class Statistics : public BaseApplLayer
{
public:
    std::vector<BeaconStat_t> global_Beacon_stat;

    std::vector<plnConfig_t> global_plnConfig_stat;
    std::vector<plnStateChange_t> global_plnStateChange_stat;
    std::vector<plnDataExchange_t> global_plnDataExchange_stat;
    std::vector<plnManeuverDuration_t> global_plnManeuverDuration_stat;

    std::map<std::string /*vehId*/, MAC_stat_t> global_MAC_stat;
    std::map<std::string /*vehId*/, PHY_stat_t> global_PHY_stat;
    std::map<std::pair<uint32_t /*msg id*/, int32_t /*nicId of receiver*/>, msgTxRxStat_t> global_frameTxRx_stat;

    uint32_t departedVehicleCount = 0; // accumulated number of departed vehicles
    uint32_t arrivedVehicleCount = 0;  // accumulated number of arrived vehicles
    uint32_t activeVehicleCount = 0;   // number of active vehicles (be it parking or driving) at current time step
    uint32_t parkingVehicleCount = 0;  // number of parking vehicles at current time step
    uint32_t drivingVehicleCount = 0;  // number of driving vehicles at current time step

    uint32_t departedPersonCount = 0;
    uint32_t arrivedPersonCount = 0;
    uint32_t activePersonCount = 0;

private:
    // NED variables
    TraCI_Commands *TraCI;

    // class variables (signals)
    omnetpp::simsignal_t Signal_initialize_withTraCI;
    omnetpp::simsignal_t Signal_executeEachTS;
    omnetpp::simsignal_t Signal_module_added;
    omnetpp::simsignal_t Signal_arrived;

    double updateInterval = 0;

    typedef struct sim_status_entry
    {
        double timeStep = -1;
        long int loaded = -1;
        long int departed = -1;
        long int arrived = -1;
        long int running = -1;
        long int waiting = -1;
    } sim_status_entry_t;

    bool record_sim_stat;
    std::vector<std::string> record_sim_tokenize;
    std::vector<sim_status_entry_t> sim_record_status;

    typedef struct veh_record_list
    {
        bool active;  // should we record statistics for this vehicle?
        std::vector<std::string> record_list;  // type of data we need to record for this vehicle
    } veh_record_list_t;

    std::map<std::string /*SUMO id*/, veh_record_list_t> record_status;

    typedef struct veh_data_entry
    {
        double timeStep = -1;
        std::string vehId = "";
        std::string vehType = "";
        std::string lane = "";
        double lanePos = -1;
        std::string pos = "";
        double speed = -1;
        double accel = std::numeric_limits<double>::infinity();
        double departure = -1;
        double arrival = -1;
        std::string route = "";
        double routeDuration = -1;
        double drivingDistance = -1;
        std::string CFMode = "";
        double timeGapSetting = -1;
        double timeGap = std::numeric_limits<double>::infinity();
        double frontSpaceGap = std::numeric_limits<double>::infinity();
        double rearSpaceGap = std::numeric_limits<double>::infinity();
        std::string nextTLId = "";    // TLid that controls this vehicle. Empty string means the vehicle is not controlled by any TLid
        char nextTLLinkStat = '\0';   // status of the TL ahead (character 'n' means no TL ahead)
    } veh_data_entry_t;

    std::vector<veh_data_entry_t> collected_veh_data;
    std::map<std::string, int /*order*/> veh_data_columns;

    typedef struct veh_emission_list
    {
        bool active;  // should we record emission for this vehicle?
        std::vector<std::string> emission_list;  // type of emission we need to record for this vehicle
    } veh_emission_list_t;

    std::map<std::string /*SUMO id*/, veh_emission_list_t> record_emission;

    typedef struct veh_emission_entry
    {
        double timeStep = -1;
        std::string vehId = "";
        std::string emissionClass = "";
        double CO2 = -1;
        double CO = -1;
        double HC = -1;
        double PMx = -1;
        double NOx = -1;
        double fuel = -1;
        double noise = -1;
    } veh_emission_entry_t;

    std::vector<veh_emission_entry_t> collected_veh_emission;
    std::map<std::string, int /*order*/> veh_emission_columns;

public:
    virtual ~Statistics();
    virtual void finish();
    virtual void initialize(int);
    virtual void handleMessage(omnetpp::cMessage *);
    virtual void receiveSignal(omnetpp::cComponent *, omnetpp::simsignal_t, long, cObject *);
    virtual void receiveSignal(omnetpp::cComponent *, omnetpp::simsignal_t, const char *, cObject *);
    virtual void receiveSignal(omnetpp::cComponent *, omnetpp::simsignal_t, cObject *, cObject *);

private:
    void save_plnConfig_toFile();
    void save_plnData_toFile();

    void save_beacon_stat_toFile();

    void save_MAC_stat_toFile();
    void save_PHY_stat_toFile();
    void save_FrameTxRx_stat_toFile();

    void init_Sim_data();
    void record_Sim_data();
    void save_Sim_data_toFile();

    void init_Veh_data(std::string SUMOID, omnetpp::cModule *mod);
    void record_Veh_data(std::string vID, bool arrived = false);
    void save_Veh_data_toFile();

    void init_Ped_data(std::string SUMOID, omnetpp::cModule *mod);
    void record_Ped_data(std::string vID, bool arrived = false);

    void init_Veh_emission(std::string SUMOID, omnetpp::cModule *mod);
    void record_Veh_emission(std::string vID);
    void save_Veh_emission_toFile();
};

}

#endif
