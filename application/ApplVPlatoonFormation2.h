
#ifndef ApplVPlatoonFORMATION2_H
#define ApplVPlatoonFORMATION2_H

#include "ApplVPlatoonFormation.h"


class ApplVPlatoonFormation2 : public ApplVPlatoonFormation
{
	public:
        ~ApplVPlatoonFormation2();
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
