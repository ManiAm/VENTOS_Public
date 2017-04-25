/****************************************************************************/
/// @file    ApplCA.h
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

#ifndef APPLCA_H_
#define APPLCA_H_

#include <eigen3/Eigen/Dense>

#include "nodes/CA/Certificate.h"
#include "baseAppl/04_BaseWaveApplLayer.h"
#include "msg/CRL_Piece_m.h"

namespace VENTOS {

class ApplCA : public BaseWaveApplLayer
{
public:
    // all entities use the same Matrix_A
    Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> Matrix_A;

private:
    typedef BaseWaveApplLayer super;

    bool active;
    bool ErasureCode;
    double Pseudonym_lifeTime;
    int NoSegments;
    int totalPieces;
    int M;
    int N;

    int InitialWait;
    int CRLsize;
    bool EnableShuffle;
    int pad;   // how many padding are added

    std::string moduleName;
    omnetpp::cMessage *Timer1 = NULL;
    omnetpp::simsignal_t Signal_Magic_Req;
    std::vector<CRL_Piece *> PiecesCRL;

public:
    virtual ~ApplCA();
    virtual void initialize(int);
    virtual void finish();
    virtual void receiveSignal(omnetpp::cComponent *, omnetpp::simsignal_t, omnetpp::cObject *, omnetpp::cObject* details);

protected:
    virtual void handleSelfMsg(omnetpp::cMessage* msg);

private:
    void CalculateMatrixA();
    void createCRL();
    std::vector<std::string> NOerasure(std::ostringstream &);
    std::vector<std::string> erasure(std::ostringstream &);
    std::vector<CRL_Piece *> addHeader(std::vector<std::string>);
    void sendPiecesToRSUs();
    std::vector<CRL_Piece *> shuffle(std::vector<CRL_Piece *>);
};

}

#endif
