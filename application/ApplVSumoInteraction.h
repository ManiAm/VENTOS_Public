
#ifndef ApplVSUMOINTERACTION_H
#define ApplVSUMOINTERACTION_H

#include "ApplVPlatoonFormation3.h"

class ApplVSumoInteraction : public ApplVPlatoonFormation3
{
	public:
        ~ApplVSumoInteraction();
		virtual void initialize(int stage);
        virtual void finish();

	protected:
        // NED variables (packet loss ratio)
        double droppT;
        std::string droppV;
        double plr;

        // NED variable
        bool SUMOdebug;
        bool modeSwitch;

        // NED variables (measurement error)
        bool measurementError;
        double errorGap;
        double errorRelSpeed;

        // Methods
        virtual void handleLowerMsg(cMessage*);
        virtual void handleSelfMsg(cMessage*);
        virtual void handlePositionUpdate(cObject*);

		virtual void onBeacon(Beacon*);
        virtual void onData(PlatoonMsg* wsm);

        bool dropBeacon(double time, std::string vehicle, double plr);
        void reportDropToStatistics(Beacon* wsm);
};

#endif
