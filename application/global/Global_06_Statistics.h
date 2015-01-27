
#ifndef STATISTICS_H
#define STATISTICS_H

#include <BaseApplLayer.h>
#include <ChannelAccess.h>
#include <WaveAppToMac1609_4Interface.h>
#include "modules/mobility/traci/TraCIMobility.h"

#include "Appl.h"
#include "TraCI_Extend.h"
#include "Histogram.h"


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


class VehicleData
{
  public:
    int index;
    double time;

    char vehicleName[20];
    char vehicleType[20];

    char lane[20];
    double pos;

    double speed;
    double accel;
    char CFMode[30];

    double timeGapSetting;
    double spaceGap;
    double timeGap;

    VehicleData(int i, double d1,
                 const char *str1, const char *str2,
                 const char *str3, double d2,
                 double d3, double d4, const char *str4,
                 double d3a, double d5, double d6)
    {
        this->index = i;
        this->time = d1;

        strcpy(this->vehicleName, str1);
        strcpy(this->vehicleType, str2);

        strcpy(this->lane, str3);
        this->pos = d2;

        this->speed = d3;
        this->accel = d4;
        strcpy(this->CFMode, str4);

        this->timeGapSetting = d3a;
        this->spaceGap = d5;
        this->timeGap = d6;
    }
};


class LoopDetector
{
  public:
    char detectorName[20];
    char vehicleName[20];
    double entryTime;
    double leaveTime;
    double entrySpeed;
    double leaveSpeed;

    LoopDetector( const char *str1, const char *str2, double entryT, double leaveT, double entryS, double leaveS )
    {
        strcpy(this->detectorName, str1);
        strcpy(this->vehicleName, str2);

        this->entryTime = entryT;
        this->leaveTime = leaveT;

        this->entrySpeed = entryS;
        this->leaveSpeed = leaveS;
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

	  void executeOneTimestep(bool); // should be public

      std::map<string, Histogram> edgeHistograms; // should be public


  private:
      void vehiclesData();
      void saveVehicleData(string);
      void vehiclesDataToFile();

      void HistogramsToFile();
      void parseHistogramFile();
      void laneCostsData();
      void processTravelTimeData();

      void inductionLoops();
      void inductionLoopToFile();

      void incidentDetectionToFile();

      void MAClayerToFile();

      void plnManageToFile();
      void plnStatToFile();





	  void postProcess();
      void printToFile();
      vector<NodeEntry *> SortByID(vector<NodeEntry *>);
      int getNodeIndex(const char*);

      int findInVector(vector<LoopDetector *> , const char *, const char *);
      int findInVector(vector<NodeEntry *>, const char *);
      int findInVector(vector<MacStatEntry *>, const char *);



      // NED variables
      TraCI_Extend *TraCI;
      double updateInterval;
      double terminate;

      // NED variables
      bool collectMAClayerData;
      bool collectVehiclesData;
      int LaneCostsMode;
      bool collectInductionLoopData;
      bool collectPlnManagerData;
      bool printBeaconsStatistics;
      bool printIncidentDetection;

      // class variables
      int index;

      // Edge weight-gathering
      std::map<string, string> vehicleEdges;
      std::map<string, double> vehicleTimes;

      //Hysteresis implementation
      std::map<string, int> vehicleLaneChangeCount;
      int HysteresisCount;

      // class variables (signals)
	  simsignal_t Signal_beaconP;
	  simsignal_t Signal_beaconO;
	  simsignal_t Signal_beaconD;

	  simsignal_t Signal_MacStats;

	  simsignal_t Signal_SentPlatoonMsg;
	  simsignal_t Signal_VehicleState;
	  simsignal_t Signal_PlnManeuver;
	  simsignal_t Signal_TimeData;

	  // class variables (vectors)
      vector<VehicleData *> Vec_vehiclesData;
      vector<LoopDetector *> Vec_loopDetectors;
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
};

}

#endif
