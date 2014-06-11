
#ifndef STATISTICS_H
#define STATISTICS_H

#include <omnetpp.h>
#include "FindModule.h"
#include "NetwControlInfo.h"
#include <BaseModule.h>
#include <Appl.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include "Global_01_TraCI_Extend.h"
#include <ApplRSU.h>
#include <stdlib.h>

using namespace std;

class Statistics : public BaseModule
{
  public:
      Statistics();
      virtual ~Statistics();
      virtual void finish();
      virtual void initialize(int);
      virtual void handleMessage(cMessage *);
	  virtual void receiveSignal(cComponent *, simsignal_t, long);
	  virtual void receiveSignal(cComponent *, simsignal_t, cObject *);

	  void executeOneTimestep(bool);

  private:
	  void postProcess();
	  void printStatistics();
      void printToFile();
      vector<NodeEntry *> SortByID(vector<NodeEntry *>);
      int getNodeIndex(const char*);
      int findInVector(vector<NodeEntry *>, const char *);
      int findInVector(vector<MacStatEntry *>, const char *);
      void printAID();

      void vehiclesData();
      void writeToFile_PerVehicle(string, string, string);
      void inductionLoops();
      void writeToFile_InductionLoop();
      int findInVector(vector<LoopDetector *> , const char *, const char *);

      // NED variables
      TraCI_Extend *TraCI;
      double updateInterval;
      double terminate;

      // NED variables
      bool collectVehiclesData;
      bool collectInductionLoopData;
      bool printBeaconsStatistics;
      bool printIncidentDetection;

      // class variables
      int index;
      FILE *VehicleDataFile;

      // class variables (signals)
	  simsignal_t Signal_beaconP;
	  simsignal_t Signal_beaconO;
	  simsignal_t Signal_beaconD;

	  simsignal_t Signal_MacStats;

	  // class variables (vectors)
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

      vector<MacStatEntry *> Vec_MacStat;

      vector<LoopDetector *> Vec_loopDetectors;
};

#endif
