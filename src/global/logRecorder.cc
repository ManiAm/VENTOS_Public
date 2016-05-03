/****************************************************************************/
/// @file    logRecorder.cc
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author  second author name
/// @date    May 2016
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

#include <logRecorder.h>

namespace VENTOS {

Define_Module(VENTOS::logRecorder);

logRecorder::~logRecorder()
{

}


void logRecorder::initialize(int stage)
{
    if(stage == 0)
    {
        // get a pointer to the TraCI module
        cModule *module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("TraCI");
        TraCI = static_cast<TraCI_Commands *>(module);
        ASSERT(TraCI);

        Signal_initialize_withTraCI = registerSignal("initialize_withTraCI");
        omnetpp::getSimulation()->getSystemModule()->subscribe("initialize_withTraCI", this);

        Signal_executeEachTS = registerSignal("executeEachTS");
        omnetpp::getSimulation()->getSystemModule()->subscribe("executeEachTS", this);




        char *argv[] = {"program name", "arg1", "arg2", NULL};
        int argc = sizeof(argv) / sizeof(char*) - 1;

        QApplication a(argc, argv);
        QLabel label;
        label.setText("Hello World");
        label.setWindowModality(Qt::WindowModal);
        label.show();
        a.exec();

    }
}


void logRecorder::finish()
{


}


void logRecorder::handleMessage(omnetpp::cMessage *msg)
{

}


void logRecorder::receiveSignal(omnetpp::cComponent *source, omnetpp::simsignal_t signalID, long i, cObject* details)
{
    Enter_Method_Silent();

    if(signalID == Signal_initialize_withTraCI)
    {
        initialize_withTraCI();
    }
    else if(signalID == Signal_executeEachTS)
    {
        executeEachTimeStep();
    }
}


void logRecorder::initialize_withTraCI()
{

}


void logRecorder::executeEachTimeStep()
{

}

}
