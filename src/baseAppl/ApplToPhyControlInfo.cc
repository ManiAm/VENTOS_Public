/****************************************************************************/
/// @file    ApplToPhyControlInfo.cc
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author
/// @date    Feb 207
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

#include <iostream>
#include <sstream>
#include "baseAppl/ApplToPhyControlInfo.h"

namespace VENTOS {

ApplToPhyControlInfo::ApplToPhyControlInfo(const char *name, int kind)
{
    this->mcs = 0;
    this->txPower_mW = -1;
}

ApplToPhyControlInfo::ApplToPhyControlInfo(const ApplToPhyControlInfo& other)
{
    copy(other);
}

ApplToPhyControlInfo::~ApplToPhyControlInfo()
{
}

ApplToPhyControlInfo& ApplToPhyControlInfo::operator=(const ApplToPhyControlInfo& other)
{
    if (this == &other)
        return *this;

    copy(other);
    return *this;
}

void ApplToPhyControlInfo::copy(const ApplToPhyControlInfo& other)
{
    this->mcs = other.mcs;
    this->txPower_mW = other.txPower_mW;
}

int ApplToPhyControlInfo::getMcs() const
{
    return this->mcs;
}

void ApplToPhyControlInfo::setMcs(int mcs)
{
    this->mcs = mcs;
}

double ApplToPhyControlInfo::getTxPower_mW() const
{
    return this->txPower_mW;
}

void ApplToPhyControlInfo::setTxPower_mW(double txPower_mW)
{
    this->txPower_mW = txPower_mW;
}

}
