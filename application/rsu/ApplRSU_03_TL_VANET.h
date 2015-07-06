/****************************************************************************/
/// @file    ApplRSU_03_TL_VANET.h
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author  second author name
/// @date    August 2013
///
/****************************************************************************/
// VENTOS, Vehicular Network Open Simulator; see http:?
// Copyright (C) 2013-2015
/****************************************************************************/
//
// This file is part of VENTOS.
// VENTOS is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#ifndef APPLRSUTLVANET_H_
#define APPLRSUTLVANET_H_

#include "ApplRSU_02_AID.h"

namespace VENTOS {

class queueData
{
  public:
    std::string vehicleName;
    std::string lane;
    std::string TLid;
    double entryTime;
    double leaveTime;
    double entrySpeed;

    queueData(std::string str1, std::string str2="", std::string str3="", double entryT=-1, double leaveT=-1, double entryS=-1)
    {
        this->vehicleName = str1;
        this->lane = str2;
        this->TLid = str3;
        this->entryTime = entryT;
        this->leaveTime = leaveT;
        this->entrySpeed = entryS;
    }

    friend bool operator== (const queueData &v1, const queueData &v2)
    {
        return ( v1.vehicleName == v2.vehicleName );
    }
};


class ApplRSUTLVANET : public ApplRSUAID
{
	public:
		~ApplRSUTLVANET();
		virtual void initialize(int stage);
		virtual void finish();
        virtual void handleSelfMsg(cMessage* msg);

	protected:
        void virtual executeEachTimeStep(bool);

        virtual void onBeaconVehicle(BeaconVehicle*);
        virtual void onBeaconBicycle(BeaconBicycle*);
        virtual void onBeaconPedestrian(BeaconPedestrian*);
        virtual void onBeaconRSU(BeaconRSU*);
        virtual void onData(LaneChangeMsg*);

	private:
        void setDetectionRegion();
        template <typename T> void onBeaconAny(T wsm);
        static void saveVehApproach();

	public:
        std::map<std::string /*lane*/, double /*time*/> detectedTime;
        std::map<std::string,double> passageTimePerLane;

	private:
        bool collectVehApproach;
        int TLControlMode;
        double minGreenTime;

        std::map<std::string /*lane*/, std::string /*TLid*/> lanesTL;  // all incoming lanes belong to each intersection
        static std::vector<queueData> Vec_queueData;    // common in all RSUs
};

}

#endif
