/****************************************************************************/
/// @file    logRecorder.h
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author  second author name
/// @date    May 2016
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

#ifndef LOGRECORDER_H
#define LOGRECORDER_H

#include <BaseApplLayer.h>
#include "TraCICommands.h"

#include <QApplication>
#include <QLabel>

namespace VENTOS {

class logRecorder : public BaseApplLayer
{
  public:
      virtual ~logRecorder();
      virtual void finish();
      virtual void initialize(int);
      virtual void handleMessage(omnetpp::cMessage *);
      virtual void receiveSignal(omnetpp::cComponent *, omnetpp::simsignal_t, long, cObject* details);

  protected:
    void virtual initialize_withTraCI();
    void virtual executeEachTimeStep();

  private:
      TraCI_Commands *TraCI;

      omnetpp::simsignal_t Signal_initialize_withTraCI;
      omnetpp::simsignal_t Signal_executeEachTS;
};

}

#endif
