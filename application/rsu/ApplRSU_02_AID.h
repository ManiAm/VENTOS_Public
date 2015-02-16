
#ifndef APPLRSUAID_H_
#define APPLRSUAID_H_

#include "ApplRSU_01_Base.h"

namespace VENTOS {

class ApplRSUAID : public ApplRSUBase
{
	public:
		~ApplRSUAID();
		virtual void initialize(int stage);
		virtual void finish();
	    virtual void receiveSignal(cComponent *, simsignal_t, long);

	protected:
        virtual void handleSelfMsg(cMessage* msg);

        virtual void onBeaconVehicle(BeaconVehicle*);
        virtual void onBeaconBicycle(BeaconBicycle*);
        virtual void onBeaconPedestrian(BeaconPedestrian*);
        virtual void onBeaconRSU(BeaconRSU*);
        virtual void onData(LaneChangeMsg*);

	private:
        static MatrixXi tableCount;
        static MatrixXd tableProb;

        simsignal_t Signal_executeEachTS;

        bool printIncidentDetection;

        void incidentDetectionToFile();
};

}

#endif
