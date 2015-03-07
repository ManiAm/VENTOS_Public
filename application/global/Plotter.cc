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
            error("Could not open pipe!");


        // set the terminal
        // persist: keep the windows open even on simulation termination
        // noraise: updating is done in the background
        fprintf(pipe, "set term wxt enhanced 0 noraise\n");

        fprintf(pipe, "set title 'minimum'\n");
        fprintf(pipe, "set xlabel 'Sim Time'\n");
        fprintf(pipe, "set ylabel 'Vehicle Speed'\n");

        // The "-" is used to specify that the data follows the plot command.
        fprintf(pipe, "plot '-' with lines\n");

        // loop over the data. Data terminated with \n
        for(int i = 1, j=0; i < 100; i = i*2, j++)
            fprintf(pipe, "%d %d\n", j, i);


//        // loop over the data. data terminated with \n
//        for(int i = 1, j=0; i < 1000; i = i*4, j++)
//            fprintf(pipe, "%d %d\n", j, i);

        // termination character
        fprintf(pipe, "%s\n", "e");

        // flush the pipe
        fflush(pipe);

      //  fprintf(pipe, "set multiplot\n");

     //   fprintf(pipe, "plot 2*x\n");

     //   fflush(pipe);
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


void Plotter::speedProfile()
{
    if(!on)
        return;


}

}
