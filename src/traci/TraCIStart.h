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
#include "addNode/AddNode.h"


namespace VENTOS {

class TraCI_Start :  public TraCI_Commands
{
private:
    typedef TraCI_Commands super;

protected:
    // NED
    bool active;  // run SUMO and establish TraCI?
    bool debug;
    double terminateTime;
    bool autoTerminate;

    BaseWorldUtility *world = NULL;
    ConnectionManager *cc = NULL;
    AddNode *ADDNODE = NULL;
    Statistics *STAT = NULL;

    omnetpp::simsignal_t Signal_departed_vehs;
    omnetpp::simsignal_t Signal_arrived_vehs;

    std::set<std::string> subscribedVehicles;    // all vehicles we have already subscribed to
    std::set<std::string> subscribedPerson;      // all person we have already subscribed to

    // next OMNeT++ module vector index to use
    size_t nextMotorVectorIndex = 0;
    size_t nextObstacleVectorIndex = 0;
    size_t nextBikeVectorIndex = 0;
    size_t nextPersonVectorIndex = 0;

    omnetpp::cMessage* sumo_step = NULL; // self-message scheduled for when to next call simulationTimeStep

    std::list<std::string> roiRoads; // which roads (e.g. "hwy1 hwy2") are considered to constitute the region of interest, if not empty
    std::list<std::pair<TraCICoord, TraCICoord> > roiRects; // which rectangles (e.g. "0,0-10,10 20,20-30,30) are considered to constitute the region of interest, if not empty
    double roiRectsRSU;

    // used for equilibrium logic
    struct departedNodes_t
    {
        std::string vehicleId;
        std::string vehicleTypeId;
        std::string routeId;
        double pos;
        double speed;
        uint32_t lane;
        colorVal_t color;
    };

    std::map<std::string /*SUMO id*/, departedNodes_t> equilibrium_departedVehs;

public:
    TraCI_Start();
    ~TraCI_Start();

    virtual void initialize(int stage);
    virtual int numInitStages() const
    {
        return std::max(cSimpleModule::numInitStages(), 3);
    }
    virtual void finish();
    virtual void handleMessage(omnetpp::cMessage *msg);
    virtual void receiveSignal(omnetpp::cComponent *, omnetpp::simsignal_t, const char *, cObject *);

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
    void processPersonSubscription(std::string objectId, TraCIBuffer& buf);

    void addVehicleModule(std::string nodeId, const Coord& position, std::string road_id, double speed, double angle);
    omnetpp::cModule* addVehicle(std::string nodeId, std::string type, std::string name, std::string displayString, int32_t nodeVectorIndex, std::string vClass, const Coord& position, std::string road_id, double speed, double angle);
    void addPerson(std::string nodeId, const Coord& position, std::string road_id, double speed, double angle);

    void deleteManagedModule(std::string nodeId);
    cModule* getManagedModule(std::string nodeId);

    // returns whether a given position lies within the simulation's region of interest.
    // Modules are destroyed and re-created as managed vehicles leave and re-enter the ROI
    bool isInRegionOfInterest(const TraCICoord& position, std::string road_id, double speed, double angle);

    bool checkEndSimulation(uint32_t);
};

}

#endif
