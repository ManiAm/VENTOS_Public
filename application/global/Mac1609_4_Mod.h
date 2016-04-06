/****************************************************************************/
/// @file    Mac1609_4_Mod.h
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

#ifndef _MAC1609_4_MOD_H_
#define _MAC1609_4_MOD_H_

#include <Mac1609_4.h>

namespace VENTOS {

class Mac1609_4_Mod : public Veins::Mac1609_4
{
	public:
		~Mac1609_4_Mod() {};
        virtual void finish();
        virtual void initialize(int);

	    virtual void handleSelfMsg(cMessage*);
		virtual void handleLowerMsg(cMessage*);
	    virtual void handleLowerControl(cMessage* msg);
		virtual void handleUpperMsg(cMessage*);
		virtual void handleUpperControl(cMessage* msg);

	private:
        bool reportMAClayerData;
};

}

#endif
