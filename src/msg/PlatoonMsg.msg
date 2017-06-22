//****************************************************************************/
/// @file    PlatoonMsg.msg
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
#include "WaveShortMessage_m.h"
#include <deque>
typedef std::deque<std::string> stringQueue;

typedef struct value
{    
    // values used by CHANGE_Tg
    double newTG = -1;
    
    // values used by CHANGE_PL
    double newPltDepth = -1;
    std::string newPltLeader;
 
     // values used by MERGE_REQ
    stringQueue myPltMembers;
    bool manualMerge = false;
    
    // values used by SPLIT_DONE
    double caller = -1;
    stringQueue myNewPltMembers;
    int maxSize = -1;
    int optPlnSize = -1;
    bool manualSplit = false;
    
    // values used by LEAVE_REQ and ELECTED_LEADER
    double myPltDepth = -1;
    
    // values used by LEAVE_ACCEPT
    bool lastFollower = false;
} value_t;

}}

class Veins::WaveShortMessage;
class noncobject value_t;

namespace VENTOS;

packet PlatoonMsg extends WaveShortMessage 
{
    string  senderID;
	string  receiverID;
    int     uCommandType;
    string  sendingPlatoonID;
    string  receivingPlatoonID;
    value_t value; // some uCommands need this field to 
                   // send extra infromation to the receiver
};
