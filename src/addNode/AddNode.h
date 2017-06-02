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

typedef struct veh_deferred_attributes
{
    // -1 means not defined

    int DSRC_status = -1; // 0: without DSRC, 1: with DSRC
    int plnMode = -1;     // platooning mode
    int maxSize = -1;     // maximum platoon size
    int optSize = -1;     // optimal platoon size
}veh_deferred_attributes_t;

class AddNode : public BaseApplLayer
{
private:
    typedef BaseApplLayer super;

    TraCI_Commands *TraCI;
    omnetpp::simsignal_t Signal_initialize_withTraCI;

    double terminateTime = 0;
    double updateInterval = -1;
    double SUMO_timeStep = 0;
    int routeCalculation = 0;

    const std::string adversary_tag = "adversary";
    const std::string rsu_tag = "rsu";
    const std::string obstacle_tag = "obstacle";
    const std::string vehicle_tag = "vehicle";
    const std::string vehicle_flow_tag = "vehicle_flow";
    const std::string vehicle_multiFlow_tag = "vehicle_multiflow";
    const std::string vehicle_platoon_tag = "vehicle_platoon";
    const std::string emulated_tag = "emulated";
    const std::string ca_tag = "ca";

    std::string id;

    enum timer_types
    {
        TYPE_TIMER_OBSTACLE,
        TYPE_TIMER_STOPPED_VEHICLE
    };

    typedef struct adversaryEntry
    {
        std::string id_str;
        TraCICoord pos;
        bool drawMaxIntfDist;
        std::string color_str;
        bool filled;
        cModule* module;
    } adversaryEntry_t;

    std::map<std::string, adversaryEntry_t> allAdversary;

    typedef struct RSUEntry
    {
        std::string id_str;
        TraCICoord pos;
        bool drawMaxIntfDist;
        std::string color_str;
        bool filled;
        cModule* module;
    } RSUEntry_t;

    std::map<std::string, RSUEntry_t> allRSU;

    typedef struct obstacleEntry
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
    } obstacleEntry_t;

    std::map<std::string, obstacleEntry_t> allObstacle;

    typedef struct vehicleEntry
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
    } vehicleEntry_t;

    std::map<std::string, vehicleEntry_t> allVehicle;

    typedef struct vehicleFlowEntry
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
    } vehicleFlowEntry_t;

    std::map<std::string, vehicleFlowEntry_t> allVehicleFlow;

    // vehicleMultiFlow does not have 'from', 'to', 'via'
    typedef struct vehicleMultiFlowEntry : vehicleFlowEntry
    {
        std::vector<std::string> routeID_str_tokenize;
        std::vector<double> routeDist_tokenize;
    } vehicleMultiFlowEntry_t;

    std::map<std::string, vehicleMultiFlowEntry_t> allVehicleMultiFlow;

    typedef struct vehiclePlatoonEntry
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
    } vehiclePlatoonEntry_t;

    std::map<std::string, vehiclePlatoonEntry_t> allVehiclePlatoon;

    typedef struct emulatedEntry
    {
        std::string id_str;
        std::string ip_str;
        std::string color_str;
    } emulatedEntry_t;

    std::map<std::string, emulatedEntry_t> allEmulated;

    typedef struct CAEntry
    {
        std::string id_str;
        TraCICoord pos;
        cModule* module;
    } CAEntry_t;

    std::map<std::string, CAEntry_t> allCA;

public:
    // list of all deferred attributes for each vehicle
    std::map<std::string /*SUMO id*/, veh_deferred_attributes_t> vehs_deferred_attributes;

public:
    virtual ~AddNode();
    virtual void initialize(int stage);
    virtual void handleMessage(omnetpp::cMessage *msg);
    virtual void finish();
    virtual void receiveSignal(omnetpp::cComponent *, omnetpp::simsignal_t, long, cObject* details);

private:
    void readInsertion(std::string);
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
    std::string getOverlappedPlatoon(vehiclePlatoonEntry_t &, int, std::string);

    void parseCA(rapidxml::xml_node<> *);
    void addCA();

    void parseEmulated(rapidxml::xml_node<> *);
    void addEmulated();

    void addCircle(std::string, std::string, const RGB, bool, TraCICoord, double);
};

}

#endif
