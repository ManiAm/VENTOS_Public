//
// Copyright (C) 2006-2012 Christoph Sommer <christoph.sommer@uibk.ac.at>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// This program is free software; you can redistribute it and/or modify
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
#include "BaseWorldUtility.h"
#include "ConnectionManager.h"
#include "TraCICommands.h"

namespace VENTOS {

// used for equilibrium logic
class departedNodes
{
public:
    std::string vehicleId;
    std::string vehicleTypeId;
    std::string routeId;
    double pos;
    double speed;
    uint32_t lane;

    departedNodes(std::string str1, std::string str2, std::string str3, double d1, double d2, uint32_t i2)
    {
        this->vehicleId = str1;
        this->vehicleTypeId = str2;
        this->routeId = str3;
        this->pos = d1;
        this->speed = d2;
        this->lane = i2;
    }
};


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


class TraCI_Start :  public TraCI_Commands
{
public:
    TraCI_Start();
    ~TraCI_Start();

    virtual void initialize(int stage);
    virtual int numInitStages() const
    {
        return std::max(cSimpleModule::numInitStages(), 2);
    }
    virtual void finish();
    virtual void handleMessage(cMessage *msg);

private:
    void init_traci();
    void initRoi();
    void roiRSUs();
    void drawRoi();

    uint32_t getCurrentTimeMs();  // get current simulation time (in ms)
    void executeOneTimestep();    // read and execute all commands for the next timestep

    void processSimSubscription(std::string objectId, TraCIBuffer& buf);
    void processVehicleSubscription(std::string objectId, TraCIBuffer& buf);
    void processSubcriptionResult(TraCIBuffer& buf);

    void addModule(std::string nodeId, const Coord& position, std::string road_id = "", double speed = -1, double angle = -1);
    void addPedestriansToOMNET();
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

    void saveVehicleData(std::string);
    void vehiclesDataToFile();

private:
    // NED
    BaseWorldUtility* world;
    ConnectionManager* cc;
    bool collectVehiclesData;
    int vehicleDataLevel;

    bool debug;                /**< whether to emit debug messages */
    simtime_t connectAt;       /**< when to connect to TraCI server (must be the initial timestep of the server) */
    simtime_t firstStepAt;     /**< when to start synchronizing with the TraCI server (-1: immediately after connecting) */
    simtime_t updateInterval;  /**< time interval of hosts' position updates */

    std::string host;

    // NED variables
    double terminate;
    bool autoShutdown;    /**< Shutdown module as soon as no more vehicles are in the simulation */

    // NED (vehicles)
    std::string vehicleModuleType;
    std::string vehicleModuleName;
    std::string vehicleModuleDisplayString;

    // NED (bicycles)
    std::string bikeModuleType;
    std::string bikeModuleName;
    std::string bikeModuleDisplayString;

    // NED (pedestrians)
    std::string pedModuleType;
    std::string pedModuleName;
    std::string pedModuleDisplayString;

    double penetrationRate;

    // class variables
    uint32_t activeVehicleCount; /**< number of vehicles, be it parking or driving **/
    uint32_t parkingVehicleCount; /**< number of parking vehicles, derived from parking start/end events */
    uint32_t drivingVehicleCount; /**< number of driving, as reported by sumo */

    size_t nextNodeVectorIndex; /**< next OMNeT++ module vector index to use */
    std::map<std::string, cModule*> hosts; /**< vector of all hosts managed by us */
    std::set<std::string> subscribedVehicles; /**< all vehicles we have already subscribed to */
    std::set<std::string> subscribedPedestrians; /**< all pedestrians we have already subscribed to */
    std::list<std::string> allPedestrians;
    std::set<std::string> unEquippedHosts;

    cMessage* connectAndStartTrigger; /**< self-message scheduled for when to connect to TraCI server and start running */
    cMessage* executeOneTimestepTrigger; /**< self-message scheduled for when to next call executeOneTimestep */

    std::list<std::string> roiRoads; /**< which roads (e.g. "hwy1 hwy2") are considered to consitute the region of interest, if not empty */
    std::list<std::pair<TraCICoord, TraCICoord> > roiRects; /**< which rectangles (e.g. "0,0-10,10 20,20-30,30) are considered to consitute the region of interest, if not empty */
    double roiSquareSizeRSU;

    // class variables
    bool equilibrium_vehicle;
    std::map<std::string, departedNodes> addedNodes;
    std::vector<VehicleData> Vec_vehiclesData;
};

}

#endif
