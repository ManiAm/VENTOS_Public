/****************************************************************************/
/// @file    GlobalConsts.h
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

#ifndef GLOBALCONSTS_H
#define GLOBALCONSTS_H

namespace VENTOS {

enum CRLdistAlgorithm {
    CRL_OFF,
    CRL_RSU_Only,
    CRL_C2C_Epidemic,
    CRL_MPB,
    CRL_ICE,
    CRL_ICEv2,
    CRL_Broadcast,
    CRL_BBroadcast,
    CRL_C2C_Epidemic_Ideal,
    CRL_NewMPB,

    NUM_CRL_ALG
};

enum LaneCostsMode {
    MODE_NOTHING,
    MODE_RECORD,
    MODE_EWMA
};

enum RouterMessage {
    DIJKSTRA,
    HYPERTREE,
    Q_ROUTING,
    DONE,
    STARTED,
};

enum ParkingMessgae {
    SWAPPING,
    PARKED
};

enum ParkingCostMode{
    HYPERBOLIC,
    CONCAVE,
    LINEAR
};


}

#endif
