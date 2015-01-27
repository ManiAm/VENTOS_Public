
#ifndef _MAC1609_4_MOD_H_
#define _MAC1609_4_MOD_H_

#include <Mac1609_4.h>
#include <Global_06_Statistics.h>

using namespace std;


namespace VENTOS {

class Mac1609_4_Mod : public Mac1609_4
{
	public:
		~Mac1609_4_Mod() { };

	protected:
		virtual void initialize(int);

	    virtual void handleSelfMsg(cMessage*);

		virtual void handleLowerMsg(cMessage*);
	    virtual void handleLowerControl(cMessage* msg);

		virtual void handleUpperMsg(cMessage*);
		virtual void handleUpperControl(cMessage* msg);

        virtual void finish();

	private:
        cModule *nodePtr;   // pointer to the Node
        Statistics *StatPtr;
        bool collectMAClayerData;
};
}

#endif
