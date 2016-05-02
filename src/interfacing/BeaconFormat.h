/****************************************************************************/
/// @file    BeaconFormat.h
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author  second author name
/// @date    Feb 2016
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

#ifndef BEACONFORMAT
#define BEACONFORMAT

enum BeaconTypeEnum
{
    iBeacon_Type = 0,   // Apple
    AltBeacon_Type,     // Radius Networks
    EddystoneUID_type,  // Google
    EddystoneURL_type,  // Google
    SamsungType,
};


// iBeacon technology creates a small area of detection where customized notifications can
// be sent to iBeacon-enabled apps on iPhone, iPad, iPod touch
// or Android devices that support Bluetooth 4.0 technology.
class iBeacon
{
public:
    // iBeacon Prefix (always fixed) = 02 01 1A 1A FF 4C 00 02 15
    std::string advFlags;
    std::string advHeader;
    std::string companyId;
    std::string iBeaconType;
    std::string iBeaconLenght;

    // iBeacon user data
    std::string UUID;
    std::string majorNumber;
    std::string minorNumber;
    std::string TXpower;

    // iBeacon total size in bytes in "hex notation"
    std::string size = "1e";
    std::string payload = "";

    iBeacon(std::vector<std::string> str)
    {
        if(str.size() != 9)
            throw omnetpp::cRuntimeError("Number of arguments in iBeacon should be 9");

        std::vector<std::string> tokens;

        tokens = omnetpp::cStringTokenizer(str[0].c_str()).asVector();
        if(tokens.size() == 3)
            this->advFlags = str[0];
        else
            throw omnetpp::cRuntimeError("advFlags should be 3 bytes!");

        tokens = omnetpp::cStringTokenizer(str[1].c_str()).asVector();
        if(tokens.size() == 2)
            this->advHeader = str[1];
        else
            throw omnetpp::cRuntimeError("advHeader should be 2 bytes!");

        tokens = omnetpp::cStringTokenizer(str[2].c_str()).asVector();
        if(tokens.size() == 2)
            this->companyId = str[2];
        else
            throw omnetpp::cRuntimeError("companyId should be 2 bytes!");

        tokens = omnetpp::cStringTokenizer(str[3].c_str()).asVector();
        if(tokens.size() == 1)
            this->iBeaconType = str[3];
        else
            throw omnetpp::cRuntimeError("iBeaconType should be 1 byte!");

        tokens = omnetpp::cStringTokenizer(str[4].c_str()).asVector();
        if(tokens.size() == 1)
            this->iBeaconLenght = str[4];
        else
            throw omnetpp::cRuntimeError("iBeaconLenght should be 1 byte!");

        tokens = omnetpp::cStringTokenizer(str[5].c_str()).asVector();
        if(tokens.size() == 16)
            this->UUID = str[5];
        else
            throw omnetpp::cRuntimeError("UUID should be 16 bytes!");

        tokens = omnetpp::cStringTokenizer(str[6].c_str()).asVector();
        if(tokens.size() == 2)
            this->majorNumber = str[6];
        else
            throw omnetpp::cRuntimeError("majorNumber should be 2 bytes!");

        tokens = omnetpp::cStringTokenizer(str[7].c_str()).asVector();
        if(tokens.size() == 2)
            this->minorNumber = str[7];
        else
            throw omnetpp::cRuntimeError("minorNumber should be 2 bytes!");

        tokens = omnetpp::cStringTokenizer(str[8].c_str()).asVector();
        if(tokens.size() == 1)
            this->TXpower = str[8];
        else
            throw omnetpp::cRuntimeError("TXpower should be 1 byte!");

        for(auto i : str)
            this->payload += (i + " ");
    }
};


// Format: https://github.com/AltBeacon/spec
class AltBeacon
{
public:
    std::string adLength;
    std::string adType;

    std::string MFGID;
    std::string beaconCode;
    std::string beaconID;
    std::string refRSSI;
    std::string MFGRSVD;   // holds specific manufacturer information

    // AltBeacon total size in bytes in "hex notation"
    std::string size = "1c";
    std::string payload = "";

    AltBeacon(std::vector<std::string> str)
    {
        if(str.size() != 7)
            throw omnetpp::cRuntimeError("Number of arguments in AltBeacon should be 7");

        std::vector<std::string> tokens;

        tokens = omnetpp::cStringTokenizer(str[0].c_str()).asVector();
        if(tokens.size() == 1)
            this->adLength = str[0];
        else
            throw omnetpp::cRuntimeError("adLength should be 1 byte!");

        tokens = omnetpp::cStringTokenizer(str[1].c_str()).asVector();
        if(tokens.size() == 1)
            this->adType = str[1];
        else
            throw omnetpp::cRuntimeError("adType should be 1 byte!");

        tokens = omnetpp::cStringTokenizer(str[2].c_str()).asVector();
        if(tokens.size() == 2)
            this->MFGID = str[2];
        else
            throw omnetpp::cRuntimeError("MFGID should be 2 bytes!");

        tokens = omnetpp::cStringTokenizer(str[3].c_str()).asVector();
        if(tokens.size() == 2)
            this->beaconCode = str[3];
        else
            throw omnetpp::cRuntimeError("beaconCode should be 2 bytes!");

        tokens = omnetpp::cStringTokenizer(str[4].c_str()).asVector();
        if(tokens.size() == 20)
            this->beaconID = str[4];
        else
            throw omnetpp::cRuntimeError("beaconID should be 20 bytes!");

        tokens = omnetpp::cStringTokenizer(str[5].c_str()).asVector();
        if(tokens.size() == 1)
            this->refRSSI = str[5];
        else
            throw omnetpp::cRuntimeError("refRSSI should be 1 byte!");

        tokens = omnetpp::cStringTokenizer(str[6].c_str()).asVector();
        if(tokens.size() == 1)
            this->MFGRSVD = str[6];
        else
            throw omnetpp::cRuntimeError("MFGRSVD should be 1 byte!");

        for(auto i : str)
            this->payload += (i + " ");
    }
};


// Eddystone, an open Bluetooth® Smart beacon format from Google.
// The Eddystone specification includes a number of broadcast frame types, including Eddystone-URL, the backbone of the Physical Web.

// check here:
// https://community.estimote.com/hc/en-us/articles/200868188-How-to-modify-UUID-Major-and-Minor-values-

// todo
class EddystoneUID
{

};


// todo
class EddystoneURL
{

};


// others: google for ibeacon hardware
// Estimote, Gelo (pronounced JEE-low), Swirl, Beaconic and Datzing.
// https://kstechnologies.com/shop/particle/

#endif
