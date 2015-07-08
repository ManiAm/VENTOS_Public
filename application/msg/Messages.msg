//****************************************************************************/
/// @file    Messages.msg
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author  second author name
/// @date    August 2013
///
//****************************************************************************/
// VENTOS, Vehicular Network Open Simulator; see http:?
// Copyright (C) 2013-2015
//****************************************************************************/
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

cplusplus {{
#include "modules/messages/WaveShortMessage_m.h"

#include <Coord.h>

#include <deque>
typedef std::deque<std::string> stringQueue;

#include <list>
typedef std::list<std::string> stringList;
}}

class WaveShortMessage;

class noncobject Coord;
class noncobject stringQueue;
class noncobject stringList;

namespace VENTOS;

packet BeaconVehicle extends WaveShortMessage 
{
	string sender;
	string senderType;
	string recipient;
	Coord pos;
	double speed;
	double accel;
	double maxDecel;
	string lane;
	string platoonID;
	int platoonDepth;
};


packet BeaconBicycle extends WaveShortMessage 
{
	string sender;
    string senderType;
	string recipient;
	Coord pos;
	double speed;
	double accel;
	double maxDecel;
	string lane;
	string platoonID;
	int platoonDepth;
};


packet BeaconPedestrian extends WaveShortMessage 
{
	string sender;
    string senderType;
	string recipient;
	Coord pos;
	double speed;
	double accel;
	double maxDecel;
	string lane;
	string platoonID;
	int platoonDepth;
};


packet BeaconRSU extends WaveShortMessage 
{
	string sender;
    string senderType;
	string recipient;
	Coord pos;
};


packet DummyMsg extends WaveShortMessage 
{
    string payload;
};


packet LaneChangeMsg extends WaveShortMessage 
{
    string sender;
    string senderType;
	string recipient;
    stringQueue laneChange;  
};


packet PlatoonMsg extends WaveShortMessage 
{
    string sender;
    string senderType;
	string recipient;
    int type;
    string sendingPlatoonID;
    string receivingPlatoonID;
    double dblValue;
    string strValue;
    stringQueue queueValue;    
};


packet SystemMsg extends WaveShortMessage
{
    string sender;
	string recipient;
    int requestType;	//0 = pathing request
    string edge;
    int target;
};


packet RouterMsg extends WaveShortMessage
{
  	string recipient;
  	stringList info; 
};


