/****************************************************************************/
/// @file    GlobalConsts.h
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

#ifndef GLOBALCONSTS_H
#define GLOBALCONSTS_H

namespace VENTOS {

enum WaveApplMessageTypes {
    TYPE_TIMER,
    TYPE_BEACON_VEHICLE,     // beaconVehicle
    TYPE_BEACON_BICYCLE,     // beaconBicycle
    TYPE_BEACON_PEDESTRIAN,  // beaconPedestrian
    TYPE_BEACON_RSU,         // beaconRSU
    TYPE_LANECHANGE_DATA,    // laneChange
    TYPE_PLATOON_DATA,       // platoonMsg
    TYPE_CRL_PIECE,
    TYPE_SAEJ2735_BSM,
    TYPE_BROADCAST_DATA,
};

enum ControllerTypes {
    SUMO_TAG_CF_KRAUSS = 107,
    SUMO_TAG_CF_KRAUSS_PLUS_SLOPE,
    SUMO_TAG_CF_KRAUSS_ORIG1,
    SUMO_TAG_CF_SMART_SK,
    SUMO_TAG_CF_DANIEL1,
    SUMO_TAG_CF_IDM,
    SUMO_TAG_CF_IDMM,
    SUMO_TAG_CF_PWAGNER2009,
    SUMO_TAG_CF_BKERNER,
    SUMO_TAG_CF_WIEDEMANN,

    SUMO_TAG_CF_OPTIMALSPEED,
    SUMO_TAG_CF_KRAUSSFIXED,
    SUMO_TAG_CF_ACC,
    SUMO_TAG_CF_CACC
};

enum CFMODES {
    Mode_Undefined,
    Mode_NoData,
    Mode_DataLoss,
    Mode_SpeedControl,
    Mode_GapControl,
    Mode_EmergencyBrake,
    Mode_Stopped
};


enum CRLdistAlgorithm {
    CRL_RSU_Only,
    CRL_C2C_Epidemic,
    CRL_MPB,
    CRL_ICE,
    CRL_ICEv2,
    CRL_Broadcast,
    CRL_BBroadcast,
    CRL_C2C_Epidemic_Ideal,
    CRL_NewMPB,
};

// SUMO signaling for vehicles
enum VehicleSignal {
    VEH_SIGNAL_UNDEF = -1,
    /// @brief Everything is switched off
    VEH_SIGNAL_NONE = 0,
    /// @brief Right blinker lights are switched on
    VEH_SIGNAL_BLINKER_RIGHT = 1,
    /// @brief Left blinker lights are switched on
    VEH_SIGNAL_BLINKER_LEFT = 2,
    /// @brief Blinker lights on both sides are switched on
    VEH_SIGNAL_BLINKER_EMERGENCY = 4,
    /// @brief The brake lights are on
    VEH_SIGNAL_BRAKELIGHT = 8,
    /// @brief The front lights are on (no visualisation)
    VEH_SIGNAL_FRONTLIGHT = 16,
    /// @brief The fog lights are on (no visualisation)
    VEH_SIGNAL_FOGLIGHT = 32,
    /// @brief The high beam lights are on (no visualisation)
    VEH_SIGNAL_HIGHBEAM = 64,
    /// @brief The backwards driving lights are on (no visualisation)
    VEH_SIGNAL_BACKDRIVE = 128,
    /// @brief The wipers are on
    VEH_SIGNAL_WIPER = 256,
    /// @brief One of the left doors is opened
    VEH_SIGNAL_DOOR_OPEN_LEFT = 512,
    /// @brief One of the right doors is opened
    VEH_SIGNAL_DOOR_OPEN_RIGHT = 1024,
    /// @brief A blue emergency light is on
    VEH_SIGNAL_EMERGENCY_BLUE = 2048,
    /// @brief A red emergency light is on
    VEH_SIGNAL_EMERGENCY_RED = 4096,
    /// @brief A yellow emergency light is on
    VEH_SIGNAL_EMERGENCY_YELLOW = 8192
};

enum LaneCostsMode {
    MODE_NOTHING,
    MODE_RECORD,
    MODE_EWMA
};

enum RouterMessage {
    DIJKSTRA,
    HYPERTREE,
    DONE,
    STARTED
};

}

#endif
