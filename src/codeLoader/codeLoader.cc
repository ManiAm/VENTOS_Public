/****************************************************************************/
/// @file    codeLoader.cc
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

#include "codeLoader.h"

namespace VENTOS {

Define_Module(VENTOS::codeLoader);

codeLoader::~codeLoader()
{

}


void codeLoader::initialize(int stage)
{
    super::initialize(stage);

    if(stage ==0)
    {
        // get a pointer to the TraCI module
        cModule *module = simulation.getSystemModule()->getSubmodule("TraCI");
        ASSERT(module);
        TraCI = static_cast<TraCI_Commands *>(module);
        ASSERT(TraCI);

        Signal_initialize_withTraCI = registerSignal("initialize_withTraCI");
        simulation.getSystemModule()->subscribe("initialize_withTraCI", this);

        Signal_executeEachTS = registerSignal("executeEachTS");
        simulation.getSystemModule()->subscribe("executeEachTS", this);

        // construct the path to VENTOS_Start_WAVE
        boost::filesystem::path VENTOS_FullPath = cSimulation::getActiveSimulation()->getEnvir()->getConfig()->getConfigEntry("network").getBaseDirectory();
        script_FullPath = VENTOS_FullPath / "src" / "codeLoader" / "VENTOS_Start_WAVE";

        on = par("on").boolValue();

        if(on)
            init_loader();
    }
}


void codeLoader::finish()
{
    if(IMX_board1)
        delete IMX_board1;
}


void codeLoader::handleMessage(cMessage *msg)
{

}


void codeLoader::receiveSignal(cComponent *source, simsignal_t signalID, long i)
{
    if(!on)
        return;

    Enter_Method_Silent();

    if(signalID == Signal_initialize_withTraCI)
    {
        initialize_withTraCI();
    }
    else if(signalID == Signal_executeEachTS)
    {
        executeEachTimestep();
    }
}


void codeLoader::initialize_withTraCI()
{

}


void codeLoader::executeEachTimestep()
{
    // run this code only once
    static bool wasExecuted = false;
    if (on && !wasExecuted)
    {


        wasExecuted = true;
    }
}


void codeLoader::init_loader()
{
    // make a new SSH session
    IMX_board1 = new SSH("192.168.60.43", 22, "ubuntu", "Redpine");
    ASSERT(IMX_board1);

    // copy the VENTOS_Start_WAVE script to the board
    IMX_board1->copyFile_SFTP(script_FullPath, "/home/dsrc/release");

    // run the script
    //IMX_board1->run_command("sudo /home/dsrc/release/VENTOS_Start_WAVE");

    // start 1609 stack in WAVE mode
    //IMX_board1->run_command("/home/dsrc/release/rsi_1609");

    // add a second parameter to run_command to take newChannel?
}

}

