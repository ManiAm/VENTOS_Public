/****************************************************************************/
/// @file    vLog.h
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

#ifndef LOGRECORDER_H
#define LOGRECORDER_H

#include <BaseApplLayer.h>
#include <omnetpp.h>
#include "boost/format.hpp"
#include <mutex>

namespace VENTOS {

enum logWindowCMD
{
    CMD_ADD_TAB,
    CMD_ADD_SUB_TEXTVIEW,
    CMD_INSERT_TXT,
    CMD_FLUSH,
};

#define   WARNING_LOG   0b00000001
#define   INFO_LOG      0b00000010
#define   ERROR_LOG     0b00000100
#define   DEBUG_LOG     0b00001000
#define   EVENT_LOG     0b00010000   // event log
#define   ALL_LOG       0b11111111
#define   NO_LOG        0b00000000

class vlog : public BaseApplLayer
{
public:
    virtual ~vlog();
    virtual void initialize(int stage);
    virtual void finish();
    virtual void handleMessage(omnetpp::cMessage *msg);

    // overloading the << operator
    template<typename T>
    vlog& operator << (const T& inv)
    {
        if( logRecordCMD || omnetpp::cSimulation::getActiveEnvir()->isGUI() )
        {
            if( (systemLogLevel & lastLogLevel) != 0 )
            {
                if(lastCategory == "std::cout")
                    std::cout << inv;
                else
                {
                    std::ostringstream tmp;
                    tmp << inv;
                    sendToLogWindow(std::to_string(CMD_INSERT_TXT) + "||" + lastCategory + "||" + lastSubcategory + "||" + tmp.str());
                }
            }
        }

        return *this;
    }

    static vlog& WARNING(std::string category = "std::cout", std::string subcategory = "default");
    static vlog& INFO(std::string category = "std::cout", std::string subcategory = "default");
    static vlog& ERROR(std::string category = "std::cout", std::string subcategory = "default");
    static vlog& DEBUG(std::string category = "std::cout", std::string subcategory = "default");
    static vlog& EVENT(std::string category = "std::cout", std::string subcategory = "default");

    static void flush();

private:
    vlog& setLog(uint8_t logLevel, std::string cat, std::string subcat);
    void start_TCP_client();
    void sendToLogWindow(std::string);

public:
    static std::mutex lock_log; // global lock

private:
    // NED variables
    uint8_t systemLogLevel = 0;
    bool logRecordCMD = false;

    typedef BaseApplLayer super;

    std::map< std::string, std::vector <std::string> * > allCategories;
    static vlog *objPtr;
    pid_t child_pid = -1;
    int* socketPtr = NULL;

    uint8_t lastLogLevel = INFO_LOG;
    std::string lastCategory = "std::cout";
    std::string lastSubcategory = "default";
};

}

#endif
