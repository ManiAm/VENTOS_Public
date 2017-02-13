/****************************************************************************/
/// @file    TraCIBuffer.h
/// @author  Christoph Sommer <mail@christoph-sommer.de>
/// @date    August 2013
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

#ifndef VEINS_MOBILITY_TRACI_TRACIBUFFER_H_
#define VEINS_MOBILITY_TRACI_TRACIBUFFER_H_

#include <cstddef>
#include <string>

#include "omnetpp.h"
#include "mobility/TraCICoord.h"

namespace VENTOS {

bool isBigEndian();

/**
 * Byte-buffer that stores values in TraCI byte-order
 */
class TraCIBuffer
{
private:
    std::string buf;
    size_t buf_index;

public:
    TraCIBuffer();
    TraCIBuffer(std::string buf);

    template<typename T>
    T read()
    {
        T buf_to_return;
        unsigned char *p_buf_to_return = reinterpret_cast<unsigned char*>(&buf_to_return);

        if (isBigEndian())
        {
            for (size_t i=0; i<sizeof(buf_to_return); ++i)
            {
                if (eof())
                    throw omnetpp::cRuntimeError("Attempted to read past end of byte buffer");

                p_buf_to_return[i] = buf[buf_index++];
            }
        }
        else
        {
            for (size_t i=0; i<sizeof(buf_to_return); ++i)
            {
                if (eof())
                    throw omnetpp::cRuntimeError("Attempted to read past end of byte buffer");

                p_buf_to_return[sizeof(buf_to_return)-1-i] = buf[buf_index++];
            }
        }

        return buf_to_return;
    }

    template<typename T>
    void write(T inv)
    {
        unsigned char *p_buf_to_send = reinterpret_cast<unsigned char*>(&inv);

        if (isBigEndian())
        {
            for (size_t i=0; i<sizeof(inv); ++i)
                buf += p_buf_to_send[i];
        }
        else
        {
            for (size_t i=0; i<sizeof(inv); ++i)
                buf += p_buf_to_send[sizeof(inv)-1-i];
        }
    }

    template<typename T>
    T read(T& out)
    {
        out = read<T>();
        return out;
    }

    template<typename T>
    TraCIBuffer& operator >>(T& out)
    {
        out = read<T>();
        return *this;
    }

    template<typename T>
    TraCIBuffer& operator <<(const T& inv)
    {
        write(inv);
        return *this;
    }

    bool eof() const;
    void set(std::string buf);
    void clear();
    std::string str() const;
    std::string hexStr() const;
};

template<> void TraCIBuffer::write(std::string inv);
template<> void TraCIBuffer::write(TraCICoord inv);
template<> std::string TraCIBuffer::read();
template<> TraCICoord TraCIBuffer::read();

}

#endif
