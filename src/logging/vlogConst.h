/****************************************************************************/
/// @file    vLogConsts.h
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

namespace VENTOS {

enum logWindowCMD
{
    CMD_ADD_TAB,
    CMD_ADD_SUB_TEXTVIEW,
    CMD_INSERT_TXT,
    CMD_FLUSH,
};

#define   WARNING_LOG_VAL   0b00000001
#define   INFO_LOG_VAL      0b00000010
#define   ERROR_LOG_VAL     0b00000100
#define   DEBUG_LOG_VAL     0b00001000
#define   EVENT_LOG_VAL     0b00010000   // event log
#define   ALL_LOG_VAL       0b11111111
#define   NO_LOG_VAL        0b00000000

// logs are redirected to the standard output stream (std::cout)

#define WARNING_LOG \
        std::lock_guard<std::mutex>{vlog::lock_log}, vlog::WARNING()

#define INFO_LOG \
        std::lock_guard<std::mutex>{vlog::lock_log}, vlog::INFO()

#define ERROR_LOG \
        std::lock_guard<std::mutex>{vlog::lock_log}, vlog::ERROR()

#define DEBUG_LOG \
        std::lock_guard<std::mutex>{vlog::lock_log}, vlog::DEBUG()

#define EVENT_LOG \
        std::lock_guard<std::mutex>{vlog::lock_log}, vlog::EVENT()

#define FLUSH_LOG \
        std::lock_guard<std::mutex>{vlog::lock_log}, vlog::FLUSH()

// logs are redirected to the 'logWindow'. You can control it using category and subcategory

#define WARNING_LOG_C(category, subcategory) \
        std::lock_guard<std::mutex>{vlog::lock_log}, vlog::WARNING(category, subcategory)

#define INFO_LOG_C(category, subcategory) \
        std::lock_guard<std::mutex>{vlog::lock_log}, vlog::INFO(category, subcategory)

#define ERROR_LOG_C(category, subcategory) \
        std::lock_guard<std::mutex>{vlog::lock_log}, vlog::ERROR(category, subcategory)

#define DEBUG_LOG_C(category, subcategory) \
        std::lock_guard<std::mutex>{vlog::lock_log}, vlog::DEBUG(category, subcategory)

#define EVENT_LOG_C(category, subcategory) \
        std::lock_guard<std::mutex>{vlog::lock_log}, vlog::EVENT(category, subcategory)

#define FLUSH_LOG_C(category, subcategory) \
        std::lock_guard<std::mutex>{vlog::lock_log}, vlog::FLUSH(category, subcategory)
}
