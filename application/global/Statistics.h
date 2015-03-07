/****************************************************************************/
/// @file    Statistics.h
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

#ifndef STATISTICS_H
#define STATISTICS_H

#include <BaseApplLayer.h>
#include <ChannelAccess.h>
#include <WaveAppToMac1609_4Interface.h>
#include "modules/mobility/traci/TraCIMobility.h"

#include "Appl.h"
#include "TraCI_Extend.h"


namespace VENTOS {

class MacStatEntry
{
public:
  char name[20];
  int nodeID;  // is used to sort the vector (used as a key)
  double time;
  vector<long> MacStatsVec;

  MacStatEntry(const char *str, int id, double t, vector<long> v)
  {
      strcpy(this->name, str);
      this->nodeID = id;
      this->time = t;
      MacStatsVec.swap(v);
  }
};


class NodeEntry
{
  public:
    char name1[20];
    int nodeID;  // is used to sort the vector (used as a key)

    char name2[20];
    int count;
    simtime_t time;

    NodeEntry(const char *str1, const char *str2, int id, int n, simtime_t t)
    {
        strcpy(this->name1, str1);
        this->nodeID = id;

        strcpy(this->name2, str2);
        this->count = n;
        this->time = t;
    }
};


class plnManagement
{
  public:
      double time;
      char sender[20];
      char receiver[20];
      char type[30];
      char sendingPlnID[20];
      char receivingPlnID[20];

      plnManagement(double t, const char *str1, const char *str2, const char *str3, const char *str4, const char *str5)
      {
          this->time = t;

          strcpy(this->sender, str1);
          strcpy(this->receiver, str2);
          strcpy(this->type, str3);
          strcpy(this->sendingPlnID, str4);
          strcpy(this->receivingPlnID, str5);
      }
};


class plnStat
{
  public:
      double time;
      char from[20];
      char to[20];
      char maneuver[20];

      plnStat(double d, const char *str1, const char *str2, const char *str3)
      {
          this->time = d;

          strcpy(this->from, str1);
          strcpy(this->to, str2);
          strcpy(this->maneuver, str3);
      }
};


class Statistics : public BaseModule
{
  public:
      virtual ~Statistics();
      virtual void finish();
      virtual void initialize(int);
      virtual void handleMessage(cMessage *);
	  virtual void receiveSignal(cComponent *, simsignal_t, long);
	  virtual void receiveSignal(cComponent *, simsignal_t, cObject *);

  private:
      // NED variables
      TraCI_Extend *TraCI;
      double updateInterval;
      double terminate;

      // NED variables
      bool collectMAClayerData;
      bool collectPlnManagerData;
      bool printBeaconsStatistics;

      // class variables (signals)
      simsignal_t Signal_executeEachTS;

	  simsignal_t Signal_beaconP;
	  simsignal_t Signal_beaconO;
	  simsignal_t Signal_beaconD;

	  simsignal_t Signal_MacStats;

	  simsignal_t Signal_SentPlatoonMsg;
	  simsignal_t Signal_VehicleState;
	  simsignal_t Signal_PlnManeuver;
	  simsignal_t Signal_TimeData;

	  // class variables (vectors)
      vector<MacStatEntry *> Vec_MacStat;
      vector<plnManagement *> Vec_plnManagement;
      vector<plnStat *> Vec_plnStat;

      vector<NodeEntry *> Vec_BeaconsP;    // beacons from proceeding
      vector<NodeEntry *> Vec_BeaconsO;    // beacons from other vehicles
      vector<NodeEntry *> Vec_BeaconsDP;   // Doped beacons from preceding vehicle
      vector<NodeEntry *> Vec_BeaconsDO;   // Doped beacons from other vehicles

      vector<NodeEntry *> totalBeaconsP;
      vector<NodeEntry *> totalBeaconsO;
      vector<NodeEntry *> totalBeaconsDP;
      vector<NodeEntry *> totalBeaconsDO;

      vector<NodeEntry *> beaconsDO_interval;
      vector<NodeEntry *> beaconsDP_interval;

  private:
      void executeEachTimestep(bool);

      void MAClayerToFile();

      void plnManageToFile();
      void plnStatToFile();

      void postProcess();
      void printToFile();
      vector<NodeEntry *> SortByID(vector<NodeEntry *>);
      int getNodeIndex(const char *ModName);

      int findInVector(vector<NodeEntry *>, const char *);
      int findInVector(vector<MacStatEntry *>, const char *);
};

}

#endif
