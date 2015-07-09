/****************************************************************************/
/// @file    Appl.h
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

#ifndef Appl_H
#define Appl_H

#include <list>
#include <msg/Messages_m.h>

namespace VENTOS {

enum ControllerTypes
{
    SUMO_TAG_CF_KRAUSS = 102,
    SUMO_TAG_CF_KRAUSS_PLUS_SLOPE,
    SUMO_TAG_CF_KRAUSS_ACCEL_BOUND,
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
    SUMO_TAG_CF_ACC,    // 114
    SUMO_TAG_CF_CACC    // 115
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

// SUMO signaling for vehicles
enum Signalling {
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

enum TLControlTypes
{
    TL_OFF,
    TL_Fix_Time,
    TL_Adaptive_Webster,
    TL_TrafficActuated,
    TL_LowDelay,
    TL_Adaptive_Time_Queue,
    TL_MultiClass,
    TL_Router
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

enum LaneCostsMode
{
    MODE_NOTHING,
    MODE_RECORD,
    MODE_EWMA
};

enum RouterMessage
{
    DIJKSTRA,
    HYPERTREE,
    DONE,
    STARTED
};

class systemData : public cObject, noncopyable
{
    std::string edge;
    std::string node;
    std::string sender;
    int requestType;
    std::string recipient;
    std::list<std::string> edgeList;

public:
    systemData(std::string e, std::string n, std::string s, int r, std::string re)
{
        edge = e;
        node = n;
        sender = s;
        requestType = r;
        recipient = re;
}

    systemData(std::string e, std::string n, std::string s, int r, std::string re, std::list<std::string> el)
    {
        edge = e;
        node = n;
        sender = s;
        requestType = r;
        recipient = re;
        edgeList = el;
    }
    std::string getEdge()
    {
        return edge;
    }
    std::string getNode()
    {
        return node;
    }
    const char* getSender()
    {
        return sender.c_str();
    }
    int getRequestType()
    {
        return requestType;
    }
    const char* getRecipient()
    {
        return recipient.c_str();
    }
    std::list<std::string> getInfo()
            {
        return edgeList;
            }
};


// for beacons
class data : public cObject, noncopyable
{
public:
    std::string sender;
    std::string receiver;
    bool dropped;

    data(std::string str1, std::string str2, bool d)
    {
        this->sender = str1;
        this->receiver = str2;
        this->dropped = d;
    }
};

class MacStat : public cObject, noncopyable
{
public:
    std::vector<long> vec;

    MacStat( std::vector<long> v)
    {
        vec.swap(v);
    }
};


class CurrentVehicleState : public cObject, noncopyable
{
public:
    std::string name;
    std::string state;

    CurrentVehicleState(std::string str1, std::string str2)
    {
        this->name = str1;
        this->state = str2;
    }
};


class CurrentPlnMsg : public cObject, noncopyable
{
public:
    PlatoonMsg *msg;
    std::string type;

    CurrentPlnMsg(PlatoonMsg *m, std::string str)
    {
        this->msg = m;
        this->type = str;
    }
};


class PlnManeuver : public cObject, noncopyable
{
public:
    std::string from;
    std::string to;
    std::string maneuver;

    PlnManeuver(std::string str1, std::string str2, std::string str3)
    {
        this->from = str1;
        this->to = str2;
        this->maneuver = str3;
    }
};

class TimeData : public cObject, noncopyable
{
public:
    std::string vName;
    int time;
    bool end;

    TimeData(std::string vName, int time, bool end)
    {
        this->vName = vName;
        this->time = time;
        this->end = end;
    }
};


}

#endif

