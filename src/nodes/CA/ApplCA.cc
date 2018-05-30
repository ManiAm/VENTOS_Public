/****************************************************************************/
/// @file    ApplCA.cc
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

#include "nodes/CA/ApplCA.h"
#include "nodes/CA/FiniteFieldMath.h"
#include "global/SignalObj.h"

namespace VENTOS {

Define_Module(VENTOS::ApplCA);

ApplCA::~ApplCA()
{

}

void ApplCA::initialize(int stage)
{
    super::initialize(stage);

    if(stage == 0) 
    {
        active = par("active").boolValue();
        if(!active)
            return;

        moduleName = this->getParentModule()->getFullName();

        InitialWait = par("InitialWait").intValue();
        if(InitialWait < 0)
            throw omnetpp::cRuntimeError("value for InitialWait is incorrect !!");

        CRLsize = par("CRLsize").intValue();
        if(CRLsize <= 0)
            throw omnetpp::cRuntimeError("value for CRLsize is incorrect !!");

        EnableShuffle = par("EnableShuffle").boolValue();

        Pseudonym_lifeTime = par("Pseudonym_lifeTime").doubleValue();
        if(Pseudonym_lifeTime <= 0)
            throw omnetpp::cRuntimeError("value for Pseudonym_lifeTime is incorrect !!");

        ErasureCode = par("ErasureCode").boolValue();

        if(!ErasureCode)
        {
            NoSegments = par("NoSegments").intValue();

            if(NoSegments <= 0)
                throw omnetpp::cRuntimeError("Value of NoSegments is incorrect! Check configuration file.");

            totalPieces = NoSegments;
        }
        else
        {
            N = par("N").intValue();
            M = par("M").intValue();

            if(N <= 0 || N >= 256)
                throw omnetpp::cRuntimeError("Value of N is not correct! Check configuration file.");

            if(M <= 0 || M > N)
                throw omnetpp::cRuntimeError("Value of M is not correct! Check configuration file.");

            totalPieces = N;

            // if ErasureCode is enabled, we calculate Matrix_A according to its dimension (N * M).
            // This calculation is done only once and Matrix_A is shared between all nodes.
            CalculateMatrixA();
        }

        // register Magic_Req signal and subscribe it globally.
        // Magic cars emit this signal to request CRL from CA.
        Signal_Magic_Req = registerSignal("Magic_Req");
        omnetpp::getSimulation()->getSystemModule()->subscribe("Magic_Req", this);

        Timer1 = new omnetpp::cMessage("Timer_Initial_Wait_CA");
        scheduleAt(omnetpp::simTime() + InitialWait, Timer1);
    }
}


void ApplCA::finish()
{
    super::finish();

    if(!active)
        return;

    // unsubscribe from Magic_Req
    omnetpp::getSimulation()->getSystemModule()->unsubscribe("Magic_Req", this);

    for(unsigned int i = 0; i < PiecesCRL.size(); i++)
        delete PiecesCRL[i];
}


void ApplCA::handleSelfMsg(omnetpp::cMessage *msg)
{
    if(msg == Timer1)
    {
        createCRL();
        delete msg;
    }
    else
        super::handleSelfMsg(msg);
}


void ApplCA::receiveSignal(omnetpp::cComponent* source, omnetpp::simsignal_t signalID, omnetpp::cObject *obj, cObject* details)
{
    Enter_Method_Silent();

    // we receive a Magic_Req signal from magic vehicle.
    if(signalID == Signal_Magic_Req)
    {
        LOG_INFO << boost::format("\n>>> %s receives a CRL request signal from %s \n") % moduleName % source->getFullName();

        if(PiecesCRL.size() > 0)
        {
            LOG_INFO << "    sending the CRL ... \n";

            CRLPiecesData *data = new CRLPiecesData(source->getFullName(), EnableShuffle ? shuffle(PiecesCRL): PiecesCRL);

            // note that all magic cars in the network will receive this signal, but only
            // the magic car that sent the Magic_Req will store this
            omnetpp::simsignal_t Signal_Magic_Res = registerSignal("Magic_Res");
            this->getParentModule()->emit(Signal_Magic_Res, data);
        }
        else
            LOG_INFO << "    CRL is empty! \n";

        LOG_FLUSH;
    }
    else
        super::receiveSignal(source, signalID, obj, details);
}


// Calculate Matrix A when Erasure Code is enabled. This matrix is N by M.
void ApplCA::CalculateMatrixA()
{
    LOG_INFO << boost::format("\n>>> %1% is calculating Matrix_A --> N is %2% and M is %3% \n") % moduleName % N % M << std::flush;

    Matrix_A.resize(N,M);

    int rnd;

    for(int i = 0; i < N; i++)
    {
        for (int j = 0; j < M ; j++)
        {
            if(j == 0)
                Matrix_A(i,j) = 1;
            else if(j == 1)
            {
                // choose a random number in the range [0,255]
                rnd = intrand(256);
                Matrix_A(i,j) = rnd;
            }
            else
            {
                Matrix_A(i,j) = FiniteFieldMath::gpow(rnd,j);
            }
        }
    }

    LOG_DEBUG << "\n" << Matrix_A << "\n" << std::flush;
}


void ApplCA::createCRL()
{
    LOG_INFO << boost::format("\n>>> %1% is creating a CRL of size %2% \n") % moduleName % CRLsize << std::flush;

    // CRL consists of one or more certificates
    std::vector<Certificate *> CRL;

    // CRLsize is a parameter
    for(int i = 0; i < CRLsize; i++)
    {
        char buf[25];
        sprintf(buf, "Node%d_Certificate", i+1);

        Certificate *cert = new Certificate();

        cert->CerName = buf;
        cert->CAname = "CA0";
        cert->CAid = 123456789;
        cert->NodeName = "V[x]";
        cert->NodeID = i+1;

        CRL.push_back(cert);
    }

    // Step 1: change CRL message into bytes and save it in CRLbytes stream.
    std::ostringstream CRLbytes;
    boost::archive::text_oarchive oa(CRLbytes);
    oa << CRL;

    LOG_INFO << boost::format(">>> %1% converted the CRL into a raw data of %2% bytes \n") % moduleName % CRLbytes.str().size() << std::flush;

    // Step 2: encode CRL
    std::vector<std::string> EncodedCRL = ErasureCode ? erasure(CRLbytes): NOerasure(CRLbytes);

    // Step 3: adding header to each of the entries in EncodedCRL
    PiecesCRL = addHeader(EncodedCRL);

    // now we have all CRL pieces in PiecesCRL vector and we can send them to each RSU
    sendPiecesToRSUs();
}


std::vector<std::string> ApplCA::NOerasure(std::ostringstream &CRLbytes)
{
    LOG_INFO << boost::format(">>> %1% is dividing CRL message into %2% segments (Erasure code is disabled) \n") % moduleName % NoSegments;

    unsigned long len = CRLbytes.str().size();
    int n;   // number of bytes in each pieces

    if(len % NoSegments == 0)
        n = len / NoSegments;
    else
        n = (len / NoSegments) + 1;

    LOG_DEBUG << boost::format("    This means that each segment has maximum of %1% bytes. \n") % n;

    if(LOG_ACTIVE(DEBUG_LOG_VAL) && false)
    {
        for(unsigned long i=0; i<len; i++)
        {
            std::cout << (unsigned int)CRLbytes.str().at(i) << " ";

            if(i != 0 && (i+1)%n == 0)
                LOG_DEBUG << "  -   ";
        }
    }

    // now we save the bytes of each piece to the tmp vector.
    std::vector<std::string> tmp;
    std::ostringstream oss;

    for(unsigned long i=0; i<len; i++)
    {
        oss << CRLbytes.str().at(i);

        if(i != 0 && (i+1)%n == 0)
        {
            tmp.push_back(oss.str());
            oss.str("");
        }
    }

    if(len % NoSegments != 0)
        tmp.push_back(oss.str());

    LOG_INFO << "\n \n" << std::flush;

    return tmp;
}


std::vector<std::string> ApplCA::erasure(std::ostringstream &CRLbytes)
{
    LOG_INFO << boost::format(">>> %1% is applying erasure code on CRL \n") % moduleName;

    LOG_DEBUG << boost::format("    Step 1: M is %1%, so we split CRL bytes into %2%-byte segments. \n") % M % M;

    unsigned long len = CRLbytes.str().size();

    if(false)
    {
        for(unsigned long i=0; i<len; i++)
        {
            printf("%-3d ", (unsigned int)CRLbytes.str().at(i));
            if(i != 0 && (i+1)%M == 0)
                std::cout << "\n";
        }

        std::cout << std::endl << std::endl;
    }

    int L;

    if(len % M == 0)
    {
        std::cout << "    We have " << len << " bytes which is multiple of M (padding is not needed)." << std::endl;
        L = len / M;
        pad = 0;
        std::cout << "    Number of parts (L) = " << L << std::endl;
    }
    else
    {
        std::cout << "    We have " << len << " bytes which is not multiple of M (padding will be added in the last part)." << std::endl;
        L = (len / M) + 1;
        pad = M - (len % M);
        std::cout << "    Number of parts (L) = " << L << std::endl;
        std::cout << "    Number of padding (pad) = " << pad << std::endl;
    }

    std::cout << "    Now we construct Matrix B and put each segment vertically" << std::endl;

    Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> Matrix_B;
    Matrix_B.resize(M,L);
    Matrix_B = Eigen::ArrayXXf::Zero(M,L);

    int row = 0, col = 0;

    for(unsigned long i=0; i<len; i++)
    {
        row = i % M;
        if(i != 0 && i%M == 0) col++;
        Matrix_B(row,col) = (unsigned int)CRLbytes.str().at(i);
    }

    std::cout << "\n    Step 2: We calculate Matrix W = Matrix A * Matrix B:" << std::endl;

    Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> Matrix_W;
    Matrix_W.resize(N,L);
    Matrix_W = Eigen::ArrayXXf::Zero(N,L);

    // Matrix A was calculated before (All nodes use the same Matrix_A)
    Matrix_W = Matrix_A * Matrix_B;

    /* todo: uncomment this :)
    // matrix multiplication in GF(256)
    for (int row = 0; row < N; row++)
    {
        for (int col = 0; col < L; col++)
        {
            // Multiply the row of A by the column of B to get the row, column of product.
            for (int inner = 0; inner < M; inner++)
            {
                uint8_t tmp = gmul( (uint8_t)Matrix_A(row,inner), (uint8_t)Matrix_B(inner,col) );
                Matrix_W(row,col) = gadd ( (uint8_t)Matrix_W(row,col), tmp );
            }
        }
    }
     */

    std::cout << "    Matrix W is " << Matrix_W.rows() << " by " << Matrix_W.cols() << std::endl;
    std::cout << "    Each row of Matrix W is a CRL piece. We have " << N << " pieces in total" << std::endl;
    std::cout << "    CA sends each of the pieces separately to the RSUs." << std::endl;
    std::cout << "    Any M = " << M << " out of N = " << N << " CRL pieces is enough for re-construction of the original CRL." << std::endl;

    std::vector<std::string> tmp;
    std::ostringstream oss;

    for(int i=0; i<N; i++)
    {
        oss.str("");

        for(int j=0; j<L; j++)
            oss << Matrix_W(i,j) << "#";  // # is the delimiter!

        tmp.push_back(oss.str());
    }

    return tmp;
}


std::vector<CRL_Piece *> ApplCA::addHeader(std::vector<std::string> vec)
{
    printf(">>> %s is adding header to each entry \n", moduleName.c_str());

    // generate a random initial Sequence number
    // int ISeqNo = 1 + intrand(1000);
    int ISeqNo = 0;

    // a vector for storing the CRL pieces
    std::vector<CRL_Piece *> vecResult;

    for(unsigned int i=0; i< vec.size(); i++)
    {
        // create the packet for transmitting a certificate
        CRL_Piece *pkt = new CRL_Piece(moduleName.c_str(), TYPE_CRL_PIECE);

        // set the WSM parameters
        pkt->setWsmVersion(1);
        pkt->setSecurityType(1);
        pkt->setChannelNumber(Veins::Channels::CCH);
        pkt->setDataRate(1);
        pkt->setPriority(1);
        pkt->setPsid(0);

        pkt->setCRLversion(1);
        pkt->setTimestamp(0);
        pkt->setSeqNo(ISeqNo++);
        pkt->setCAid(23);

        // when ErasureCode is enabled, we add number of padding in all pieces!
        // you do not know which subset of pieces will be received in a vehicle
        if(ErasureCode)
            pkt->setPad(pad);
        else
            pkt->setPad(-1);

        pkt->setPayload(vec[i].c_str());

        // size of each msg is 2300 Bytes.
        // note that 4 Bytes header will be added in network layer.
        pkt->setBitLength(18400);

        // pkt is ready. we add it to the vector
        vecResult.push_back(pkt);
    }

    return vecResult;
}


// assign CRL pieces to each of the RSUs
void ApplCA::sendPiecesToRSUs()
{
    printf(">>> %s is sending CRL pieces to each RSU \n\n", moduleName.c_str());

    // get a pointer to the first RSU
    omnetpp::cModule *module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("RSU", 0);
    if(module == NULL)
        throw omnetpp::cRuntimeError("No RSU module was found in the network!");

    // how many RSUs are in the network?
    int RSUcount = module->getVectorSize();

    for(int i=0; i<RSUcount; i++)
    {
        if(!EnableShuffle)
        {
            cModule *rmodule = omnetpp::getSimulation()->getSystemModule()->getSubmodule("RSU", i);
            if(rmodule == NULL)
                throw omnetpp::cRuntimeError("RSU %d was found in the network!", i);

            CRLPiecesData *data = new CRLPiecesData(rmodule->getFullName(), PiecesCRL);

            // note that all RSUs will receive this signal
            omnetpp::simsignal_t Signal_CRL_pieces = registerSignal("CRL_pieces");
            this->getParentModule()->emit(Signal_CRL_pieces, data);
        }
        else
        {
            // shuffle PiecesCRL
            std::vector<CRL_Piece *> PiecesCRL_shuffled= shuffle(PiecesCRL);  // passing by value!

            cModule *rmodule = omnetpp::getSimulation()->getSystemModule()->getSubmodule("RSU", i);
            if(rmodule == NULL)
                throw omnetpp::cRuntimeError("RSU %d was not found in the network!", i);

            CRLPiecesData *data = new CRLPiecesData(rmodule->getFullName(), PiecesCRL_shuffled);

            // note that all RSUs will receive this signal
            omnetpp::simsignal_t Signal_CRL_pieces = registerSignal("CRL_pieces");
            this->getParentModule()->emit(Signal_CRL_pieces, data);
        }
    }
}


std::vector<CRL_Piece *> ApplCA::shuffle(std::vector<CRL_Piece *> vec)
{
    int n = 0;

    for (int i = vec.size() - 1; i >= 0; i--)
    {
        n = intrand( vec.size() );

        if(i != n)
            std::swap(vec[i], vec[n]);
    }

    return vec;
}

}
