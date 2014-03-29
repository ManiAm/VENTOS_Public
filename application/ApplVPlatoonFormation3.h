
#ifndef ApplVPlatoonFORMATION3_H
#define ApplVPlatoonFORMATION3_H

// platoon merge

#include "ApplVPlatoonFormation2.h"


class ApplVPlatoonFormation3 : public ApplVPlatoonFormation2
{
	public:
        ~ApplVPlatoonFormation3();
		virtual void initialize(int stage);
        virtual void finish();

	protected:
        virtual void handleLowerMsg(cMessage* msg);
        virtual void handleSelfMsg(cMessage* msg);
        virtual void handlePositionUpdate(cObject* obj);

		virtual void onBeacon(Beacon* wsm);
		virtual void onData(PlatoonMsg* wsm);

		void FSMchangeState();

	private:
        // NED variables


        // Class variables

};

#endif
