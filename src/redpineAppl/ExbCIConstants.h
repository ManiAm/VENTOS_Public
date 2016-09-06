/****************************************************************************/
/// @file    ExbCIConstants.h
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author  second author name
/// @date    Jun 2016
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

// the received message is always an array of uint8_t.
// message type determines how the message should be interpreted.
// recvMsgType enum defines the message types (all message types are defined in onebox_util.h)
enum recvMsgType
{
    TYPE_WSMP_SEND_MESSAGE = 87,
    TYPE_STRING = 1100,
    TYPE_INTEGER = 1101,
};


// signal is a message of type TYPE_INTEGER
enum signalType
{
    Signal_ForwardCollisionWarning = 0xa4,
    Signal_EmergencyBreak = 0xb4,
    Signal_EEBL = 0xc4,
};


enum wsmPayloadType
{
    SAE_RES = 0,
    SAE_ACM,
    SAE_BSM,
    SAE_TEST,
    SAE_CSR,
    SAE_EVA,
    SAE_ICA,
    SAE_MAP,
    SAE_NMEA,
    SAE_PDM,
    SAE_PVD,
    SAE_RSA,
    SAE_RTCM,
    SAE_SPAT,
    SAE_SRM,
    SAE_SSM,
    SAE_TIM,

    GENERIC,  // payload is a generic array of bytes and
    // interpretation is up to the application
};
