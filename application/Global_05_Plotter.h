
#ifndef PLOTTER_H
#define PLOTTER_H

#include <omnetpp.h>
#include "FindModule.h"
#include <BaseModule.h>
#include <Appl.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include "Global_01_TraCI_Extend.h"
#include <ApplRSU_03_Manager.h>
#include <stdlib.h>

using namespace std;

class Plotter : public BaseModule
{
  public:
      Plotter();
      virtual ~Plotter();
      virtual void finish();
      virtual void initialize(int);
      virtual void handleMessage(cMessage *);

      void speedProfile();

  private:
      TraCI_Extend *TraCI;
      bool on;
      FILE *pipe;
};

#endif
