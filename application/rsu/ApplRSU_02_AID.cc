/****************************************************************************/
/// @file    ApplRSU_02_AID.cc
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

#include "ApplRSU_02_AID.h"
#include <boost/tokenizer.hpp>

namespace VENTOS {

Eigen::MatrixXi ApplRSUAID::tableCount;
Eigen::MatrixXd ApplRSUAID::tableProb;

Define_Module(VENTOS::ApplRSUAID);

ApplRSUAID::~ApplRSUAID()
{

}


void ApplRSUAID::initialize(int stage)
{
    ApplRSUBase::initialize(stage);

    if (stage==0)
    {
        // todo: change n and m dynamically!
        tableCount = Eigen::MatrixXi::Zero(3, 2000);
        tableProb = Eigen::MatrixXd::Constant(3, 2000, 0.1);

        printIncidentDetection = par("printIncidentDetection").boolValue();
    }
}


void ApplRSUAID::finish()
{
    ApplRSUBase::finish();
}


void ApplRSUAID::handleSelfMsg(cMessage* msg)
{
    ApplRSUBase::handleSelfMsg(msg);
}


void ApplRSUAID::executeEachTimeStep(bool simulationDone)
{
    ApplRSUBase::executeEachTimeStep(simulationDone);

    // only RSU[0] executes this
    if( printIncidentDetection && std::string("RSU[0]") == myFullId )
    {
        incidentDetectionToFile();
    }
}


void ApplRSUAID::incidentDetectionToFile()
{
    boost::filesystem::path filePath;

    if( ev.isGUI() )
    {
        filePath = "results/gui/IncidentTable.txt";
    }
    else
    {
        // get the current run number
        int currentRun = ev.getConfigEx()->getActiveRunNumber();
        std::ostringstream fileName;
        fileName << currentRun << "_IncidentTable.txt";
        filePath = "results/cmd/" + fileName.str();
    }

    std::ofstream filePtr( filePath.string().c_str() );

    if (filePtr.is_open())
    {
        filePtr << tableCount;
    }

    filePtr.close();
}


void ApplRSUAID::onBeaconVehicle(BeaconVehicle* wsm)
{
    // no passing down!
}


void ApplRSUAID::onBeaconBicycle(BeaconBicycle* wsm)
{
    // no passing down!
}


void ApplRSUAID::onBeaconPedestrian(BeaconPedestrian* wsm)
{
    // no passing down!
}


void ApplRSUAID::onBeaconRSU(BeaconRSU* wsm)
{
    // no passing down!
}


void ApplRSUAID::onData(LaneChangeMsg* wsm)
{
    // no passing down!

    std::deque<std::string> input = wsm->getLaneChange();

    for(unsigned int i = 0; i < input.size(); i++)
    {
        // tokenize
        int readCount = 1;
        boost::char_separator<char> sep("#", "", boost::keep_empty_tokens);
        boost::tokenizer< boost::char_separator<char> > tokens(input[i], sep);

        std::string fromLane;
        std::string toLane;
        double fromX;
        double toX;
        double time;

        for(boost::tokenizer< boost::char_separator<char> >::iterator beg=tokens.begin(); beg!=tokens.end(); ++beg)
        {
            if(readCount == 1)
            {
                fromLane = (*beg);
            }
            else if(readCount == 2)
            {
                toLane = (*beg);
            }
            else if(readCount == 3)
            {
                fromX = atof( (*beg).c_str() );
            }
            else if(readCount == 4)
            {
                toX = atof( (*beg).c_str() );
            }
            else if(readCount == 5)
            {
                time = atof( (*beg).c_str() );
            }

            readCount++;
        }

        // todo: change them dynamically
        int index_N_start = floor(fromX / 5);
        int index_N_end = floor(toX / 5);
        int index_M = -1;

        if(fromLane == "")
            fromLane = toLane;

        if(fromLane == "1to2_0")
        {
            index_M = 0;
        }
        else if(fromLane == "1to2_1")
        {
            index_M = 1;
        }
        else if(fromLane == "1to2_2")
        {
            index_M = 2;
        }

        // increase all corresponding indices in tableCount by 1
        for(int j = index_N_start; j <= index_N_end; j++)
        {
            tableCount(index_M, j) = tableCount(index_M, j) + 1;
        }
    }
}

}

