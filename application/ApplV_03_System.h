
#ifndef ApplVSystem_H
#define ApplVSystem_H

#include "ApplV_02_Beacon.h"

class ApplVSystem : public ApplVBeacon
{

    simsignal_t Signal_router;

    public:
        ~ApplVSystem();
        virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj);
        virtual void initialize(int stage);
        virtual void finish();

    protected:

        // NED variables (beaconing parameters)
        bool requestRoutes;         //like sendBeacons;
        double requestInterval;     //like beaconInterval;
        double maxOffsetSystem;     //From beacon maxOffset
        int systemMsgLengthBits;    //Not sure how to derive this, yet
        int systemMsgPriority;      //like beaconPriority

        // Class variables
        simtime_t individualOffset;
        cMessage* sendSystemMsgEvt;

        // Routing
        std::string targetNode;

        // Methods
        virtual void handleSelfMsg(cMessage*);
        virtual void handlePositionUpdate(cObject*);

        virtual void onBeaconVehicle(BeaconVehicle*);
        virtual void onBeaconRSU(BeaconRSU*);
        virtual void onData(PlatoonMsg* wsm);

        virtual void onSystemMsg(SystemMsg*);

        SystemMsg* prepareSystemMsg();
        void printSystemMsgContent(SystemMsg*);
};

#endif

