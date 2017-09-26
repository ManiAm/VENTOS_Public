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

#include "boost/format.hpp"
#include <mutex>

#include "omnetpp.h"
#include "baseAppl/03_BaseApplLayer.h"
#include "logging/debugStream.h"

namespace VENTOS {

#define   WARNING_LOG_VAL   0b0001
#define   ERROR_LOG_VAL     0b0010
#define   INFO_LOG_VAL      0b0100
#define   DEBUG_LOG_VAL     0b1000

#define   ALL_LOG_VAL       0b1111
#define   NO_LOG_VAL        0b0000

class vlog : public BaseApplLayer
{
public:
    static std::mutex lock_log; // global lock

protected:
    uint8_t systemLogLevel = 0;

private:
    typedef BaseApplLayer super;

    // NED variables
    bool printLogLevel = false;
    bool printTimeStep = false;
    bool printFileName = false;
    bool printLineNumber = false;

    // NED variables
    bool logRecordCMD = false;
    bool saveLog2File = false;

    debugStreamLog *buff = NULL;
    std::ostream *out = NULL;
    static vlog *objPtr;

    // internal use
    uint8_t logLevel = 0;
    std::string lastLogPrefix = "";
    std::string logPrefix = "";

public:
    virtual ~vlog();
    virtual void initialize(int stage);
    virtual void finish();
    virtual void handleMessage(omnetpp::cMessage *msg);

    // overloading the << operator
    template<typename T>
    vlog& operator << (const T& inv)
    {
        if(logActive(logLevel))
            printWithLogPrefix(inv);

        return *this;
    }

    // overloading the << operator to accept std::endl and std::flush
    vlog& operator << (std::ostream& (*pf) (std::ostream&))
    {
        if(logActive(logLevel))
            *(out) << pf;

        return *this;
    }

    static vlog& WARNING(std::string file, int line);
    static vlog& ERROR(std::string file, int line);
    static vlog& INFO(std::string file, int line);
    static vlog& DEBUG(std::string file, int line);
    static void FLUSH();
    static bool ISLOGACTIVE(uint8_t userLogLevel);

private:
    bool logActive(uint8_t);
    std::string generateLogPrefix(std::string file, int line);

    template<typename T> void printWithLogPrefix (const T& inv)
    {
        if(lastLogPrefix == logPrefix)
        {
            *(out) << inv;
            return;
        }

        std::stringstream ss;
        ss << inv;
        std::string ss_string = ss.str();

        // count number of leading new lines in ss_string
        int newLineCount = 0;
        for(auto &c : ss_string)
        {
            if(c != '\n')
                break;
            else
                newLineCount++;
        }

        // remove leading new lines from ss_string and
        // prepending to logPrefix
        if(newLineCount != 0)
        {
            ss_string.erase(0, newLineCount);
            logPrefix.insert(0, newLineCount, '\n');
        }

        *(out) << logPrefix;
        *(out) << ss_string;

        lastLogPrefix = logPrefix;
    }
};

}

#endif
