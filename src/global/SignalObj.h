/****************************************************************************/
/// @file    SignalObj.h
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

#ifndef SIGNALOBJ_H
#define SIGNALOBJ_H

#include "msg/CRL_Piece_m.h"

namespace VENTOS {

class systemData : public omnetpp::cObject, omnetpp::noncopyable
{
    std::string edge;
    std::string node;
    std::string sender;
    int requestType;
    std::string recipient;
    std::list<std::string> edgeList;

public:
    systemData(std::string e, std::string n, std::string s, int r, std::string re) {
        edge = e;
        node = n;
        sender = s;
        requestType = r;
        recipient = re;
    }

    systemData(std::string e, std::string n, std::string s, int r, std::string re, std::list<std::string> el) {
        edge = e;
        node = n;
        sender = s;
        requestType = r;
        recipient = re;
        edgeList = el;
    }

    std::string getEdge()
    {
        return edge;
    }

    std::string getNode()
    {
        return node;
    }

    const char* getSender()
    {
        return sender.c_str();
    }

    int getRequestType()
    {
        return requestType;
    }

    const char* getRecipient()
    {
        return recipient.c_str();
    }

    std::list<std::string> getInfo() {
        return edgeList;
    }
};


// is used to send CRL pieces to an RSU or vehicle
class CRLPiecesData : public omnetpp::cObject, omnetpp::noncopyable
{
public:
    std::string name;  // Recipient name
    std::vector<CRL_Piece *> data;

    CRLPiecesData(std::string str, std::vector<CRL_Piece *> d)
    {
        this->name = str;
        this->data = d;
    }
};

}

#endif
