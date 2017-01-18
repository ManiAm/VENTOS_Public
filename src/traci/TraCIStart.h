/****************************************************************************/
/// @file    TraCIStart.h
/// @author  Mani Amoozadeh   <maniam@ucdavis.edu>
/// @author  Christoph Sommer <mail@christoph-sommer.de>
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

#ifndef TRACISTART_H
#define TRACISTART_H

#include <queue>

#include "MIXIM/modules/BaseWorldUtility.h"
#include "MIXIM/connectionManager/ConnectionManager.h"
#include "traci/TraCICommands.h"

namespace VENTOS {

class TraCI_Start :  public TraCI_Commands
{
private:
    typedef TraCI_Commands super;

    // NED
    BaseWorldUtility* world;
    ConnectionManager* cc;
    cModule *addNode_module;

    bool active;
    bool debug;   /**< whether to emit debug messages */
    omnetpp::simtime_t connectAt;       /**< when to connect to TraCI server (must be the initial timestep of the server) */
    omnetpp::simtime_t firstStepAt;     /**< when to start synchronizing with the TraCI server (-1: immediately after connecting) */
    omnetpp::simtime_t updateInterval;  /**< time interval of hosts' position updates */

    // NED variables
    double terminate;
    bool autoShutdown;  /**< Shutdown module as soon as no more vehicles are in the simulation */

    double penetrationRate;

    // class variables
    uint32_t activeVehicleCount; /**< number of vehicles, be it parking or driving **/
    uint32_t parkingVehicleCount; /**< number of parking vehicles, derived from parking start/end events */
    uint32_t drivingVehicleCount; /**< number of driving, as reported by SUMO */

    size_t nextNodeVectorIndex; /**< next OMNeT++ module vector index to use */
    std::map<std::string, cModule*> hosts; /**< vector of all hosts managed by us */
    std::set<std::string> subscribedVehicles; /**< all vehicles we have already subscribed to */
    std::set<std::string> subscribedPedestrians; /**< all pedestrians we have already subscribed to */
    std::vector<std::string> allPedestrians;
    std::set<std::string> unEquippedHosts;

    omnetpp::cMessage* connectAndStartTrigger = NULL; /**< self-message scheduled for when to connect to TraCI server and start running */
    omnetpp::cMessage* executeOneTimestepTrigger = NULL; /**< self-message scheduled for when to next call executeOneTimestep */

    std::list<std::string> roiRoads; /**< which roads (e.g. "hwy1 hwy2") are considered to constitute the region of interest, if not empty */
    std::list<std::pair<TraCICoord, TraCICoord> > roiRects; /**< which rectangles (e.g. "0,0-10,10 20,20-30,30) are considered to constitute the region of interest, if not empty */
    double roiSquareSizeRSU;

    // used for equilibrium logic
    typedef struct departedNodes
    {
        std::string vehicleId;
        std::string vehicleTypeId;
        std::string routeId;
        double pos;
        double speed;
        uint32_t lane;
    } departedNodes_t;

    bool equilibrium_vehicle;
    std::map<std::string /*SUMO id*/, departedNodes> departedVehicles;

    typedef struct veh_status_entry
    {
        bool active;  // should we record stat for this vehicle?
        std::vector<std::string> record_list;  // type of data we need to record for this vehicle
    } veh_status_entry_t;

    std::map<std::string /*SUMO id*/, veh_status_entry_t> record_status;

    typedef struct veh_data_entry
    {
        double timeStep;
        std::string id;
        std::string type;
        std::string lane;
        double lanePos;
        double speed;
        double accel;
        std::string CFMode;
        double timeGapSetting;
        double spaceGap;
        double timeGap;
        std::string TLid;  // TLid that controls this vehicle. Empty string means the vehicle is not controlled by any TLid
        char linkStat;     // status of the TL ahead (character 'n' means no TL ahead)
    } veh_data_entry_t;

    std::vector<veh_data_entry_t> collected_veh_data;
    std::map<std::string, int /*order*/> allColumns;

public:
    TraCI_Start();

    virtual void initialize(int stage);
    virtual int numInitStages() const
    {
        return std::max(cSimpleModule::numInitStages(), 2);
    }
    virtual void finish();
    virtual void handleMessage(omnetpp::cMessage *msg);

private:
    void init_traci();
    void initRoi();
    void roiRSUs();
    void drawRoi();

    uint32_t getCurrentTimeMs();  // get current simulation time (in ms)
    void executeOneTimestep();    // read and execute all commands for the next timestep

    void processSubcriptionResult(TraCIBuffer& buf);
    void processSimSubscription(std::string objectId, TraCIBuffer& buf);
    void processVehicleSubscription(std::string objectId, TraCIBuffer& buf);

    void addModule(std::string nodeId, const Coord& position, std::string road_id = "", double speed = -1, double angle = -1);
    void addVehicle(std::string nodeId, std::string type, std::string name, std::string displayString, std::string vClass, const Coord& position, std::string road_id, double speed, double angle);
    void addPedestrian();
    void deleteManagedModule(std::string nodeId);

    /** returns a pointer to the managed module named moduleName, or 0 if no module can be found */
    cModule* getManagedModule(std::string nodeId);
    /** returns true if this vehicle is Unequipped */
    bool isModuleUnequipped(std::string nodeId);
    /**
     * returns whether a given position lies within the simulation's region of interest.
     * Modules are destroyed and re-created as managed vehicles leave and re-enter the ROI
     */
    bool isInRegionOfInterest(const TraCICoord& position, std::string road_id, double speed, double angle);

    void record_Veh_data(std::string);
    void save_Veh_data_toFile();
};

}

#endif
