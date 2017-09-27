/****************************************************************************/
/// @file    debugStream.h
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author  second author name
/// @date    March 2017
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

// best tutorial on streambuf ever!
// http://www.mr-edd.co.uk/blog/beginners_guide_streambuf

#ifndef DEBUGSTREAMLOG_H
#define DEBUGSTREAMLOG_H

#include <cassert>
#include <streambuf>
#include <vector>
#undef ev
#include "boost/filesystem.hpp"
#include "traci/TraCICommands.h"

namespace VENTOS {

class debugStreamLog : public std::streambuf
{
private:
    bool save2file;
    std::vector<char> buffer_;
    VENTOS::TraCI_Commands *TraCI;
    std::string buffer_file;

public:

    explicit debugStreamLog(bool save2file, std::size_t buff_sz = 512) : save2file(save2file), buffer_(buff_sz + 1)
    {
        char *base = &buffer_.front();
        setp(base, base + buffer_.size() - 1); // -1 to make overflow() easier

        // get a pointer to the TraCI module
        TraCI = TraCI_Commands::getTraCI();
    }

    debugStreamLog(const debugStreamLog &);
    debugStreamLog &operator= (const debugStreamLog &);

    void save_log_toFile()
    {
        if(buffer_file == "")
            return;

        int currentRun = omnetpp::getEnvir()->getConfigEx()->getActiveRunNumber();

        std::ostringstream fileName;
        fileName << boost::format("%03d_dataLog.txt") % currentRun;

        boost::filesystem::path filePath ("results");
        filePath /= fileName.str();

        FILE *filePtr = fopen (filePath.c_str(), "w");
        if (!filePtr)
            throw omnetpp::cRuntimeError("Cannot create file '%s'", filePath.c_str());

        // write simulation parameters at the beginning of the file
        {
            // get the current config name
            std::string configName = omnetpp::getEnvir()->getConfigEx()->getVariable("configname");

            std::string iniFile = omnetpp::getEnvir()->getConfigEx()->getVariable("inifile");

            // PID of the simulation process
            std::string processid = omnetpp::getEnvir()->getConfigEx()->getVariable("processid");

            // globally unique identifier for the run, produced by
            // concatenating the configuration name, run number, date/time, etc.
            std::string runID = omnetpp::getEnvir()->getConfigEx()->getVariable("runid");

            // get number of total runs in this config
            int totalRun = omnetpp::getEnvir()->getConfigEx()->getNumRunsInConfig(configName.c_str());

            // get the current run number
            int currentRun = omnetpp::getEnvir()->getConfigEx()->getActiveRunNumber();

            // get configuration name
            std::vector<std::string> iterVar = omnetpp::getEnvir()->getConfigEx()->getConfigChain(configName.c_str());

            // write to file
            fprintf (filePtr, "configName      %s\n", configName.c_str());
            fprintf (filePtr, "iniFile         %s\n", iniFile.c_str());
            fprintf (filePtr, "processID       %s\n", processid.c_str());
            fprintf (filePtr, "runID           %s\n", runID.c_str());
            fprintf (filePtr, "totalRun        %d\n", totalRun);
            fprintf (filePtr, "currentRun      %d\n", currentRun);
            fprintf (filePtr, "currentConfig   %s\n", iterVar[0].c_str());
            fprintf (filePtr, "sim timeStep    %u ms\n", TraCI->simulationGetDelta());
            fprintf (filePtr, "startDateTime   %s\n", TraCI->simulationGetStartTime_str().c_str());
            fprintf (filePtr, "endDateTime     %s\n", TraCI->simulationGetEndTime_str().c_str());
            fprintf (filePtr, "duration        %s\n\n\n", TraCI->simulationGetDuration_str().c_str());
        }

        fprintf (filePtr, "%s", buffer_file.c_str());
        fclose(filePtr);
    }

protected:

    // overflow is called whenever pptr() == epptr()
    virtual int_type overflow(int_type ch)
    {
        if (ch != traits_type::eof())
        {
            // making sure 'pptr' have not passed 'epptr'
            assert(std::less_equal<char *>()(pptr(), epptr()));

            *pptr() = ch;
            pbump(1);  // advancing the write position

            std::ptrdiff_t n = pptr() - pbase();
            pbump(-n);

            // inserting the first n characters pointed by pbase() into the sink_
            std::ostringstream sink_;
            sink_.write(pbase(), n);

            printOutput(sink_);

            return ch;
        }

        return traits_type::eof();
    }

    // write the current buffered data to the target, even when the buffer isn't full.
    // This could happen when the std::flush manipulator is used on the stream
    virtual int sync()
    {
        std::ptrdiff_t n = pptr() - pbase();
        pbump(-n);

        // inserting the first n characters pointed by pbase() into the sink_
        std::ostringstream sink_;
        sink_.write(pbase(), n);

        printOutput(sink_);

        return 0;
    }

private:

    void printOutput(std::ostringstream & sink_)
    {
        std::cout << sink_.str() << std::flush;

        if(save2file)
            buffer_file += sink_.str();
    }

};

}

#endif
