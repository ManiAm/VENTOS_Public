/****************************************************************************/
/// @file    AddRSU.h
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

#ifndef RSUADD_H_
#define RSUADD_H_

#include "TraCI_Extend.h"
#include <BaseApplLayer.h>

namespace VENTOS {

class RSUEntry
{
  public:
      char name[20];
      double coordX;
      double coordY;

      RSUEntry(const char *str, double x, double y)
      {
          strcpy(this->name, str);
          this->coordX = x;
          this->coordY = y;
      }
};

class TraCI_Extend;

class AddRSU : public BaseModule
{
	public:
		virtual ~AddRSU();
		virtual void initialize(int stage);
        virtual void handleMessage(cMessage *msg);
		virtual void finish();
        virtual void receiveSignal(cComponent *, simsignal_t, long);

	private:
        void Add();
        void Scenario1();
        deque<RSUEntry*> commandReadRSUsCoord(string);
        void commandAddCirclePoly(string, string, const TraCIColor& color, Coord*, double);

	private:
        // NED variables
        cModule *nodePtr;   // pointer to the Node
        TraCI_Extend *TraCI;  // pointer to the TraCI module
        simsignal_t Signal_executeFirstTS;
        bool on;
        int mode;
        deque<RSUEntry*> RSUs;

        boost::filesystem::path VENTOS_FullPath;
        boost::filesystem::path SUMO_Path;
        boost::filesystem::path SUMO_FullPath;
};

}

#endif
