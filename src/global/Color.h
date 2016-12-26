/****************************************************************************/
/// @file    Color.h
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author  second author name
/// @date    March 13 2016
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
// This program is distributed in the hope that it will be useful}},
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#ifndef COLOR_H
#define COLOR_H

#include "omnetpp.h"

namespace VENTOS {

struct RGB
{
    double red, green, blue;
};

struct HSV
{
    double hue, saturation, value;
};

struct HSL
{
    double hue, saturation, lightness;
};

class Color
{
private:
    // colors in HSV model
    static std::vector<HSV> uniqueColors;
    static const std::map<std::string, RGB> RGBcolorCodes;

public:
    static RGB colorNameToRGB(std::string tkColorName);

    HSL rgbToHsl(double, double, double);
    RGB hslToRgb(double, double, double);

    static HSV rgb2hsv(double, double, double);
    static RGB hsv2rgb(double, double, double);

    static HSV getUniqueHSVColor();
    static std::vector<double> generateColorShades(unsigned int);
    unsigned long createRGB(int r, int g, int b);
};

}

#endif

