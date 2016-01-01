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

#include <BaseApplLayer.h>
#include <ChannelAccess.h>
#include <WaveAppToMac1609_4Interface.h>
#include "TraCI_Extend.h"

namespace VENTOS {

class Router;   // Forward-declaration so Statistics may hold a Router*

class VehicleData
{
public:
    double time;
    std::string vehicleName;
    std::string vehicleType;
    std::string lane;
    double pos;
    double speed;
    double accel;
    std::string CFMode;
    double timeGapSetting;
    double spaceGap;
    double timeGap;
    std::string TLid;  // TLid that controls this vehicle
    char linkStatus;  // status of the TL

    VehicleData(double t, std::string str1, std::string str2, std::string str3,
            double d2, double d3, double d4, std::string str4, double d3a, double d5, double d6,
            std::string str5, char st)
    {
        this->time = t;
        this->vehicleName = str1;
        this->vehicleType = str2;
        this->lane = str3;
        this->pos = d2;
        this->speed = d3;
        this->accel = d4;
        this->CFMode = str4;
        this->timeGapSetting = d3a;
        this->spaceGap = d5;
        this->timeGap = d6;
        this->TLid = str5;
        this->linkStatus = st;
    }
};


class MacStatEntry
{
public:
    double time;
    std::string name;
    int nodeID;  // is used to sort the std::vector (used as a key)
    std::vector<long> MacStatsVec;

    MacStatEntry(double t, std::string str, int id, std::vector<long> v)
    {
        this->time = t;
        this->name = str;
        this->nodeID = id;
        MacStatsVec.swap(v);
    }

    friend bool operator== (const MacStatEntry &v1, const MacStatEntry &v2)
    {
        return ( v1.name == v2.name );
    }
};


class plnManagement
{
public:
    double time;
    std::string sender;
    std::string receiver;
    std::string type;
    std::string sendingPlnID;
    std::string receivingPlnID;

    plnManagement(double t, std::string str1, std::string str2, std::string str3, std::string str4, std::string str5)
    {
        this->time = t;
        this->sender = str1;
        this->receiver = str2;
        this->type = str3;
        this->sendingPlnID = str4;
        this->receivingPlnID = str5;
    }
};


class plnStat
{
public:
    double time;
    std::string from;
    std::string to;
    std::string maneuver;

    plnStat(double t, std::string str1, std::string str2, std::string str3)
    {
        this->time = t;
        this->from = str1;
        this->to = str2;
        this->maneuver = str3;
    }
};


class BeaconStat
{
public:
    double time;
    std::string senderID;
    std::string receiverID;
    bool dropped;

    BeaconStat(double t, std::string str1, std::string str2, bool b)
    {
        this->time = t;
        this->senderID = str1;
        this->receiverID = str2;
        this->dropped = b;
    }
};


class Statistics : public BaseApplLayer
{
public:
    virtual ~Statistics();
    virtual void finish();
    virtual void initialize(int);
    virtual void handleMessage(cMessage *);
    virtual void receiveSignal(cComponent *, simsignal_t, long);
    virtual void receiveSignal(cComponent *, simsignal_t, cObject *);

private:
    // NED variables
    bool collectVehiclesData;
    int vehicleDataLevel;
    bool reportMAClayerData;
    bool reportPlnManagerData;
    bool reportBeaconsData;

    // NED variables
    TraCI_Extend *TraCI;
    double updateInterval;
    double terminate;
    Router* router;

    // class variables (signals)
    simsignal_t Signal_executeFirstTS;
    simsignal_t Signal_executeEachTS;
    simsignal_t Signal_MacStats;
    simsignal_t Signal_SentPlatoonMsg;
    simsignal_t Signal_VehicleState;
    simsignal_t Signal_PlnManeuver;
    simsignal_t Signal_beacon;

    // class variables
    std::list<std::string> TLList;      // list of traffic-lights in the network
    std::list<std::string> lanesList;   // list of all lanes in the network

    // class variables (vectors)
    std::vector<VehicleData> Vec_vehiclesData;
    std::vector<MacStatEntry> Vec_MacStat;
    std::vector<plnManagement> Vec_plnManagement;
    std::vector<plnStat> Vec_plnStat;
    std::vector<BeaconStat> Vec_Beacons;

private:
    void executeFirstTimeStep();
    void executeEachTimestep();

    void vehiclesData();
    void saveVehicleData(std::string);
    void vehiclesDataToFile();

    void MAClayerToFile();

    void plnManageToFile();
    void plnStatToFile();

    void beaconToFile();

    int getNodeIndex(std::string);
};

}

#endif
