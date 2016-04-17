/****************************************************************************/
/// @file    Cobalt.h
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author  second author name
/// @date    August 2015
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

#ifndef COBALT_H_
#define COBALT_H_

#include <BaseApplLayer.h>
#include "TraCICommands.h"
#include "SNMP.h"

namespace VENTOS {

class Cobalt : public BaseApplLayer
{
public:
    virtual ~Cobalt();
    virtual void initialize(int stage);
    virtual void finish();
    virtual void handleMessage(cMessage *msg);
    virtual void receiveSignal(cComponent *, simsignal_t, long);

private:
    void init_cobalt();

    void initialize_withTraCI();
    void executeEachTimestep();

private:
    typedef BaseApplLayer super;

    // NED variables
    TraCI_Commands *TraCI;  // pointer to the TraCI module
    simsignal_t Signal_executeEachTS;
    simsignal_t Signal_initialize_withTraCI;
    bool on;

    SNMP *cobaltSNMP = NULL;
};

}

#endif
