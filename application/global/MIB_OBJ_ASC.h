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

#define sysDescr        "1.3.6.1.2.1.1.1.0"
#define sysObjectID     "1.3.6.1.2.1.1.2.0"
#define sysUpTime       "1.3.6.1.2.1.1.3.0"
#define sysContact      "1.3.6.1.2.1.1.4.0"
#define sysName         "1.3.6.1.2.1.1.5.0"
#define sysLocation     "1.3.6.1.2.1.1.6.0"

/*
OID: 1.3.6.1.4.1.1206.4.2.1
Full path: iso(1).org(3).dod(6).internet(1).private(4).enterprises(1).nema(1206).transportation(4).devices(2).asc(1)
Module: NTCIP8004-A-2004
*/

// phase 1.3.6.1.4.1.1206.4.2.1.1
#define maxPhases                "1.3.6.1.4.1.1206.4.2.1.1.1"
#define phaseTable               "1.3.6.1.4.1.1206.4.2.1.1.2"
#define maxPhaseGroups           "1.3.6.1.4.1.1206.4.2.1.1.3"
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

