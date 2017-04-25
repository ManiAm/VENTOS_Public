/****************************************************************************/
/// @file    FiniteFieldMath.cc
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

#include "nodes/CA/FiniteFieldMath.h"

namespace VENTOS {

/* Add two numbers in a GF(2^8) finite field */
uint8_t FiniteFieldMath::gadd(uint8_t a, uint8_t b)
{
    return a ^ b;
}


/* Subtract two numbers in a GF(2^8) finite field */
uint8_t FiniteFieldMath::gsub(uint8_t a, uint8_t b)
{
    return a ^ b;
}


/* Multiply two numbers in the GF(2^8) finite field defined
   by the polynomial x^8 + x^4 + x^3 + x^2 + 1, hexadecimal 0x11d.*/
uint8_t FiniteFieldMath::gmul(uint8_t a, uint8_t b)
{
    uint8_t p = 0;
    uint8_t counter;
    uint8_t hi_bit_set;

    for (counter = 0; counter < 8; counter++)
    {
        if (b & 1) p ^= a;
        hi_bit_set = (a & 0x80);
        a <<= 1;
        if (hi_bit_set) a ^= 0x11D;
        b >>= 1;
    }

    return p;
}


uint8_t FiniteFieldMath::gpow(uint8_t a, uint8_t e)
{
    uint8_t tmp = 1;

    for(int i=1; i<=e; i++)
        tmp = gmul(a,tmp);

    return tmp;
}

}

