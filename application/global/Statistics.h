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

class Router;   //Forward-declaration so TraCI_App may hold a Router*

class VehicleData
{
  public:
    int index;
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
    int YorR;          // if the TL state ahead is yellow or red

    VehicleData(int i, double d1, std::string str1, std::string str2, std::string str3, double d2, double d3, double d4, std::string str4, double d3a, double d5, double d6, std::string str5, int YR)
    {
        this->index = i;
        this->time = d1;
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
        this->YorR = YR;
    }
};


class MacStatEntry
{
public:
  char name[20];
  int nodeID;  // is used to sort the std::vector (used as a key)
  double time;
  std::vector<long> MacStatsVec;

  MacStatEntry(const char *str, int id, double t, std::vector<long> v)
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
    int nodeID;  // is used to sort the std::vector (used as a key)

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
      bool collectVehiclesData;
      bool useDetailedFilenames;
      bool collectMAClayerData;
      bool collectPlnManagerData;
      bool printBeaconsStatistics;

      // NED variables
      TraCI_Extend *TraCI;
      double updateInterval;
      double terminate;
      Router* router;

      // class variables (signals)
      simsignal_t Signal_executeFirstTS;
      simsignal_t Signal_executeEachTS;

	  simsignal_t Signal_beaconP;
	  simsignal_t Signal_beaconO;
	  simsignal_t Signal_beaconD;

	  simsignal_t Signal_MacStats;

	  simsignal_t Signal_SentPlatoonMsg;
	  simsignal_t Signal_VehicleState;
	  simsignal_t Signal_PlnManeuver;
	  simsignal_t Signal_TimeData;

	  // class variables
      int index;
      std::list<std::string> TLList;   // list of traffic-lights in the network

      // class variables (vectors)
      std::vector<VehicleData> Vec_vehiclesData;
      std::vector<MacStatEntry *> Vec_MacStat;
      std::vector<plnManagement *> Vec_plnManagement;
      std::vector<plnStat *> Vec_plnStat;

      std::vector<NodeEntry *> Vec_BeaconsP;    // beacons from proceeding
      std::vector<NodeEntry *> Vec_BeaconsO;    // beacons from other vehicles
      std::vector<NodeEntry *> Vec_BeaconsDP;   // Doped beacons from preceding vehicle
      std::vector<NodeEntry *> Vec_BeaconsDO;   // Doped beacons from other vehicles

      std::vector<NodeEntry *> totalBeaconsP;
      std::vector<NodeEntry *> totalBeaconsO;
      std::vector<NodeEntry *> totalBeaconsDP;
      std::vector<NodeEntry *> totalBeaconsDO;

      std::vector<NodeEntry *> beaconsDO_interval;
      std::vector<NodeEntry *> beaconsDP_interval;

  private:
      void executeFirstTimeStep();
      void executeEachTimestep(bool);

      void vehiclesData();
      void saveVehicleData(std::string);
      void vehiclesDataToFile();

      void MAClayerToFile();

      void plnManageToFile();
      void plnStatToFile();

      void postProcess();
      void printToFile();
      std::vector<NodeEntry *> SortByID(std::vector<NodeEntry *>);
      int getNodeIndex(const char *ModName);

      int findInVector(std::vector<NodeEntry *>, const char *);
      int findInVector(std::vector<MacStatEntry *>, const char *);
};

}

#endif
