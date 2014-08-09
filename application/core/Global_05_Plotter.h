
#ifndef PLOTTER_H
#define PLOTTER_H

#include <BaseModule.h>
#include <Appl.h>
#include "Global_01_TraCI_Extend.h"

namespace VENTOS {

class Plotter : public BaseModule
{
  public:
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
}

#endif
