/****************************************************************************/
/// @file    TLStateRecord.h
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author
/// @date    April 2015
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

#ifndef TLSTATERECORD_H
#define TLSTATERECORD_H

#include <04_MeasureTrafficParams.h>

namespace VENTOS {

class currentStatusTL
{
public:
    int cycle;
    std::string allowedMovements;
    double greenLength;
    double greenStart;
    double yellowStart;
    double redStart;
    double phaseEnd;
    int incommingLanes;
    int totalQueueSize;

    currentStatusTL(int i0, std::string str1, double d0, double d1, double d2, double d3, double d4, int i1, int i2)
    {
        this->cycle = i0;
        this->allowedMovements = str1;
        this->greenLength = d0;
        this->greenStart = d1;
        this->yellowStart = d2;
        this->redStart = d3;
        this->phaseEnd = d4;
        this->incommingLanes = i1;
        this->totalQueueSize = i2;
    }
};


class TLStateRecord : public MeasureTrafficParams
{
  public:
    virtual ~TLStateRecord();
    virtual void initialize(int);
    virtual void finish();
    virtual void handleMessage(omnetpp::cMessage *);

  protected:
    void virtual initialize_withTraCI();
    void virtual executeEachTimeStep();

    void updateTLstate(std::string, std::string, std::string = "", bool = false);

  private:
    void saveTLPhasingData();

  protected:
    // NED variables
    double minGreenTime;
    double maxGreenTime;
    double yellowTime;
    double redTime;
    double maxCycleLength;

    // current phase in each TL
    std::unordered_map<std::string /*TLid*/, int /*phase number*/> phaseTL;
    // current status of each TL in each phase
    std::map<std::pair<std::string /*TLid*/, int /*phase number*/>, currentStatusTL> statusTL;

  private:
    typedef MeasureTrafficParams super;
    bool collectTLPhasingData;
};

}

#endif
