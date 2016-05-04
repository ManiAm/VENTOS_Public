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

#define   WARNING_LOG   0b00000001
#define   INFO_LOG      0b00000010
#define   ERROR_LOG     0b00000100
#define   DEBUG_LOG     0b00001000
#define   EVENT_LOG     0b00010000   // event log
#define   ALL_LOG       0b11111111
#define   NO_LOG        0b00000000

#include <omnetpp.h>
#include <cassert>
#include "boost/format.hpp"

#include <QApplication>
#include <QLabel>

#undef emit   // name conflict with emit on omnetpp

namespace VENTOS {

class vlog_streambuf : public std::streambuf
{
private:

    std::ostream &sink_;
    std::vector<char> buffer_;

public:

    explicit vlog_streambuf(std::ostream &sink, std::size_t buff_sz = 512) : sink_(sink), buffer_(buff_sz + 1)
    {
        sink_.clear();
        char *base = &buffer_.front();
        setp(base, base + buffer_.size() - 1); // -1 to make overflow() easier
    }

    vlog_streambuf(const vlog_streambuf &);
    vlog_streambuf &operator= (const vlog_streambuf &);

protected:

    // overflow is called whenever pptr() == epptr()
    virtual int_type overflow (int_type ch)
    {
        if (sink_ && ch != traits_type::eof())
        {
            // making sure 'pptr' have not passed 'epptr'
            assert(std::less_equal<char *>()(pptr(), epptr()));

            *pptr() = ch;
            pbump(1);  // advancing the write position

            std::ptrdiff_t n = pptr() - pbase();
            pbump(-n);

            // inserting the first n characters pointed by pbase()
            // into the std::ostream
            if (sink_.write(pbase(), n))
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

        // inserting the first n characters pointed by pbase()
        // into the std::ostream
        if (sink_.write(pbase(), n))
            return 0;
        else return -1;
    }
};


class vLog
{
public:
    vLog(uint8_t logLevel) {

        // creating an std::ostream object
        vlog_streambuf *buff = new vlog_streambuf(std::cout);
        out = new std::ostream(buff);

        this->systemLogLevel = logLevel;

        categories.clear();
    }

    ~vLog() {
        // making sure to flush the remaining data in buffer
        *out << std::flush;
        delete out;
    }

    void setLog(uint8_t logLevel)
    {
        this->systemLogLevel = logLevel;
    }

    void flush()
    {
        out->flush();
    }

    template<typename T>
    vLog& operator << (const T& inv)
    {
        if(omnetpp::cSimulation::getActiveEnvir()->isGUI() && (systemLogLevel & lastLogLevel) != 0)
            *out << inv;

        return *this;
    }

    // overloading the subscript operator
    vLog& operator() (uint8_t logLevel = INFO_LOG, std::string cat = "")
    {
        // add the category name
        if(cat != "")
        {
            auto it = find (categories.begin(), categories.end(), cat);
            // the category name already exist
            if(it == categories.end())
            {
                // add the new category name
                categories.push_back(cat);

                updateQtWin();
            }
        }

        this->lastLogLevel = logLevel;

        return *this;
    }


private:
    void updateQtWin();

private:
    std::ostream *out;
    std::vector<std::string> categories;
    uint8_t systemLogLevel;
    uint8_t lastLogLevel;
};

}

#endif