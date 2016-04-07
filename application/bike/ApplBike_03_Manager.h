/****************************************************************************/
/// @file    ApplBike_03_Manager.h
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

#ifndef ApplBIKEMANAGER_H
#define ApplBIKEMANAGER_H

#include "ApplBike_02_Beacon.h"
#include "BeaconVehicle_m.h"
#include "BeaconRSU_m.h"
#include "PlatoonMsg_m.h"

namespace VENTOS {

class ApplBikeManager : public ApplBikeBeacon
{
	public:
        ~ApplBikeManager();
		virtual void initialize(int stage);
        virtual void finish();
        virtual void receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj);

	protected:
        // Methods
        virtual void handleLowerMsg(cMessage*);
        virtual void handleSelfMsg(cMessage*);
        virtual void handlePositionUpdate(cObject*);

		virtual void onBeaconVehicle(BeaconVehicle*);
        virtual void onBeaconRSU(BeaconRSU*);
        virtual void onData(PlatoonMsg* wsm);

	private:
        static const simsignalwrap_t mobilityStateChangedSignal;
};

}

#endif
