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
    iBeaconType = 0,
    AltBeaconType,
};


class iBeacon
{
public:
    // iBeacon Prefix (always fixed) = 02 01 1A 1A FF 4C 00 02 15
    std::string adv_flags;
    std::string adv_header;
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
            throw cRuntimeError("Number of arguments should be 9");

        std::vector<std::string> tokens;

        tokens = cStringTokenizer(str[0].c_str()).asVector();
        if(tokens.size() == 3)
            this->adv_flags = str[0];
        else
            throw cRuntimeError("adv_flags length should be 3 bytes!");

        tokens = cStringTokenizer(str[1].c_str()).asVector();
        if(tokens.size() == 2)
            this->adv_header = str[1];
        else
            throw cRuntimeError("adv_header length should be 2 bytes!");

        tokens = cStringTokenizer(str[2].c_str()).asVector();
        if(tokens.size() == 2)
            this->companyId = str[2];
        else
            throw cRuntimeError("companyId length should be 2 bytes!");

        tokens = cStringTokenizer(str[3].c_str()).asVector();
        if(tokens.size() == 1)
            this->iBeaconType = str[3];
        else
            throw cRuntimeError("iBeaconType length should be 1 byte!");

        tokens = cStringTokenizer(str[4].c_str()).asVector();
        if(tokens.size() == 1)
            this->iBeaconLenght = str[4];
        else
            throw cRuntimeError("iBeaconLenght length should be 1 byte!");

        tokens = cStringTokenizer(str[5].c_str()).asVector();
        if(tokens.size() == 16)
            this->UUID = str[5];
        else
            throw cRuntimeError("UUID length should be 16 bytes!");

        tokens = cStringTokenizer(str[6].c_str()).asVector();
        if(tokens.size() == 2)
            this->majorNumber = str[6];
        else
            throw cRuntimeError("majorNumber length should be 2 bytes!");

        tokens = cStringTokenizer(str[7].c_str()).asVector();
        if(tokens.size() == 2)
            this->minorNumber = str[7];
        else
            throw cRuntimeError("minorNumber length should be 2 bytes!");

        tokens = cStringTokenizer(str[8].c_str()).asVector();
        if(tokens.size() == 1)
            this->TXpower = str[8];
        else
            throw cRuntimeError("TXpower length should be 1 byte!");

        for(auto i : str)
            this->payload += (i + " ");
    }
};


// todo
class AltBeacon
{

};

#endif
