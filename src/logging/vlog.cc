/****************************************************************************/
/// @file    vlog.cc
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

#include "logging/vlog.h"


namespace VENTOS {

vlog * vlog::objPtr = NULL;
std::mutex vlog::lock_log;

Define_Module(VENTOS::vlog);

vlog::~vlog()
{
    if(buff)
    {
        delete(buff);
        buff = NULL;
    }

    if(out)
    {
        delete(out);
        out = NULL;
    }
}


void vlog::initialize(int stage)
{
    super::initialize(stage);

    if(stage == 0)
    {
        printLogLevel = par("printLogLevel").boolValue();
        printTimeStep = par("printTimeStep").boolValue();
        printFileName = par("printFileName").boolValue();
        printLineNumber = par("printLineNumber").boolValue();

        logRecordCMD = par("logRecordCMD").boolValue();
        saveLog2File = par("saveLog2File").boolValue();
        systemLogLevel = par("systemLogLevel").intValue();

        // store the pointer to class object
        objPtr = this;

        // making my own stream buffer
        buff = new debugStreamLog(saveLog2File);
        // re-direct ostream to my own stream buffer
        out = new std::ostream(buff);
    }
}


void vlog::finish()
{
    // making sure to flush the remaining data in buffer
    *(out) << std::flush;

    buff->save_log_toFile();
}


void vlog::handleMessage(omnetpp::cMessage *msg)
{

}


vlog& vlog::WARNING(std::string file, int line)
{
    // save log context information
    objPtr->logLevel = WARNING_LOG_VAL;
    objPtr->logPrefix = objPtr->generateLogPrefix(file, line);

    return (*objPtr);
}


vlog& vlog::ERROR(std::string file, int line)
{
    // save log context information
    objPtr->logLevel = ERROR_LOG_VAL;
    objPtr->logPrefix = objPtr->generateLogPrefix(file, line);

    return (*objPtr);
}


vlog& vlog::INFO(std::string file, int line)
{
    // save log context information
    objPtr->logLevel = INFO_LOG_VAL;
    objPtr->logPrefix = objPtr->generateLogPrefix(file, line);

    return (*objPtr);
}


vlog& vlog::DEBUG(std::string file, int line)
{
    // save log context information
    objPtr->logLevel = DEBUG_LOG_VAL;
    objPtr->logPrefix = objPtr->generateLogPrefix(file, line);

    return (*objPtr);
}


void vlog::FLUSH()
{
    *(objPtr->out) << std::flush;
}


bool vlog::ISLOGACTIVE(uint8_t userLogLevel)
{
    return objPtr->logActive(userLogLevel);
}


bool vlog::logActive(uint8_t userLogLevel)
{
    if( logRecordCMD || omnetpp::cSimulation::getActiveEnvir()->isGUI() )
    {
        if( (systemLogLevel & userLogLevel) != 0 )
            return true;
    }

    return false;
}


std::string vlog::generateLogPrefix(std::string file, int line)
{
    if(!printTimeStep && !printLogLevel && !printFileName && !printLineNumber)
        return "";

    std::ostringstream str;

    if(printTimeStep)
        str << boost::format("[%0.8f] ") % omnetpp::simTime().dbl();

    std::string logLevel_str = "";
    if(this->logLevel == WARNING_LOG_VAL)
        logLevel_str = "WARNING";
    else if(this->logLevel == ERROR_LOG_VAL)
        logLevel_str = "ERROR";
    else if(this->logLevel == INFO_LOG_VAL)
        logLevel_str = "INFO";
    else if(this->logLevel == DEBUG_LOG_VAL)
        logLevel_str = "DEBUG";
    else
        throw omnetpp::cRuntimeError("Unknown log level %d", this->logLevel);

    if(printLogLevel)
        str << boost::format("%s ") % logLevel_str;

    if(printFileName)
        str << boost::format("%s ") % file;

    if(printLineNumber)
        str << boost::format("(%d) ") % line;

    str << ": ";
    return str.str();
}

}
