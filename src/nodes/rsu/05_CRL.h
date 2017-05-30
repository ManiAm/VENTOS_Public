/****************************************************************************/
/// @file    CRL.h
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

#include "nodes/rsu/04_AID.h"
#include "msg/CRL_Piece_m.h"

namespace VENTOS {

class ApplRSUCRL : public ApplRSUAID
{
private:
    typedef ApplRSUAID super;

    // parameters
    int CRLdistAlg;
    double CRL_Interval;
    double I2V_tho;
    double bitrate;

    // variables
    omnetpp::cMessage *Timer1 = NULL;   // beacon
    omnetpp::cMessage *Timer2 = NULL;   // CRL_Interval
    omnetpp::cMessage *Timer3 = NULL;

    omnetpp::simsignal_t Signal_CRL_pieces;

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

public:
    virtual ~ApplRSUCRL();
    virtual void initialize(int);
    virtual void finish();
    virtual void receiveSignal(omnetpp::cComponent *, omnetpp::simsignal_t, cObject *, cObject* details);
    virtual void handleSelfMsg(omnetpp::cMessage *);

protected:
    void virtual executeEachTimeStep();

    virtual void onBeaconVehicle(BeaconVehicle*);
    virtual void onBeaconBicycle(BeaconBicycle*);
    virtual void onBeaconPedestrian(BeaconPedestrian*);
    virtual void onBeaconRSU(BeaconRSU*);

    void recieveCRL(std::vector<CRL_Piece *>);

private:
    void broadcastCRL();
    void broadcastCRL_Mask();
    void broadcastCRL_Maskv2();

    void recieveBeacon(omnetpp::cMessage *);
    void sendBeacon_CRL();
    int Maximum();
    int IsExist(int);
};

}

#endif
