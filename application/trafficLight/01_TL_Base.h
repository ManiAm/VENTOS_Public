/****************************************************************************/
/// @file    TrafficLightBase.h
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

#ifndef TRAFFICLIGHTBASE_H
#define TRAFFICLIGHTBASE_H

#include <BaseApplLayer.h>
#include "TraCICommands.h"
#include "ApplRSU_02_Monitor.h"

// un-defining ev!
// why? http://stackoverflow.com/questions/24103469/cant-include-the-boost-filesystem-header
#undef ev
#include "boost/filesystem.hpp"
#define ev  (*cSimulation::getActiveEnvir())

namespace VENTOS {

class TrafficLightBase : public BaseApplLayer
{
  public:
      virtual ~TrafficLightBase();
      virtual void finish();
      virtual void initialize(int);
      virtual void handleMessage(cMessage *);
      virtual void receiveSignal(cComponent *, simsignal_t, long);

  protected:
      virtual void executeFirstTimeStep();
      virtual void executeEachTimeStep();
      void findRSU(std::string);

  protected:
      TraCI_Commands *TraCI;
      ApplRSUMonitor *RSUptr;
      simsignal_t Signal_executeFirstTS;
      simsignal_t Signal_executeEachTS;

      int TLControlMode;
      int debugLevel;
      double updateInterval;
};

}

#endif
