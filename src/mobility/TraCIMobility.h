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

#ifndef VEINS_MOBILITY_TRACI_TRACIMOBILITY_H
#define VEINS_MOBILITY_TRACI_TRACIMOBILITY_H

#include <string>
#include <fstream>
#include <list>
#include <stdexcept>

#include "mobility/BaseMobility.h"
#include "traci/TraCICommands.h"

/**
 * @brief
 * Used in modules created by the TraCIScenarioManager.
 *
 * This module relies on the TraCIScenarioManager for state updates
 * and can not be used on its own.
 *
 * See the Veins website <a href="http://veins.car2x.org/"> for a tutorial, documentation, and publications </a>.
 *
 * @author Christoph Sommer, David Eckhoff, Luca Bedogni, Bastian Halmos, Stefan Joerer
 *
 * @see TraCIScenarioManager
 * @see TraCIScenarioManagerLaunchd
 *
 * @ingroup mobility
 */

namespace VENTOS {

#define TRACI_SIGNAL_PARKING_CHANGE_NAME "parkingStateChanged"

class TraCIMobilityMod : public BaseMobility
{
protected:

    bool debug; /**< whether to emit debug messages */

    bool isPreInitialized; /**< true if preInitialize() has been called immediately before initialize() */

    std::string external_id; /**< updated by setExternalId() */
    double antennaPositionOffset; /**< front offset for the antenna on this car */

    omnetpp::simtime_t lastUpdate; /**< updated by nextPosition() */
    Coord roadPosition; /**< position of front bumper, updated by nextPosition() */
    std::string road_id; /**< updated by nextPosition() */
    double speed; /**< updated by nextPosition() */
    double angle; /**< updated by nextPosition() */
    VehicleSignal vehSignals; /**<updated by nextPosition() */

    const static simsignalwrap_t parkingStateChangedSignal;

    bool isParking = false;

public:

    TraCIMobilityMod() : BaseMobility(), isPreInitialized(false) { }
    ~TraCIMobilityMod() { }
    virtual void initialize(int);
    virtual void finish();
    virtual void handleSelfMsg(omnetpp::cMessage *msg);

    virtual void preInitialize(std::string, const Coord&, std::string = "", double = -1, double = -1);
    virtual void nextPosition(const Coord&, std::string = "", double = -1, double = -1, VehicleSignal = VEH_SIGNAL_UNDEF);
    virtual void changePosition();
    virtual void changeParkingState(bool);
    virtual void updateDisplayString();
    virtual void setExternalId(std::string external_id) { this->external_id = external_id; }
    virtual std::string getExternalId() const
    {
        if (external_id == "")
            throw omnetpp::cRuntimeError("TraCIMobility::getExternalId called with no external_id set yet");
        return external_id;
    }
    virtual double getAntennaPositionOffset() const { return antennaPositionOffset; }
    virtual Coord getPositionAt(const omnetpp::simtime_t& t) const { return move.getPositionAt(t); }
    virtual bool getParkingState() const { return isParking; }
    virtual std::string getRoadId() const
    {
        if (road_id == "")
            throw omnetpp::cRuntimeError("TraCIMobility::getRoadId called with no road_id set yet");
        return road_id;
    }
    virtual double getSpeed() const
    {
        if (speed == -1)
            throw omnetpp::cRuntimeError("TraCIMobility::getSpeed called with no speed set yet");
        return speed;
    }
    virtual VehicleSignal getSignals() const
    {
        if (vehSignals == -1)
            throw omnetpp::cRuntimeError("TraCIMobility::getSignals called with no signals set yet");
        return vehSignals;
    }
    /**
     * returns angle in rads, 0 being east, with -M_PI <= angle < M_PI.
     */
    virtual double getAngleRad() const
    {
        if (angle == M_PI)
            throw omnetpp::cRuntimeError("TraCIMobility::getAngleRad called with no angle set yet");
        return angle;
    }

protected:
    virtual void fixIfHostGetsOutside(); /**< called after each read to check for (and handle) invalid positions */

    /**
     * Calculates where the antenna of this car is, given its front bumper position
     */
    Coord calculateAntennaPosition(const Coord& vehiclePos) const;
};

}

#endif
