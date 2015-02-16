
#ifndef RSUADD_H_
#define RSUADD_H_

#include "TraCI_Extend.h"
#include <BaseApplLayer.h>

namespace VENTOS {

class RSUEntry
{
  public:
      char name[20];
      double coordX;
      double coordY;

      RSUEntry(const char *str, double x, double y)
      {
          strcpy(this->name, str);
          this->coordX = x;
          this->coordY = y;
      }
};

class TraCI_Extend;

class AddRSU : public BaseModule
{
	public:
		virtual ~AddRSU();
		virtual void initialize(int stage);
        virtual void handleMessage(cMessage *msg);
		virtual void finish();
        virtual void receiveSignal(cComponent *, simsignal_t, long);

	private:
        void Add();
        void Scenario1();
        deque<RSUEntry*> commandReadRSUsCoord(string);
        void commandAddCirclePoly(string, string, const TraCIColor& color, Coord*, double);

	private:
        // NED variables
        cModule *nodePtr;   // pointer to the Node
        TraCI_Extend *TraCI;  // pointer to the TraCI module
        simsignal_t Signal_executeFirstTS;
        bool on;
        int mode;
        deque<RSUEntry*> RSUs;

        boost::filesystem::path VENTOS_FullPath;
        boost::filesystem::path SUMO_Path;
        boost::filesystem::path SUMO_FullPath;
};

}

#endif
