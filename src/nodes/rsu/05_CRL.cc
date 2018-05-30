/****************************************************************************/
/// @file    CRL.cc
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

#include "nodes/rsu/05_CRL.h"
#include "global/SignalObj.h"

namespace VENTOS {

Define_Module(VENTOS::ApplRSUCRL);

ApplRSUCRL::~ApplRSUCRL()
{
    cancelAndDelete(Timer1);
    cancelAndDelete(Timer2);
    cancelAndDelete(Timer3);
}

void ApplRSUCRL::initialize(int stage)
{
    super::initialize(stage);

    if(stage == 0) 
    {
        CRLdistAlg = par("CRLdistAlg").intValue();
        if(CRLdistAlg < 0 || CRLdistAlg >= NUM_CRL_ALG)
            throw omnetpp::cRuntimeError("Invalid CRLdistAlg!");

        if(CRLdistAlg == 0)
            return;

        CRL_Interval = par("CRL_Interval");
        if(CRL_Interval <= 0)
            throw omnetpp::cRuntimeError("value for CRL_Interval is incorrect !!");

        I2V_tho = par("I2V_tho");
        if(I2V_tho < 0)
            throw omnetpp::cRuntimeError("value for I2V_tho is incorrect !!");

        // get bitrate from the MAC layer
        bitrate = mac->par("bitrate");

        // RSUs broadcast CRL pieces periodically
        if(CRLdistAlg == CRL_RSU_Only || CRLdistAlg == CRL_C2C_Epidemic)
        {
            Timer2 = new omnetpp::cMessage("Timer_CRL_Interval_RSU");
            scheduleAt(omnetpp::simTime() + dblrand() * 10, Timer2);  // CRL broadcast start is random in each RSU
        }
        // otherwise RSUs send beacon
        else
        {
            Timer1 = new omnetpp::cMessage("Timer_Beacon_RSU");
            //scheduleAt(omnetpp::simTime() + dblrand() * beacon_Interval, Timer1); // todo

            Timer3 = new omnetpp::cMessage("Timer_Wait_Beacon_V");
        }

        Signal_CRL_pieces = registerSignal("CRL_pieces");
        omnetpp::getSimulation()->getSystemModule()->subscribe("CRL_pieces", this);
    }
}


void ApplRSUCRL::finish()
{
    super::finish();

    if(CRLdistAlg == 0)
        return;

    // unsubscribe
    omnetpp::getSimulation()->getSystemModule()->unsubscribe("CRL_pieces", this);
}


void ApplRSUCRL::receiveSignal(omnetpp::cComponent *source, omnetpp::simsignal_t signalID, cObject *obj, cObject* details)
{
    // CA sends this RSU a CRL
    if(signalID == Signal_CRL_pieces)
    {
        CRLPiecesData *m = static_cast<CRLPiecesData *>(obj);
        ASSERT(m);

        if(m->name == std::string(myFullId))
            recieveCRL(m->data);
    }
}


void ApplRSUCRL::handleSelfMsg(omnetpp::cMessage *msg)
{
    if(msg == Timer2)
    {
        broadcastCRL();
        scheduleAt(omnetpp::simTime() + CRL_Interval, Timer2);
    }
    else if(msg == Timer1)
    {
        sendBeacon_CRL();
    }
    else if(msg == Timer3)
    {
        if(state == STATE_WAIT)
        {
            if(CRLdistAlg == CRL_MPB)
            {
                broadcastCRL();

                // go back to state IDLE
                state = STATE_IDLE;
            }
            else if(CRLdistAlg == CRL_ICE)
            {
                broadcastCRL_Mask();

                AnyoneNeedCRL = false;

                // reset the broadcastMask
                for(int i=0; i<totalPieces;i++)
                    broadcastMask[i] = 0;

                // go back to state IDLE
                state = STATE_IDLE;
            }
            else if(CRLdistAlg == CRL_ICEv2)
            {
                broadcastCRL_Maskv2();

                AnyoneNeedCRL = false;

                // reset the broadcastMask
                for(int i=0; i<totalPieces;i++)
                    broadcastMask[i] = 0;

                // go back to state IDLE
                state = STATE_IDLE;
            }
        }
    }
    else
        super::handleSelfMsg(msg);
}


void ApplRSUCRL::executeEachTimeStep()
{
    super::executeEachTimeStep();
}


void ApplRSUCRL::onBeaconVehicle(BeaconVehicle* wsm)
{
    super::onBeaconVehicle(wsm);

    if(CRLdistAlg == 0)
        return;

    // todo
    //recieveBeacon(msg);
}


void ApplRSUCRL::onBeaconBicycle(BeaconBicycle* wsm)
{
    super::onBeaconBicycle(wsm);
}


void ApplRSUCRL::onBeaconPedestrian(BeaconPedestrian* wsm)
{
    super::onBeaconPedestrian(wsm);
}


void ApplRSUCRL::onBeaconRSU(BeaconRSU* wsm)
{
    super::onBeaconRSU(wsm);
}


// Receive CRL from the CA
void ApplRSUCRL::recieveCRL(std::vector<CRL_Piece *> data)
{
    if ( omnetpp::cSimulation::getActiveEnvir()->isGUI() )
    {
        LOG_INFO << boost::format(">>> %1% received these CRL pieces from CA: \n") % myFullId;

        LOG_INFO << "    ";
        for(auto &i : data)
            LOG_INFO << i->getSeqNo() << " ";

        LOG_INFO << "\n";
    }

    totalPieces = (int) data.size();
    broadcastMask = new int[totalPieces];
    for (int i=0; i<totalPieces; i++)
        broadcastMask[i] = 0;    // Initialize all elements to zero.

    // save this CRL piece for future periodic CRL sending
    PiecesCRLfromCA.clear();
    PiecesCRLfromCA = data;
}


// broadcast CRL pieces
void ApplRSUCRL::broadcastCRL()
{
    double time = 0;  // elapsed time
    int counter = 0;  // number of broadcasted pieces

    printf("\n>>> %s is broadcasting these CRL pieces: \n", myFullId);
    printf("    ");

    for(unsigned int i = 0; i < PiecesCRLfromCA.size(); i++)
    {
        CRL_Piece *pkt = PiecesCRLfromCA[forCounter]->dup();

        pkt->setName(myFullId);
        // pkt->setKind(Msg_CRL_RSU);  todo
        pkt->setBitLength(18400);

        // how many CRL pieces can we send in time tho.
        // I2V_tho == 0 means we don't have any restriction.
        if(I2V_tho != 0)
        {
            // transmission time for a single frame
            // 32 bits header for network layer and 192 bits for data-link layer
            double TxTime = ( pkt->getBitLength() + 32 + 272 + 192) / bitrate;
            time = time + TxTime;
            if(time >= I2V_tho)
            {
                // stop broadcasting pieces
                delete pkt;
                break;
            }
        }

        counter++;

        printf("%d  ", pkt->getSeqNo());

        // send the pkt
        send(pkt, lowerLayerOut);
        // sendDelayed(pkt, individualOffset, lowerLayerOut);  --> todo

        // start over if all pieces are broadcasted
        if(forCounter >= PiecesCRLfromCA.size()-1)
            forCounter = 0;
        else
            forCounter++;
    }

    std::cout << std::endl << std::flush;
}


// broadcast CRL pieces of the RSU with respect to broadcastMask[i] in ICE
void ApplRSUCRL::broadcastCRL_Mask()
{
    //    if(!AnyoneNeedCRL)
    //    {
    //        EV << "**** " << this->getParentModule()->getFullName() << " broadcasting canceled!" << endl;
    //        return;
    //    }
    //
    //    if ( ev.isGUI() )
    //    {
    //        this->getParentModule()->bubble("Sending CRLs");
    //        EV << "**** " << this->getParentModule()->getFullName();
    //        EV << ": sending pieces on channel " << nicClassPtr->getChannel() << endl;
    //
    //        EV << "broadcastMask is: ";
    //
    //        for(int h=0; h<totalPieces; h++)
    //        {
    //            EV << broadcastMask[h] << ", ";
    //        }
    //
    //        EV << endl;
    //
    //        EV << "broadcast CRL pieces are: ";
    //    }
    //
    //    double time = 0;
    //    int counter = 0;
    //
    //    // iterate on all pieces in PiecesCRLfromCA
    //    // note that the pieces are not sorted by SeqNo
    //    for(unsigned int i=0; i<PiecesCRLfromCA.size(); i++)
    //    {
    //        // if corresponding bit in Mask variable is zero, then skip sending this piece
    //        if(broadcastMask[PiecesCRLfromCA[i]->getSeqNo()] == 0)
    //            continue;
    //
    //        // create the packet for transmitting a certificate
    //        CRL_Piece *pkt = PiecesCRLfromCA[i]->dup();
    //
    //        pkt->setName(this->getParentModule()->getFullName());
    //        pkt->setKind(Msg_CRL_RSU);
    //        pkt->setBitLength(18400);
    //        pkt->setControlInfo( new NetwControlInfo(-1) );
    //
    //        // we have time constrain (tho).
    //        // we have to calculate how many CRL pieces can we send in tho.
    //        // the calculation considers size of frame and data rate.
    //        // I2V_tho == 0 means we don't have any restriction.
    //        if(I2V_tho != 0)
    //        {
    //            // transmission time for a single frame
    //            // (32 bits header for network layer and 192 bits for data-link layer)
    //            double TxTime = ( pkt->getBitLength() + 32 + 272 + 192) / bitrate;
    //            time = time + TxTime;
    //            if(time >= I2V_tho)
    //            {
    //                delete pkt;
    //                break;
    //            }
    //        }
    //
    //        counter++;
    //        EV << pkt->getSeqNo() << ", ";
    //
    //        // send the pkt
    //        send(pkt, lowerLayerOut);
    //    }
    //
    //    EV << endl << counter << " CRL pieces out of " << PiecesCRLfromCA.size() << " are sent." << endl;
    //
    //    // if one or more pieces sent
    //    if(counter > 0)
    //    {
    //        simsignal_t Signal_Broadcast_RSU = registerSignal("Broadcast_RSU");
    //        this->getParentModule()->emit(Signal_Broadcast_RSU, 1);
    //    }
}


// broadcast CRL pieces of the RSU with respect to broadcastMask[i] in ICEv2
void ApplRSUCRL::broadcastCRL_Maskv2()
{
    //    if(!AnyoneNeedCRL)
    //    {
    //        EV << "**** " << this->getParentModule()->getFullName() << " broadcasting canceled!" << endl;
    //        return;
    //    }
    //
    //    if ( ev.isGUI() )
    //    {
    //        this->getParentModule()->bubble("Sending CRLs");
    //        EV << "**** " << this->getParentModule()->getFullName();
    //        EV << ": sending pieces on channel " << nicClassPtr->getChannel() << endl;
    //
    //        EV << "broadcastMask in RSU is: ";
    //
    //        for(int h=0; h<totalPieces; h++)
    //        {
    //            EV << broadcastMask[h] << ", ";
    //        }
    //
    //        EV << endl;
    //
    //        EV << "broadcast CRL pieces are: ";
    //    }
    //
    //    double time = 0;
    //    int counter = 0;
    //
    //    // iterate on all elements of broadcastMask
    //    for(int i=0; i<totalPieces; i++)
    //    {
    //        // return the SeqNo of the piece with highest priority in broadcastMask
    //        int pieceIndex = Maximum();
    //
    //        // if there is no maximum, go out of the loop
    //        if (pieceIndex == -1)
    //            break;
    //
    //        // check to see if this piece is in the PiecesCRLcollected
    //        int result = IsExist(pieceIndex);
    //
    //        if(result == -1)
    //            continue;
    //
    //        // create the packet for transmitting a certificate
    //        CRL_Piece *pkt = PiecesCRLfromCA[result]->dup();
    //
    //        pkt->setName(this->getParentModule()->getFullName());
    //        pkt->setKind(Msg_CRL_RSU);
    //        pkt->setBitLength(18400);
    //        pkt->setControlInfo( new NetwControlInfo(-1) );
    //
    //        // we have time constrain (tho).
    //        // we have to calculate how many CRL pieces can we send in tho.
    //        // the calculation considers size of frame and data rate.
    //        // I2V_tho == 0 means we don't have any restriction.
    //        if(I2V_tho != 0)
    //        {
    //            // transmission time for a single frame
    //            // (32 bits header for network layer and 192 bits for data-link layer)
    //            double TxTime = ( pkt->getBitLength() + 32 + 272 + 192) / bitrate;
    //            time = time + TxTime;
    //            if(time >= I2V_tho)
    //            {
    //                delete pkt;
    //                break;
    //            }
    //        }
    //
    //        counter++;
    //        EV << pkt->getSeqNo() << ", ";
    //
    //        // send the pkt
    //        send(pkt, lowerLayerOut);
    //    }
    //
    //    EV << endl << counter << " CRL pieces out of " << PiecesCRLfromCA.size() << " are sent." << endl;
    //
    //    // if one or more pieces sent
    //    if(counter > 0)
    //    {
    //        simsignal_t Signal_Broadcast_RSU = registerSignal("Broadcast_RSU");
    //        this->getParentModule()->emit(Signal_Broadcast_RSU, 1);
    //    }

}


// receive beacon from vehicles
void ApplRSUCRL::recieveBeacon(omnetpp::cMessage *msg)
{
    //    Beacon *m = static_cast<Beacon *>(msg);
    //    if (m == NULL) return;
    //
    //    if(ev.isGUI())
    //    {
    //        EV << "*** " << this->getParentModule()->getFullName() << " received a beacon from " << m->getNodeName() << endl;
    //        this->getParentModule()->bubble("received a beacon");
    //
    //        EV << "*** Extracting beacon information: " << endl;
    //
    //        EV << "source Add: " << m->getSrcAddr() << endl;
    //        EV << "destination Add: " << m->getDestAddr() << endl;
    //        EV << "node name: " << m->getNodeName() << endl;
    //        EV << "position (x,y): " << m->getPositionX() << ", " << m->getPositionY() << endl;
    //        EV << "speed: " << m->getSpeed() << endl;
    //        EV << "number of pieces: " << m->getCRL_Piece_No() << endl;
    //        EV << "CRL channel: " << m->getCRLchannel()  << endl;
    //        EV << "Need CRL: " << m->getNeedCRL() << endl;
    //        EV << "Range Start: " << m->getRangeS() << endl;
    //        EV << "Range End: " << m->getRangeE() << endl;
    //    }
    //
    //    // if the algorithm is MPB
    //    if(CRLdistAlg == ApplV_MPB)
    //    {
    //        if(state == STATE_IDLE)
    //        {
    //            // RSU waits for 1s, to send a beacon to the vehicle
    //            // and by this the vehicle is notified about the piece_count of RSU
    //            scheduleAt(simTime() + 1, Timer3);
    //
    //            EV << endl << "*** " << this->getParentModule()->getFullName() << ": Waiting ..." << endl;
    //            state = STATE_WAIT;
    //        }
    //    }
    //    // if the algorithm is ICE
    //    else if(CRLdistAlg == ApplV_ICE)
    //    {
    //        // update AnyoneNeedCRL
    //        if (!AnyoneNeedCRL)
    //        {
    //            if(m->getRangeE() - m->getRangeS() == -1)
    //                AnyoneNeedCRL = false;
    //            else
    //                AnyoneNeedCRL = true;
    //        }
    //
    //        if(state == STATE_IDLE)
    //        {
    //            // update the broadcastMask
    //            for(int i = m->getRangeS(); i <= m->getRangeE(); i++)
    //                broadcastMask[i] = 1;
    //
    //            // RSU waits for 1s, to send a beacon to the vehicle
    //            // and by this the vehicle is notified about the range of RSU
    //            scheduleAt(simTime() + 1, Timer3);
    //
    //            EV << endl << "*** " << this->getParentModule()->getFullName() << ": Waiting ..." << endl;
    //            state = STATE_WAIT;
    //        }
    //        else if(state == STATE_WAIT)
    //        {
    //            // update the broadcastMask
    //            for(int i = m->getRangeS(); i <= m->getRangeE(); i++)
    //                broadcastMask[i] = 1;
    //        }
    //    }
    //    // if the algorithm is ICEv2
    //    if(CRLdistAlg == ApplV_ICEv2)
    //    {
    //        // update AnyoneNeedCRL
    //        if (!AnyoneNeedCRL)
    //        {
    //            if(m->getRangeE() - m->getRangeS() == -1)
    //                AnyoneNeedCRL = false;
    //            else
    //                AnyoneNeedCRL = true;
    //        }
    //
    //        if(state == STATE_IDLE)
    //        {
    //            // update the broadcastMask
    //            for(int i = m->getRangeS(); i <= m->getRangeE(); i++)
    //                broadcastMask[i] = broadcastMask[i] + 1;
    //
    //            // RSU waits for 1s, to send a beacon to the vehicle
    //            // and by this the vehicle is notified about the range of RSU
    //            scheduleAt(simTime() + 1, Timer3);
    //
    //            EV << endl << "*** " << this->getParentModule()->getFullName() << ": Waiting ..." << endl;
    //            state = STATE_WAIT;
    //        }
    //        else if(state == STATE_WAIT)
    //        {
    //            // update the broadcastMask
    //            for(int i = m->getRangeS(); i <= m->getRangeE(); i++)
    //                broadcastMask[i] = broadcastMask[i] + 1;
    //        }
    //    }
    //
    //    delete msg;
}


// send beacon (only in MPB and ICE)
void ApplRSUCRL::sendBeacon_CRL()
{
    //    EV << "**** " << this->getParentModule()->getFullName() << " is sending a broadcast beacon." << endl;
    //    if ( ev.isGUI() ) this->getParentModule()->bubble("Sending beacon");
    //
    //    simsignal_t Signal_Beacon_RSU = registerSignal("Beacon_RSU");
    //    this->getParentModule()->emit(Signal_Beacon_RSU, 1);
    //
    //    Beacon *pkt = new Beacon(this->getParentModule()->getFullName(), Msg_Beacon_RSU);
    //
    //    pkt->setSrcAddr(nodeID);
    //    pkt->setDestAddr(-1);   // its a broadcast beacon
    //
    //    pkt->setNodeName(this->getParentModule()->getFullName());
    //
    //    // get and then set the current position of the node
    //    int xPos = std::atoi( this->getParentModule()->getDisplayString().getTagArg("p",0) );  // x-coordinate of node
    //    int yPos = std::atoi( this->getParentModule()->getDisplayString().getTagArg("p",1) );  // y-coordinate of node
    //    pkt->setPositionX( xPos );
    //    pkt->setPositionY( yPos );
    //
    //    pkt->setSpeed(0);
    //
    //    pkt->setNeedCRL(false);
    //    pkt->setCRL_Piece_No(totalPieces);
    //
    //    pkt->setRangeS(totalPieces);
    //    pkt->setRangeE(totalPieces-1);
    //
    //    pkt->setCRLchannel(-1);
    //
    //    pkt->setBitLength(1000);
    //    pkt->setControlInfo( new NetwControlInfo(-1) );
    //
    //    send(pkt, lowerLayerOut);
    //
    //    scheduleAt(simTime() + beacon_Interval, Timer1);
}


// return the index of maximum value
int ApplRSUCRL::Maximum()
{
    //    int Max = 0;
    //    int index = -1;
    //
    //    for(int i=0; i<totalPieces; i++)
    //    {
    //        if(broadcastMask[i] > Max)
    //        {
    //            Max = broadcastMask[i];
    //            index = i;
    //        }
    //    }
    //
    //    // all elements of broadcastMask are 0
    //    if(Max == 0)
    //        return -1;
    //    else
    //    {
    //        broadcastMask[index] = 0;
    //        return index;
    //    }
}


int ApplRSUCRL::IsExist(int index)
{
    //    bool found = false;
    //    unsigned int j = 0;
    //
    //    for(j=0; j<PiecesCRLfromCA.size(); j++)
    //    {
    //        if(PiecesCRLfromCA[j]->getSeqNo() == index)
    //        {
    //            found = true;
    //            break;
    //        }
    //    }
    //
    //    if(found)
    //        return j;
    //    else
    //        return -1;
}

}

