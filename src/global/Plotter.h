/****************************************************************************/
/// @file    Plotter.h
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

#ifndef PLOTTER_H
#define PLOTTER_H

#include <BaseApplLayer.h>
#include "TraCICommands.h"

namespace VENTOS {

class Plotter : public BaseApplLayer
{
  public:
      virtual ~Plotter();
      virtual void finish();
      virtual void initialize(int);
      virtual void handleMessage(omnetpp::cMessage *);

  private:
      void getVersion();

  public:
      FILE *pipeGnuPlot = NULL;  // other modules can use pipe to access gnuplot

      double vers = 0.0;
      int majvers = 0;
      int minvers = 0;

  private:
      TraCI_Commands *TraCI;
      bool active;
};

}

#endif
