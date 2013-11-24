
#ifndef STATISTICS_H
#define STATISTICS_H

#include <omnetpp.h>

#include "FindModule.h"
#include "NetwControlInfo.h"

#include <BaseModule.h>

#include <ExtraClasses.h>

#include <sstream>
#include <iostream>
#include <fstream>

#include <TraCI_Extend.h>


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

  private:
	  void postProcess();
	  void printStatistics();
      void printToFile();
      std::vector<NodeEntry *> SortByID(std::vector<NodeEntry *>);
      int getNodeIndex(const char*);
      int findInVector(std::vector<NodeEntry *>, const char *);

      TraCI_Extend *manager;

      double updateInterval;
      double terminate;

      simsignal_t Signal_terminate;

	  simsignal_t Signal_beaconP;
	  simsignal_t Signal_beaconO;
	  simsignal_t Signal_beaconD;

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
};

#endif
