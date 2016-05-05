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

#include "mainWindow.h"
#include <QApplication>
#include <qtextedit.h>
#include <debugStream.h>
#undef emit   // name conflict with emit on omnetpp

namespace VENTOS {

Define_Module(VENTOS::vLog);

vLog::~vLog()
{
    // making sure to flush the remaining data in buffer
    flush();
}


void vLog::initialize(int stage)
{
    super::initialize(stage);

    if(stage ==0)
    {
        systemLogLevel = par("systemLogLevel").longValue();
        logRecordCMD = par("logRecordCMD").boolValue();

        // default output stream
        vLogStreams["std::cout"] =  &std::cout;

        // todo
        updateQtWin();
    }
}


void vLog::finish()
{

}


void vLog::handleMessage(omnetpp::cMessage *msg)
{

}


vLog& vLog::setLog(uint8_t logLevel, std::string category, std::string subcategory)
{
    if(category == "")
        throw omnetpp::cRuntimeError("category name can't be empty!");

    auto it = vLogStreams.find(category);
    // new category?
    if(it == vLogStreams.end())
    {
        // creating a new std::ostringstream for this new category
        std::ostringstream *oss = new std::ostringstream;
        vLogStreams[category] = oss;
    }

    lastLogLevel = logLevel;
    lastCategory = category;

    return *this;
}


void vLog::flush()
{
    for(auto &i : vLogStreams)
        i.second->flush();
}


void vLog::updateQtWin()
{
    unsigned int size = vLogStreams.size();

    if(size == 1)
    {
        char *argv[] = {"program name", "arg1", "arg2", NULL};
        int argc = sizeof(argv) / sizeof(char*) - 1;

        QApplication QTapplication(argc, argv);

        mainWindow *mw = new mainWindow();

        QTextEdit *myTextEdit = new QTextEdit(mw);
        myTextEdit->setReadOnly(true);
        myTextEdit->adjustSize();

        // redirect std::ostringstream to a QTextEdit
        std::ostringstream oss;
        QDebugStream qout(oss, myTextEdit);

        // show the main window
        mw->show();

        oss << "Send this to the Text Edit! yohoooooo" << std::endl;

        QTapplication.exec();
    }
    else if(size > 1)
    {

    }
}

}
