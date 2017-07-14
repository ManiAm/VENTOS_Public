/****************************************************************************/
/// @file    gnuplot.cc
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author  second author name
/// @date    July 2017
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

#include "global/gnuplot.h"

namespace VENTOS {


gnuplot::gnuplot()
{
#ifdef WIN32
    plotterPtr = _popen("pgnuplot -persist", "w");
#else
    plotterPtr = popen("gnuplot", "w");
#endif

    if(plotterPtr == NULL)
        throw omnetpp::cRuntimeError("Could not open pipe for write!");
}


gnuplot::~gnuplot()
{
    if(plotterPtr)
    {
#ifdef WIN32
        _pclose(plotterPtr);
#else
        pclose(plotterPtr);
#endif
    }
}


double gnuplot::getVersion()
{
    FILE* pipversion = popen("gnuplot --version", "r");
    if (!pipversion)
        throw omnetpp::cRuntimeError("cannot open pipe!");

    char lineversion[128];
    memset (lineversion, 0, sizeof(lineversion));
    if (!fgets(lineversion, sizeof(lineversion), pipversion))
        throw omnetpp::cRuntimeError("fgets error!");

    // now parsing lineversion (gnuplot 5.0 patchlevel 1)
    double vers = 0.0;
    sscanf(lineversion, "gnuplot %lf", &vers);

    int majvers = 0;
    int minvers = 0;

    int pos = -1;
    char* restvers = NULL;
    if (sscanf(lineversion, "gnuplot %d.%d %n", &majvers, &minvers, &pos) >= 2)
    {
        assert(pos >= 0);
        restvers = lineversion + pos;
    }

    pclose(pipversion);
    pipversion = NULL;

    return vers;
}


void gnuplot::sendCommand(const char *fmt, ...)
{
    ASSERT(plotterPtr != NULL);

    va_list args;
    va_start(args, fmt);
    fprintf(plotterPtr, fmt, args);
}


void gnuplot::flush()
{
    fflush(plotterPtr);
}

}
