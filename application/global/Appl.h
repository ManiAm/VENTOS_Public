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
    SUMO_TAG_CF_CACC,   // 115
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
    char name[20];

    data(const char *str)
    {
        strcpy(this->name, str);
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
      char name[20];
      char state[40];

      CurrentVehicleState(const char *str1, const char *str2)
      {
          strcpy(this->name, str1);
          strcpy(this->state, str2);
      }
};


class CurrentPlnMsg : public cObject, noncopyable
{
  public:
      PlatoonMsg *msg;
      char type[30];

      CurrentPlnMsg(PlatoonMsg *m, const char *str)
      {
          this->msg = m;
          strcpy(this->type, str);
      }
};


class PlnManeuver : public cObject, noncopyable
{
  public:
      char from[20];
      char to[20];
      char maneuver[30];

      PlnManeuver(const char *str1, const char *str2, const char *str3)
      {
          strcpy(this->from, str1);
          strcpy(this->to, str2);
          strcpy(this->maneuver, str3);
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

