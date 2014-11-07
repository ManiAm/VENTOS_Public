
#ifndef APPLVCOORDINATOR_H
#define APPLVCOORDINATOR_H

#include "ApplV_06_PlatoonMg.h"

namespace VENTOS {

class ApplVCoordinator : public ApplVPlatoonMg
{
	public:
        ~ApplVCoordinator();
		virtual void initialize(int stage);
        virtual void finish();

	protected:
        // Methods
        virtual void handleSelfMsg(cMessage*);
        virtual void handlePositionUpdate(cObject*);

		virtual void onBeaconVehicle(BeaconVehicle*);
        virtual void onBeaconRSU(BeaconRSU*);
        virtual void onData(PlatoonMsg* wsm);

	private:
        void coordinator();

        void scenario2();
        void scenario3();
        void scenario4();
        void scenario5();
        void scenario6();
        void scenario7();
        void scenario8();
        void scenario9();

        int coordinationMode;
        cMessage* coordination;
        static double stopTime;
};

}

#endif
