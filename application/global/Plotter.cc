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
#include <algorithm>

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
        TraCI = static_cast<TraCI_Commands *>(module);
        ASSERT(TraCI);

#ifdef WIN32
        pipeGnuPlot = _popen("pgnuplot -persist", "w");
#else
        pipeGnuPlot = popen("gnuplot", "w");
#endif

        if(pipeGnuPlot == NULL)
            error("Could not open pipe for write!");

        getVersion();

        // interactive gnuplot terminals: x11, wxt, qt (wxt and qt offer nicer output and a wider range of features)
        // persist: keep the windows open even on simulation termination
        // noraise: updating is done in the background
        // link: http://gnuplot.sourceforge.net/docs_4.2/node441.html
        // fprintf(pipeGnuPlot, "set term wxt enhanced 0 font 'Helvetica,' noraise\n");

        // flush the pipe
        fflush(pipeGnuPlot);
    }
}


void Plotter::getVersion()
{
    FILE* pipversion = popen("gnuplot --version", "r");
    if (!pipversion)
        error("can not open pipe!");

    char lineversion[128];
    memset (lineversion, 0, sizeof(lineversion));
    if (!fgets(lineversion, sizeof(lineversion), pipversion))
        error("fgets error!");

    std::cout << std::endl << "GNUPLOT Version: " << lineversion << std::endl;

    // now parsing lineversion (gnuplot 5.0 patchlevel 1)
    sscanf(lineversion, "gnuplot %lf", &vers);

    int pos = -1;
    char* restvers = NULL;
    if (sscanf(lineversion, "gnuplot %d.%d %n", &majvers, &minvers, &pos) >= 2)
    {
        assert(pos>=0);
        restvers = lineversion+pos;
    }

    pclose(pipversion);
    pipversion = NULL;
}


void Plotter::finish()
{
    if(!on)
        return;

#ifdef WIN32
    _pclose(pipeGnuPlot);
#else
    pclose(pipeGnuPlot);
#endif
}


void Plotter::handleMessage(cMessage *msg)
{

}

}
