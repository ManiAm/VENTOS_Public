/****************************************************************************/
/// @file    SNMPConnect.h
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

#ifndef SNMPCONNECT_H_
#define SNMPCONNECT_H_

#include <BaseApplLayer.h>
#include "TraCI_Extend.h"
#include "libsnmp.h"
#include "snmp_pp/snmp_pp.h"

namespace VENTOS {

class TraCI_Extend;

class SNMPConnect : public BaseModule
{
	public:
		virtual ~SNMPConnect();
		virtual void initialize(int stage);
        virtual void handleMessage(cMessage *msg);
		virtual void finish();
        virtual void receiveSignal(cComponent *, simsignal_t, long);

	private:
        void SNMPGet(std::string OID);

	private:
        // NED variables
        cModule *nodePtr;   // pointer to the Node
        TraCI_Extend *TraCI;  // pointer to the TraCI module
        simsignal_t Signal_executeFirstTS;
        bool on;
        std::string IPAddress;

        Snmp_pp::Snmp *snmp;
};

}

#endif
