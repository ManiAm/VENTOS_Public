/****************************************************************************/
/// @file    tutorial.h
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author  second author name
/// @date    Sep 2016
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

#ifndef TUTORIAL_H
#define TUTORIAL_H

#include "BaseApplLayer.h"
#include "TraCICommands.h"

// all codes are in the VENTOS namespace
namespace VENTOS {

// tutorial class is instantiated from BaseApplLayer class
class tutorial : public BaseApplLayer
{
public:
    // class destructor
    virtual ~tutorial();
    // OMNET++ invokes this method when tutorial.ned module is created.
    virtual void initialize(int);
    // By default, OMNET++ provides one-stage initialization for this module. This method returns the number of init stages required in a multi-stage initialization. Stages are numbers from 0. For simplicity we use the one-stage init by returning 1.
    virtual int numInitStages() const { return 1; }
    // OMNET++ invokes this method when simulation has terminated successfully.
    virtual void finish();
    // OMNET++ invokes this method with a message as parameter whenever the tutorial.ned module receives a message.
    virtual void handleMessage(omnetpp::cMessage *);
    // This method receives emitted signal that carries a long value. There are multiple overloaded receiveSignal methods.
    virtual void receiveSignal(omnetpp::cComponent *, omnetpp::simsignal_t, long, cObject* details);

private:
    // This method is similar to initialize method but you have access to the TraCI interface to communicate with SUMO. In the initialize method, TraCI is still closed.
    void initialize_withTraCI();
    // This method is called in every simulation time step.
    void executeEachTimestep();

private:
    // You can access the TraCI interface using this pointer
    TraCI_Commands *TraCI;
    // This module subscribes to these signals. The first signal notifies this module that the TraCI interface is up and running. The second signal notifies this module that one simulation time step is executed.
    omnetpp::simsignal_t Signal_initialize_withTraCI;
    omnetpp::simsignal_t Signal_executeEachTS;
};

}

#endif
