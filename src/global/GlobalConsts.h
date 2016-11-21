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

enum SUMOVehicleClass {
    /// @brief vehicles ignoring classes
    SVC_IGNORING = 0,

    /// @name vehicle ownership
    //@{

    /// @brief private vehicles
    SVC_PRIVATE = 1,
    /// @brief public emergency vehicles
    SVC_EMERGENCY = 1 << 1,
    /// @brief authorities vehicles
    SVC_AUTHORITY = 1 << 2,
    /// @brief army vehicles
    SVC_ARMY = 1 << 3,
    /// @brief vip vehicles
    SVC_VIP = 1 << 4,
    //@}


    /// @name vehicle size
    //@{

    /// @brief vehicle is a passenger car (a "normal" car)
    SVC_PASSENGER = 1 << 5,
    /// @brief vehicle is a HOV
    SVC_HOV = 1 << 6,
    /// @brief vehicle is a taxi
    SVC_TAXI = 1 << 7,
    /// @brief vehicle is a bus
    SVC_BUS = 1 << 8,
    /// @brief vehicle is a coach
    SVC_COACH = 1 << 9,
    /// @brief vehicle is a small delivery vehicle
    SVC_DELIVERY = 1 << 10,
    /// @brief vehicle is a large transport vehicle
    SVC_TRUCK = 1 << 11,
    /// @brief vehicle is a large transport vehicle
    SVC_TRAILER = 1 << 12,
    /// @brief vehicle is a light rail
    SVC_TRAM = 1 << 13,
    /// @brief vehicle is a city rail
    SVC_RAIL_URBAN = 1 << 14,
    /// @brief vehicle is a not electrified rail
    SVC_RAIL = 1 << 15,
    /// @brief vehicle is a (possibly fast moving) electric rail
    SVC_RAIL_ELECTRIC = 1 << 16,

    /// @brief vehicle is a motorcycle
    SVC_MOTORCYCLE = 1 << 17,
    /// @brief vehicle is a moped
    SVC_MOPED = 1 << 18,
    /// @brief vehicle is a bicycle
    SVC_BICYCLE = 1 << 19,
    /// @brief is a pedestrian
    SVC_PEDESTRIAN = 1 << 20,
    /// @brief is an electric vehicle
    SVC_E_VEHICLE = 1 << 21,
    /// @brief is an arbitrary ship
    SVC_SHIP = 1 << 22,
    /// @brief is a user-defined type
    SVC_CUSTOM1 = 1 << 23,
    /// @brief is a user-defined type
    SVC_CUSTOM2 = 1 << 24,
    //@}

    /// @brief classes which (normally) do not drive on normal roads
    SVC_NON_ROAD = SVC_TRAM | SVC_RAIL | SVC_RAIL_URBAN | SVC_RAIL_ELECTRIC | SVC_SHIP
};

enum VehicleStatus {
    VEH_STATUS_Driving = 0,
    VEH_STATUS_Waiting,
    VEH_STATUS_Parking,
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

enum TLControlTypes {
    TL_OFF,
    TL_Fix_Time,
    TL_Adaptive_Webster,
    TL_TrafficActuated,
    TL_LQF,
    TL_OJF,
    TL_LQF_MWM,
    TL_LQF_MWM_Aging,
    TL_FMSC,
    TL_Router
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

// LinkState in SUMO
enum LinkState {
    /// @brief The link has green light, may pass
    LINKSTATE_TL_GREEN_MAJOR = 'G',
    /// @brief The link has green light, has to brake
    LINKSTATE_TL_GREEN_MINOR = 'g',
    /// @brief The link has red light (must brake)
    LINKSTATE_TL_RED = 'r',
    /// @brief The link has red light (must brake) but indicates upcoming green
    LINKSTATE_TL_REDYELLOW = 'u',
    /// @brief The link has yellow light, may pass
    LINKSTATE_TL_YELLOW_MAJOR = 'Y',
    /// @brief The link has yellow light, has to brake anyway
    LINKSTATE_TL_YELLOW_MINOR = 'y',
    /// @brief The link is controlled by a tls which is off and blinks, has to brake
    LINKSTATE_TL_OFF_BLINKING = 'o',
    /// @brief The link is controlled by a tls which is off, not blinking, may pass
    LINKSTATE_TL_OFF_NOSIGNAL = 'O',
    /// @brief This is an uncontrolled, major link, may pass
    LINKSTATE_MAJOR = 'M',
    /// @brief This is an uncontrolled, minor link, has to brake
    LINKSTATE_MINOR = 'm',
    /// @brief This is an uncontrolled, right-before-left link
    LINKSTATE_EQUAL = '=',
    /// @brief This is an uncontrolled, minor link, has to stop
    LINKSTATE_STOP = 's',
    /// @brief This is an uncontrolled, all-way stop link.
    LINKSTATE_ALLWAY_STOP = 'w',
    /// @brief This is a dead end link
    LINKSTATE_DEADEND = '-'
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

