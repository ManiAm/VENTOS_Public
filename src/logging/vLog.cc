/****************************************************************************/
/// @file    vLog.cc
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author  second author name
/// @date    May 2016
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

#include <vLog.h>
#include <algorithm>
#include "vLog_streambuf.h"

namespace VENTOS {

Define_Module(VENTOS::vLog);

vLog::~vLog()
{
    // making sure to flush the remaining data in buffer
    *out << std::flush;
    delete out;
}


void vLog::initialize(int stage)
{
    super::initialize(stage);

    if(stage ==0)
    {
        systemLogLevel = par("systemLogLevel").longValue();
        logRecordCMD = par("logRecordCMD").boolValue();

        // creating an std::ostream object
        vlog_streambuf *buff = new vlog_streambuf(std::cout);
        out = new std::ostream(buff);

        categories.clear();
    }
}


void vLog::finish()
{

}


void vLog::handleMessage(omnetpp::cMessage *msg)
{

}


vLog& vLog::setLog(uint8_t logLevel, std::string category)
{
    // add the category name
    if(category != "")
    {
        auto it = find (categories.begin(), categories.end(), category);
        // the category name already exist
        if(it == categories.end())
        {
            // add the new category name
            categories.push_back(category);

            updateQtWin();
        }
    }

    this->lastLogLevel = logLevel;

    return *this;
}


void vLog::flush()
{
    out->flush();
}


void vLog::updateQtWin()
{
    unsigned int size = categories.size();

    if(size == 1)
    {
        char *argv[] = {"program name", "arg1", "arg2", NULL};
        int argc = sizeof(argv) / sizeof(char*) - 1;

        QApplication a(argc, argv);
        QLabel label;
        label.setText("Hello World");
        label.setWindowModality(Qt::WindowModal);
        label.show();
        a.exec();
    }
    else if(size > 1)
    {

    }
}

}
