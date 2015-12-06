/****************************************************************************/
/// @file    Plotter.cc
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

#include <Plotter.h>

namespace VENTOS {

Define_Module(VENTOS::Plotter);

Plotter::~Plotter()
{

}


void Plotter::initialize(int stage)
{
    if(stage == 0)
	{
        on = par("on").boolValue();

        if(!on)
            return;

        // get a pointer to the TraCI module
        cModule *module = simulation.getSystemModule()->getSubmodule("TraCI");
        TraCI = static_cast<TraCI_Extend *>(module);

        #ifdef WIN32
            pipe = _popen("pgnuplot -persist", "w");
        #else
            pipe = popen("gnuplot", "w");
        #endif

        if(pipe == NULL)
            error("Could not open pipe for write!");

        // interactive gnuplot terminals: x11, wxt, qt (wxt and qt offer nicer output and a wider range of features)
        // persist: keep the windows open even on simulation termination
        // noraise: updating is done in the background
        // link: http://gnuplot.sourceforge.net/docs_4.2/node441.html
        fprintf(pipe, "set term wxt enhanced 0 font 'Helvetica,14' noraise\n");

        // flush the pipe
        fflush(pipe);
    }
}


void Plotter::finish()
{
    if(!on)
        return;

    #ifdef WIN32
        _pclose(pipe);
    #else
        pclose(pipe);
    #endif
}


void Plotter::handleMessage(cMessage *msg)
{

}

}
