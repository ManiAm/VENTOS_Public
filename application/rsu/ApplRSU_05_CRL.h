/****************************************************************************/
/// @file    ApplRSU_05_CRL.h
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author  second author name
/// @date    Apr 2016
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

#ifndef APPLRSUCRL_H_
#define APPLRSUCRL_H_

#include "ApplRSU_04_AID.h"
#include "CRL_Piece_m.h"

namespace VENTOS {

class ApplRSUCRL : public ApplRSUAID
{
  public:
    virtual ~ApplRSUCRL();
    virtual void initialize(int);
    virtual void finish();
	virtual void handleSelfMsg(cMessage *);

  protected:
    void virtual executeEachTimeStep();

    virtual void onBeaconVehicle(BeaconVehicle*);
    virtual void onBeaconBicycle(BeaconBicycle*);
    virtual void onBeaconPedestrian(BeaconPedestrian*);
    virtual void onBeaconRSU(BeaconRSU*);
    virtual void onData(LaneChangeMsg*);

    void recieveCRL(std::vector<CRL_Piece *>);

  private:
    void broadcastCRL();
    void broadcastCRL_Mask();
    void broadcastCRL_Maskv2();

    void recieveBeacon(cMessage *);
    void sendBeacon();
    int Maximum();
    int IsExist(int);

  private:
    // parameters
    int CRLdistAlg;
    double CRL_Interval;
    double beacon_Interval;
    double I2V_tho;
    double bitrate;

    // variables
	cMessage *Timer1;   // beacon
	cMessage *Timer2;   // CRL_Interval
	cMessage *Timer3;

	int totalPieces = -1;
    unsigned int forCounter = 0;  // save forCounter between calls
	std::vector<CRL_Piece *> PiecesCRLfromCA;
	bool AnyoneNeedCRL = false;
    int *broadcastMask;  // for ICE

    enum ApplRSU_States
    {
        STATE_IDLE,  // 0
        STATE_WAIT,
        STATE_SENDING_CRLS,
        STATE_CRL_LIST_SEND
    };

    ApplRSU_States state = STATE_IDLE;  // state of the RSU
};

}

#endif
