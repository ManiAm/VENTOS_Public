/****************************************************************************/
/// @file    TraCIBuffer.cc
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

#include <iomanip>
#include <sstream>

#include "traci/TraCIBuffer.h"
#include "traci/TraCIConstants.h"
#include "mobility/TraCICoord.h"

namespace VENTOS {

TraCIBuffer::TraCIBuffer() : buf() {
    buf_index = 0;
}

TraCIBuffer::TraCIBuffer(std::string buf) : buf(buf) {
    buf_index = 0;
}

bool TraCIBuffer::eof() const {
    return buf_index == buf.length();
}

void TraCIBuffer::set(std::string buf) {
    this->buf = buf;
    buf_index = 0;
}

void TraCIBuffer::clear() {
    set("");
}

std::string TraCIBuffer::str() const {
    return buf;
}

std::string TraCIBuffer::hexStr() const {
    std::stringstream ss;
    for (std::string::const_iterator i = buf.begin() + buf_index; i != buf.end(); ++i) {
        if (i != buf.begin()) ss << " ";
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)(uint8_t)*i;
    }
    return ss.str();
}

template<> void TraCIBuffer::write(std::string inv) {
    uint32_t length = inv.length();
    write<uint32_t> (length);
    for (size_t i = 0; i < length; ++i) write<char> (inv[i]);
}

template<> std::string TraCIBuffer::read() {
    uint32_t length = read<uint32_t> ();
    if (length == 0) return std::string();
    char obuf[length + 1];

    for (size_t i = 0; i < length; ++i) read<char> (obuf[i]);
    obuf[length] = 0;

    return std::string(obuf, length);
}

template<> void TraCIBuffer::write(TraCICoord inv) {
    write<uint8_t>(POSITION_2D);
    write<double>(inv.x);
    write<double>(inv.y);
}

template<> TraCICoord TraCIBuffer::read() {
    uint8_t posType = read<uint8_t>();
    ASSERT(posType == POSITION_2D);

    TraCICoord p;
    p.x = read<double>();
    p.y = read<double>();

    return p;
}

bool isBigEndian() {
    short a = 0x0102;
    unsigned char *p_a = reinterpret_cast<unsigned char*>(&a);
    return (p_a[0] == 0x01);
}

}
