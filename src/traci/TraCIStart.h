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

#include "global/BaseWorldUtility.h"
#include "MIXIM_veins/connectionManager/ConnectionManager.h"
#include "traci/TraCICommands.h"
#include "global/Statistics.h"


namespace VENTOS {

class TraCI_Start :  public TraCI_Commands
{
private:
    typedef TraCI_Commands super;

    // NED
    bool active;  // run SUMO and establish TraCI?
    bool debug;
    double terminate;  // simulation end time (in seconds)
    bool autoShutdown;  // terminate simulation as soon as no more vehicles are in the simulation
    double penetrationRate;

    BaseWorldUtility* world = NULL;
    ConnectionManager* cc = NULL;
    cModule *addNode_module = NULL;
    Statistics* STAT = NULL;

    std::set<std::string> subscribedVehicles;    // all vehicles we have already subscribed to
    std::set<std::string> subscribedPedestrians; // all pedestrians we have already subscribed to
    std::vector<std::string> allPedestrians;

    std::set<std::string> unEquippedHosts;
    size_t nextNodeVectorIndex = 0;   // next OMNeT++ module vector index to use

    omnetpp::cMessage* executeOneTimestepTrigger = NULL; // self-message scheduled for when to next call executeOneTimestep

    std::list<std::string> roiRoads; // which roads (e.g. "hwy1 hwy2") are considered to constitute the region of interest, if not empty
    std::list<std::pair<TraCICoord, TraCICoord> > roiRects; // which rectangles (e.g. "0,0-10,10 20,20-30,30) are considered to constitute the region of interest, if not empty
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
    std::map<std::string /*SUMO id*/, departedNodes_t> departedVehicles;

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
    // initialize TraCI connection
    void init_traci();
    // initialize obstacles for PHY layer emulation
    void init_obstacles();
    // initialize 'region of interest'
    void init_roi();

    void processSubcriptionResult(TraCIBuffer& buf);
    void processSimSubscription(std::string objectId, TraCIBuffer& buf);
    void processVehicleSubscription(std::string objectId, TraCIBuffer& buf);

    void addModule(std::string nodeId, const Coord& position, std::string road_id = "", double speed = -1, double angle = -1);
    omnetpp::cModule* addVehicle(std::string nodeId, std::string type, std::string name, std::string displayString, std::string vClass, const Coord& position, std::string road_id, double speed, double angle);
    omnetpp::cModule* addPedestrian();
    void deleteManagedModule(std::string nodeId);

    // returns a pointer to the managed module named moduleName, or 0 if no module can be found
    cModule* getManagedModule(std::string nodeId);
    // returns true if this vehicle is un-equipped
    bool isModuleUnequipped(std::string nodeId);
    // returns whether a given position lies within the simulation's region of interest.
    // Modules are destroyed and re-created as managed vehicles leave and re-enter the ROI
    bool isInRegionOfInterest(const TraCICoord& position, std::string road_id, double speed, double angle);
};

}

#endif
