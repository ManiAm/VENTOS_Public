/****************************************************************************/
/// @file    AddNode.h
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author  second author name
/// @date    Jan 2017
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

#ifndef AddNode_H_
#define AddNode_H_

#include <rapidxml.hpp>
#include <rapidxml_utils.hpp>
#include <rapidxml_print.hpp>

#include "traci/TraCICommands.h"
#include "baseAppl/03_BaseApplLayer.h"

#define LANECHANGEMODE_DEFAULT   597  // 10 01 01 01 01
#define LANECHANGEMODE_STOPPED   533  // 10 00 01 01 01
#define LANECHANGEMODE_OBSTACLE  0


namespace VENTOS {

struct veh_deferred_attributes_t
{
    // -1 and "" means not defined

    int DSRC_status = -1;    // 0: without DSRC, 1: with DSRC
    std::string ipv4 = "";   // if the vehicle is emulated?
    std::string color = "";

    int plnMode = -1;        // platooning mode
    int plnDepth = -1;       // position of this vehicle in platoon
    std::string plnId = "";  // platoon id
    int plnSize = -1;        // platoon size
    int maxSize = -1;        // maximum platoon size
    int optSize = -1;        // optimal platoon size
    double interGap = -1;    // time-gap between platoons
};

struct adversaryEntry_t
{
    std::string id_str;
    TraCICoord pos;
    bool drawMaxIntfDist;
    std::string color_str;
    bool filled;
    omnetpp::cModule* module;
};

struct RSUEntry_t
{
    std::string id_str;
    TraCICoord pos;
    bool drawMaxIntfDist;
    std::string color_str;
    bool filled;
    omnetpp::cModule* module;
};

struct obstacleEntry_t
{
    std::string id_str;
    std::string edge_str;
    int lane;
    double lanePos;
    bool onRoad;
    int length;
    std::string color_str;
    double begin;
    double end;
    double duration;
};

struct vehicleEntry_t
{
    std::string id_str;
    std::string type_str;
    std::string routeID_str;
    std::string from_str;
    std::string to_str;
    std::vector<std::string> via_str_tokenize;
    std::string color_str;
    double depart;
    int departLane;
    double departPos;
    double departSpeed;
    int laneChangeMode;
    std::string status_str;
    double duration;
    double DSRCprob;
};

struct vehicleFlowEntry_t
{
    std::string id_str;
    std::vector<std::string> type_str_tokenize;
    std::vector<double> typeDist_tokenize;
    std::string routeID_str;
    std::string from_str;
    std::string to_str;
    std::vector<std::string> via_str_tokenize;
    std::string color_str;
    int departLane;
    double departPos;
    double departSpeed;
    int laneChangeMode;
    double begin;
    int number;
    double end;
    int seed;
    std::string distribution_str;
    double period;
    double lambda;
    double probability;
    double DSRCprob;
};

// vehicleMultiFlow does not have 'from', 'to', 'via'
struct vehicleMultiFlowEntry_t : vehicleFlowEntry_t
{
    std::vector<std::string> routeID_str_tokenize;
    std::vector<double> routeDist_tokenize;
};

struct vehiclePlatoonEntry_t
{
    std::string id_str;
    std::string type_str;
    int size;
    std::string routeID_str;
    std::string from_str;
    std::string to_str;
    std::vector<std::string> via_str_tokenize;
    std::string color_str;
    double depart;
    int departLane;
    double departPos;
    double platoonMaxSpeed;
    bool fastCatchUp;
    double interGap;
    bool pltMgmtProt;
    int maxSize;
    int optSize;
    std::map<int /*index*/, std::string /*veh type*/> platoonChild;

    // for internal use
    bool processed = false;
    uint32_t retryCount = 0;
};

struct CAEntry_t
{
    std::string id_str;
    TraCICoord pos;
    omnetpp::cModule* module;
};


class AddNode : public BaseApplLayer
{
private:
    typedef BaseApplLayer super;

protected:
    TraCI_Commands *TraCI;
    omnetpp::simsignal_t Signal_initialize_withTraCI;
    omnetpp::simsignal_t Signal_executeEachTS;

    double terminateTime = 0;
    double SUMO_timeStep = -1;
    int routeCalculation = 0;

    const std::string adversary_tag = "adversary";
    const std::string rsu_tag = "rsu";
    const std::string obstacle_tag = "obstacle";
    const std::string vehicle_tag = "vehicle";
    const std::string vehicle_flow_tag = "vehicle_flow";
    const std::string vehicle_multiFlow_tag = "vehicle_multiflow";
    const std::string vehicle_platoon_tag = "vehicle_platoon";
    const std::string ca_tag = "ca";

    std::string id;

    enum timer_types
    {
        TYPE_TIMER_OBSTACLE,
        TYPE_TIMER_STOPPED_VEHICLE
    };

    // list of all deferred attributes for each vehicle
    std::map<std::string /*SUMO id*/, veh_deferred_attributes_t> vehs_deferred_attributes;

    std::map<std::string, adversaryEntry_t> allAdversary;
    std::map<std::string, RSUEntry_t> allRSU;
    std::map<std::string, obstacleEntry_t> allObstacle;
    std::map<std::string, vehicleEntry_t> allVehicle;
    std::map<std::string, vehicleFlowEntry_t> allVehicleFlow;
    std::map<std::string, vehicleMultiFlowEntry_t> allVehicleMultiFlow;
    std::map<std::string, vehiclePlatoonEntry_t> allVehiclePlatoon;
    std::map<std::string, CAEntry_t> allCA;

public:
    static AddNode * getAddNodeInterface();

    veh_deferred_attributes_t getDeferredAttribute(std::string vehID);
    bool hasDeferredAttribute(std::string SUMOID);
    void addDeferredAttribute(std::string vehID, veh_deferred_attributes_t def);
    void removeDeferredAttribute(std::string vehID);
    void updateDeferredAttribute_ip(std::string vehID, std::string ip);
    void updateDeferredAttribute_color(std::string SUMOID, std::string color);

    adversaryEntry_t addNodeGetAdversary(std::string advID);
    RSUEntry_t addNodeGetRSU(std::string RSUID);
    obstacleEntry_t addNodeGetObstacle(std::string obsID);
    vehicleEntry_t addNodeGetVehicle(std::string vehID);
    vehicleFlowEntry_t addNodeGetVehicleFlow(std::string flowID);
    vehicleMultiFlowEntry_t addNodeGetMultiFlow(std::string multiFlowID);
    vehiclePlatoonEntry_t addNodeGetPlatoon(std::string platoonID);
    CAEntry_t addNodeGetCA(std::string CAID);

public:
    virtual ~AddNode();
    virtual void initialize(int stage);
    virtual void handleMessage(omnetpp::cMessage *msg);
    virtual void finish();
    virtual void receiveSignal(omnetpp::cComponent *, omnetpp::simsignal_t, long, cObject* details);

protected:
    virtual void readXMLFile(std::string);
    virtual void parsXMLFile(rapidxml::xml_node<> *);
    virtual void insertNodes();

private:
    void checkDuplicateModuleName(std::string);

    void printLoadedStatistics();

    void parseAdversary(rapidxml::xml_node<> *);
    void addAdversary();

    void parseRSU(rapidxml::xml_node<> *);
    void addRSU();

    void parseObstacle(rapidxml::xml_node<> *);
    void addObstacle();

    void parseVehicle(rapidxml::xml_node<> *);
    void addVehicle();
    std::string getShortestRoute(std::vector<std::string>);

    void parseVehicleFlow(rapidxml::xml_node<> *);
    void addVehicleFlow();
    std::string getVehType(vehicleFlowEntry_t, double);

    void parseVehicleMultiFlow(rapidxml::xml_node<> *);
    void addVehicleMultiFlow();
    std::string getVehRoute(vehicleMultiFlowEntry_t, double);

    void parseVehiclePlatoon(rapidxml::xml_node<> *);
    void parseVehiclePlatoonChild(rapidxml::xml_node<> *);
    void addVehiclePlatoon();
    double getPlatoonStartingPosition(vehiclePlatoonEntry_t &, int, std::string);
    bool checkPlatoonInsertion(vehiclePlatoonEntry_t &, int, std::string);

    void parseCA(rapidxml::xml_node<> *);
    void addCA();

    void addCircle(std::string, std::string, const RGB, bool, TraCICoord, double);
};

}

#endif
