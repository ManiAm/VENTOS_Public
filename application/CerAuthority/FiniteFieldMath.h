/****************************************************************************/
/// @file    FiniteFieldMath.h
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author  second author name
/// @date    Apr 2016
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

#ifndef FFMATH_H_
#define FFMATH_H_

#include <stdint.h>

namespace VENTOS {

class FiniteFieldMath
{
  public:
    static uint8_t gadd(uint8_t a, uint8_t b);
    static uint8_t gsub(uint8_t a, uint8_t b);
    static uint8_t gmul(uint8_t a, uint8_t b);
    static uint8_t gpow(uint8_t a, uint8_t e);
};

}

#endif
