/****************************************************************************/
/// @file    vglog.h
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

#ifndef GLOGRECORDER_H
#define GLOGRECORDER_H

#include "boost/format.hpp"
#include <mutex>

#include "omnetpp.h"
#include "logging/vlog.h"
#include "logging/debugStream.h"

namespace VENTOS {

class vglog : public vlog
{
private:
    typedef vlog super;

    // NED variables
    std::string loggingWindowPath = "";
    std::string loggingWindowTitle = "";
    bool syntaxHighlighting;
    std::string syntaxHighlightingExpression = "";
    bool glogRecordCMD = false;
    bool glogActive = true;

    std::mutex lock_socket;

    std::string delimiter = "<||?>";
    std::map< std::string /*tab name*/, std::vector <std::string>> allCategories;
    static vglog *objPtr;
    pid_t child_pid = -1;
    int *socketPtr = NULL;

    std::string lastTab = "";
    std::string lastPane = "";

    enum logWindowCMD
    {
        CMD_SYNTAX_HIGHLIGHTING,
        CMD_ADD_TAB,
        CMD_ADD_SUB_TEXTVIEW,
        CMD_INSERT_TXT,
        CMD_FLUSH,
    };

public:
    virtual ~vglog();
    virtual void initialize(int stage);
    virtual void finish();
    virtual void handleMessage(omnetpp::cMessage *msg);

    // overloading the << operator
    template<typename T>
    vglog& operator << (const T& inv)
    {
        if(logActive())
        {
            std::ostringstream tmp;
            tmp << inv;
            sendToLogWindow(std::to_string(CMD_INSERT_TXT) + objPtr->delimiter + lastTab + objPtr->delimiter + lastPane + objPtr->delimiter + tmp.str());
        }

        return *this;
    }

    // overloading the << operator to accept std::endl and std::flush
    vglog& operator << (std::ostream& (*pf) (std::ostream&))
    {
        if(logActive())
        {
            if(pf == (std::basic_ostream<char>& (*)(std::basic_ostream<char>&)) &std::endl)
                sendToLogWindow(std::to_string(CMD_INSERT_TXT) + objPtr->delimiter + lastTab + objPtr->delimiter + lastPane + objPtr->delimiter + "\n");
            else if(pf == (std::basic_ostream<char>& (*)(std::basic_ostream<char>&)) &std::flush)
                sendToLogWindow(std::to_string(CMD_FLUSH) + objPtr->delimiter + lastTab + objPtr->delimiter + lastPane);
            else
                throw omnetpp::cRuntimeError("The string manipulator is not supported!");
        }

        return *this;
    }

    static vglog& GLOGF(std::string tab, std::string pane);
    static void GFLUSH(std::string tab, std::string pane);
    static void GFLUSHALL();

private:
    bool logActive();
    vglog& setGLog(std::string cat, std::string subcat);
    void openLogWindow();
    void connect_to_TCP_server();
    void sendToLogWindow(std::string);
};

}

#endif
