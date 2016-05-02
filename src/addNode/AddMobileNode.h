/****************************************************************************/
/// @file    AddMobileNode.h
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

#ifndef ADDMOBILENODE_H
#define ADDMOBILENODE_H

#include "BaseApplLayer.h"
#include "TraCICommands.h"

namespace VENTOS {

class AddMobileNode : public BaseApplLayer
{
public:
    virtual ~AddMobileNode();
    virtual void initialize(int stage);
    virtual void handleMessage(omnetpp::cMessage *msg);
    virtual void finish();
    virtual void receiveSignal(omnetpp::cComponent *, omnetpp::simsignal_t, long, cObject* details);

private:
    void beginLoading();
    void printLoadedStatistics();

    void Scenario1();
    void Scenario2();
    void Scenario4();
    void Scenario5();
    void Scenario6();
    void Scenario7();
    void Scenario8();
    void Scenario9();
    void Scenario10();
    void Scenario11();
    void Scenario12();

    void addFlow();
    std::string getFullPathToSumoRou(std::string);

private:
    typedef BaseApplLayer super;

    TraCI_Commands *TraCI;
    int mode;
    double terminate;

    // class variables
    omnetpp::simsignal_t Signal_initialize_withTraCI;
    omnetpp::simsignal_t Signal_addFlow;
};

}

#endif
