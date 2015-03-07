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

using namespace std;

namespace VENTOS {

class systemData : public cObject, noncopyable
{
    string edge;
    string node;
    string sender;
    int requestType;
    string recipient;
    list<string> edgeList;

public:
    systemData(string e, string n, string s, int r, string re)
    {
        edge = e;
        node = n;
        sender = s;
        requestType = r;
        recipient = re;
    }

    systemData(string e, string n, string s, int r, string re, list<string> el)
    {
        edge = e;
        node = n;
        sender = s;
        requestType = r;
        recipient = re;
        edgeList = el;
    }
    string getEdge()
    {
        return edge;
    }
    string getNode()
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
    list<string> getInfo()
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
    vector<long> vec;

    MacStat( vector<long> v)
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
      string vName;
      int time;
      bool end;

      TimeData(string vName, int time, bool end)
      {
          this->vName = vName;
          this->time = time;
          this->end = end;
      }
};


}

#endif

