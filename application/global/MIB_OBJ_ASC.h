/****************************************************************************/
/// @file    MIB_OBJ_ASC.h
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

#ifndef MIBOBJASC_H
#define MIBOBJASC_H

namespace VENTOS {

/*
To specify an object to an SNMP agent, both the Object ID (which defines the type of object) and
the instance (the specific object of the given type) need to be provided.

For non-tabular or scalar objects, the instance is 0. For example sysDescr is a scalar object under
the system group in RFC1213-MIB and it should be specified as sysDescr.0 in Object ID field of MibBrowser.

For tabular objects, the instance is defined in the MIB as index, and it is a sequence of one or more objects.
For example, tcpConnTable under tcp group of RFC1213-MIB has five columns: tcpConnState, tcpConnLocalAddress,
tcpConnLocalPort, tcpConnRemAddress, tcpConnRemPort

tcpConnState     tcpConnLocalAddress     tcpConnLocalPort     tcpConnRemAddress     tcpConnRemPort
--------------------------------------------------------------------------------------------------
listen(2)         0.0.0.0                 21                   0.0.0.0               0
closeWait(8)      192.168.1.78            1156                 192.168.4.144         80







and may be specified as tcpConnState.179.74.15.126.1192.225.226.126.197.80 provided that there exists a row with
Index 179.74.15.126.1192.225.226.126.197.80 in the querying agent where

179.74.15.126 represents the value of the first index tcpConnLocalAddress (IpAddress),

1192 represents the value of the second index tcpConnLocalPort (INTEGER),

225.226.126.197 represents the value of the third index tcpConnRemAddress (IpAddress)

80 represents the value of the fourth index tcpConnRemPort (INTEGER).

*/





/*
Name: mib-2
OID: 1.3.6.1.2.1
Full path: iso(1).org(3).dod(6).internet(1).mgmt(2).mib-2(1)
Module: RFC1213-MIB
*/

// system 1.3.6.1.2.1.1
#define sysDescr        "1.3.6.1.2.1.1.1.0"
#define sysObjectID     "1.3.6.1.2.1.1.2.0"
#define sysUpTime       "1.3.6.1.2.1.1.3.0"
#define sysContact      "1.3.6.1.2.1.1.4.0"
#define sysName         "1.3.6.1.2.1.1.5.0"
#define sysLocation     "1.3.6.1.2.1.1.6.0"
#define sysServices     "1.3.6.1.2.1.1.7.0"

// snmp 1.3.6.1.2.1.11
#define snmpInPkts          "1.3.6.1.2.1.11.1.0"
#define snmpOutPkts         "1.3.6.1.2.1.11.2.0"
#define snmpInTotalReqVars  "1.3.6.1.2.1.11.13.0"
#define snmpInTotalSetVars  "1.3.6.1.2.1.11.14.0"
#define snmpInGetRequests   "1.3.6.1.2.1.11.15.0"
#define snmpInGetNexts      "1.3.6.1.2.1.11.16.0"
#define snmpInSetRequests   "1.3.6.1.2.1.11.17.0"
#define snmpInGetResponses  "1.3.6.1.2.1.11.18.0"

/*
Name: asc
OID: 1.3.6.1.4.1.1206.4.2.1
Full path: iso(1).org(3).dod(6).internet(1).private(4).enterprises(1).nema(1206).transportation(4).devices(2).asc(1)
Module: NTCIP8004-A-2004
*/

// phase 1.3.6.1.4.1.1206.4.2.1.1
#define maxPhases                   "1.3.6.1.4.1.1206.4.2.1.1.1.0"

#define phaseNumber_C               "1.3.6.1.4.1.1206.4.2.1.1.2.1.1"
#define phaseWalk_C                 "1.3.6.1.4.1.1206.4.2.1.1.2.1.2"
#define phasePedestrianClear_C      "1.3.6.1.4.1.1206.4.2.1.1.2.1.3"
#define phaseMinimumGreen_C         "1.3.6.1.4.1.1206.4.2.1.1.2.1.4"
#define phasePassage_C              "1.3.6.1.4.1.1206.4.2.1.1.2.1.5"
#define phaseMaximum1_C             "1.3.6.1.4.1.1206.4.2.1.1.2.1.6"
#define phaseMaximum2_C             "1.3.6.1.4.1.1206.4.2.1.1.2.1.7"
#define phaseYellowChange_C         "1.3.6.1.4.1.1206.4.2.1.1.2.1.8"
#define phaseRedClear_C             "1.3.6.1.4.1.1206.4.2.1.1.2.1.9"
#define phaseRedRevert_C            "1.3.6.1.4.1.1206.4.2.1.1.2.1.10"
#define phaseAddedInitial_C         "1.3.6.1.4.1.1206.4.2.1.1.2.1.11"
#define phaseMaximumInitial_C       "1.3.6.1.4.1.1206.4.2.1.1.2.1.12"
#define phaseTimeBeforeReduction_C  "1.3.6.1.4.1.1206.4.2.1.1.2.1.13"
#define phaseCarsBeforeReduction_C  "1.3.6.1.4.1.1206.4.2.1.1.2.1.14"
#define phaseTimeToReduce_C         "1.3.6.1.4.1.1206.4.2.1.1.2.1.15"
#define phaseReduceBy_C             "1.3.6.1.4.1.1206.4.2.1.1.2.1.16"
#define phaseMinimumGap_C           "1.3.6.1.4.1.1206.4.2.1.1.2.1.17"
#define phaseDynamicMaxLimit_C      "1.3.6.1.4.1.1206.4.2.1.1.2.1.18"
#define phaseDynamicMaxStep_C       "1.3.6.1.4.1.1206.4.2.1.1.2.1.19"
#define phaseStartup_C              "1.3.6.1.4.1.1206.4.2.1.1.2.1.20"
#define phaseOptions_C              "1.3.6.1.4.1.1206.4.2.1.1.2.1.21"
#define phaseRing_C                 "1.3.6.1.4.1.1206.4.2.1.1.2.1.22"
#define phaseConcurrency_C          "1.3.6.1.4.1.1206.4.2.1.1.2.1.23"

#define maxPhaseGroups           "1.3.6.1.4.1.1206.4.2.1.1.3.0"
#define phaseStatusGroupTable    "1.3.6.1.4.1.1206.4.2.1.1.4"
#define phaseControlGroupTable   "1.3.6.1.4.1.1206.4.2.1.1.5"

// detector 1.3.6.1.4.1.1206.4.2.1.2
#define maxVehicleDetectors              "1.3.6.1.4.1.1206.4.2.1.2.1"
#define vehicleDetectorTable             "1.3.6.1.4.1.1206.4.2.1.2.2"
#define maxVehicleDetectorStatusGroups   "1.3.6.1.4.1.1206.4.2.1.2.3"
#define vehicleDetectorStatusGroupTable  "1.3.6.1.4.1.1206.4.2.1.2.4"
#define volumeOccupancySequence          "1.3.6.1.4.1.1206.4.2.1.2.5.1"
#define volumeOccupancyPeriod            "1.3.6.1.4.1.1206.4.2.1.2.5.2"
#define activeVolumeOccupancyDetectors   "1.3.6.1.4.1.1206.4.2.1.2.5.3"
#define volumeOccupancyTable             "1.3.6.1.4.1.1206.4.2.1.2.5.4"
#define maxPedestrianDetectors           "1.3.6.1.4.1.1206.4.2.1.2.6"
#define pedestrianDetectorTable          "1.3.6.1.4.1.1206.4.2.1.2.7"

// unit 1.3.6.1.4.1.1206.4.2.1.3


}

#endif

