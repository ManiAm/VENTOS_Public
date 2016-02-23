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


double hue2rgb(double p, double q, double t)
{
    if(t < 0) t += 1;
    if(t > 1) t -= 1;
    if(t < 1/(double)6) return p + (q - p) * 6 * t;
    if(t < 1/(double)2) return q;
    if(t < 2/(double)3) return p + (q - p) * (2/(double)3 - t) * 6;

    return p;
}


// more info here: http://colorizer.org/
RGB Plotter::hslToRgb(double hue, double saturation, double lightness)
{
    hue /= 360, saturation /= 100, lightness /= 100;  // normalize value between [0,1]
    double red, green, blue;

    if(saturation == 0)
    {
        red = green = blue = lightness; // achromatic
    }
    else
    {
        double q = lightness < 0.5 ? lightness * (1 + saturation) : lightness + saturation - lightness * saturation;
        double p = 2 * lightness - q;
        red = hue2rgb(p, q, hue + 1/(double)3);
        green = hue2rgb(p, q, hue);
        blue = hue2rgb(p, q, hue - 1/(double)3);
    }

    RGB result;
    result.red = red * 255;
    result.green = green * 255;
    result.blue = blue * 255;

    return result;
}


// more info here: http://colorizer.org/
HSL Plotter::rgbToHsl(double red, double green, double blue)
{
    red /= 255, green /= 255, blue /= 255; // normalize value between [0,1]
    double max = std::max(red, std::max(green, blue));
    double min = std::min(red, std::min(green, blue));
    double h, s, l = (max + min) / 2;

    if(max == min)
    {
        h = s = 0; // achromatic
    }
    else
    {
        double d = max - min;
        s = l > 0.5 ? d / (2 - max - min) : d / (max + min);

        if(max == red)
            h = (green - blue) / d + (green < blue ? 6 : 0);
        else if(max == green)
            h = (blue - red) / d + 2;
        else if(max == blue)
            h = (red - green) / d + 4;

        h /= 6;
    }

    HSL result;
    result.hue = h * 360;
    result.saturation = s * 100;
    result.lightness = l * 100;

    return result;
}


HSV Plotter::rgb2hsv(double red, double green, double blue)
{
    double min = std::min(red, std::min(green, blue));
    double max = std::max(red, std::max(green, blue));
    double delta = max - min;

    HSV result;
    result.value = 100 * (max / 255);

    if (delta < 0.00001)
    {
        result.saturation = 0;
        result.hue = 0; // undefined, maybe nan?
        return result;
    }

    // NOTE: if Max is == 0, this divide would cause a crash
    if( max > 0.0 )
    {
        result.saturation = 100 * (delta / max);
    }
    else
    {
        // if max is 0, then r = g = b = 0
        // s = 0, v is undefined
        result.saturation = 0.0;
        result.hue = -1;       // its now undefined
        return result;
    }

    if( red >= max )                           // > is bogus, just keeps compiler happy
        result.hue = ( green - blue ) / delta;        // between yellow & magenta
    else if( green >= max )
        result.hue = 2.0 + ( blue - red ) / delta;  // between cyan & yellow
    else
        result.hue = 4.0 + ( red - green ) / delta;  // between magenta & cyan

    result.hue *= 60.0;                              // degrees

    if( result.hue < 0.0 )
        result.hue += 360.0;

    return result;
}


RGB Plotter::hsv2rgb(double hue, double saturation, double value)
{
    saturation /= 100, value /= 100;  // normalize value between [0,1]
    double hh, p, q, t, ff;
    RGB result;

    // < is bogus, just shuts up warnings
    if(saturation <= 0.0)
    {
        result.red = value;
        result.green = value;
        result.blue = value;

        return result;
    }

    hh = hue;
    if(hh >= 360.0) hh = 0.0;
    hh /= 60.0;
    long i = (long)hh;
    ff = hh - i;
    p = value * (1.0 - saturation);
    q = value * (1.0 - (saturation * ff));
    t = value * (1.0 - (saturation * (1.0 - ff)));

    switch(i)
    {
    case 0:
        result.red = value * 255;
        result.green = t * 255;
        result.blue = p * 255;
        break;
    case 1:
        result.red = q * 255;
        result.green = value * 255;
        result.blue = p * 255;
        break;
    case 2:
        result.red = p * 255;
        result.green = value * 255;
        result.blue = t * 255;
        break;

    case 3:
        result.red = p * 255;
        result.green = q * 255;
        result.blue = value * 255;
        break;
    case 4:
        result.red = t * 255;
        result.green = p * 255;
        result.blue = value * 255;
        break;
    case 5:
    default:
        result.red = value * 255;
        result.green = p * 255;
        result.blue = q * 255;
        break;
    }

    return result;
}


HSV Plotter::getUniqueHSVColor()
{
    if(uniqueColors.empty())
        error("No more colors exist!");

    HSV color = uniqueColors.back();
    uniqueColors.pop_back();
    return color;
}


// change saturation from minSaturation to 100
std::vector<double> Plotter::generateColorShades(unsigned int num_shades)
{
    const double minSaturation = 40.0;

    if(num_shades <= 0)
        error("num_shades is not right!");

    std::vector<double> v;

    if(num_shades == 1)
    {
        v.push_back(100);
        return v;
    }

    double offset = (100-minSaturation) / (double)(num_shades-1);
    double count = minSaturation;
    for(unsigned int i = 1; i <= num_shades; ++i)
    {
        v.push_back(count);
        count = count + offset;
    }

    // reverse the vector
    std::reverse(v.begin(), v.end());

    return v;
}


unsigned long Plotter::createRGB(int r, int g, int b)
{
    return ((r & 0xff) << 16) + ((g & 0xff) << 8) + (b & 0xff);
}

}
