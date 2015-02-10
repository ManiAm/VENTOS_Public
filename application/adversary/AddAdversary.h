
#ifndef ADVERSARYADD_H_
#define ADVERSARY_H_

#include "TraCI_Extend.h"
#include <BaseApplLayer.h>

namespace VENTOS {

class TraCI_Extend;

class AddAdversary : public BaseModule
{
	public:
		virtual ~AddAdversary();
		virtual void initialize(int stage);
        virtual void handleMessage(cMessage *msg);
		virtual void finish();
        virtual void receiveSignal(cComponent *, simsignal_t, long);

	private:
        // NED variables
        cModule *nodePtr;   // pointer to the Node
        TraCI_Extend *TraCI;  // pointer to the TraCI module
        simsignal_t Signal_executeFirstTS;
        bool on;
        int mode;

        void Add();
};

}

#endif
