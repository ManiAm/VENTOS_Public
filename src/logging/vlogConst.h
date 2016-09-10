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

// check if a particular logLevel is active?
#define LOG_ACTIVE(logLevel) \
        std::lock_guard<std::mutex>{VENTOS::vlog::lock_log}, VENTOS::vlog::ISLOGACTIVE(logLevel)

// logs are redirected to the standard output stream (std::cout)

#define LOG_WARNING \
        std::lock_guard<std::mutex>{VENTOS::vlog::lock_log}, VENTOS::vlog::WARNING()

#define LOG_INFO \
        std::lock_guard<std::mutex>{VENTOS::vlog::lock_log}, VENTOS::vlog::INFO()

#define LOG_ERROR \
        std::lock_guard<std::mutex>{VENTOS::vlog::lock_log}, VENTOS::vlog::ERROR()

#define LOG_DEBUG \
        std::lock_guard<std::mutex>{VENTOS::vlog::lock_log}, VENTOS::vlog::DEBUG()

#define LOG_EVENT \
        std::lock_guard<std::mutex>{VENTOS::vlog::lock_log}, VENTOS::vlog::EVENT()

#define LOG_FLUSH \
        std::lock_guard<std::mutex>{VENTOS::vlog::lock_log}, VENTOS::vlog::FLUSH()

// logs are redirected to the 'logWindow'. You can control it using category and subcategory

#define LOG_WARNING_C(category, subcategory) \
        std::lock_guard<std::mutex>{VENTOS::vlog::lock_log}, VENTOS::vlog::WARNING(category, subcategory)

#define LOG_INFO_C(category, subcategory) \
        std::lock_guard<std::mutex>{VENTOS::vlog::lock_log}, VENTOS::vlog::INFO(category, subcategory)

#define LOG_ERROR_C(category, subcategory) \
        std::lock_guard<std::mutex>{VENTOS::vlog::lock_log}, VENTOS::vlog::ERROR(category, subcategory)

#define LOG_DEBUG_C(category, subcategory) \
        std::lock_guard<std::mutex>{VENTOS::vlog::lock_log}, VENTOS::vlog::DEBUG(category, subcategory)

#define LOG_EVENT_C(category, subcategory) \
        std::lock_guard<std::mutex>{VENTOS::vlog::lock_log}, VENTOS::vlog::EVENT(category, subcategory)

#define LOG_FLUSH_C(category, subcategory) \
        std::lock_guard<std::mutex>{VENTOS::vlog::lock_log}, VENTOS::vlog::FLUSH(category, subcategory)
}
