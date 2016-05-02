/****************************************************************************/
/// @file    dev.cc
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author  second author name
/// @date    Apr 2016
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

#include "dev.h"

namespace VENTOS {

Define_Module(VENTOS::dev);

dev::~dev()
{

}


void dev::initialize(int stage)
{
    super::initialize(stage);

    if(stage ==0)
    {
        // get a pointer to the TraCI module
        cModule *module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("TraCI");
        ASSERT(module);
        TraCI = static_cast<TraCI_Commands *>(module);
        ASSERT(TraCI);
    }
}


void dev::finish()
{

}


void dev::handleMessage(omnetpp::cMessage *msg)
{

}


void dev::receiveSignal(omnetpp::cComponent *source, omnetpp::simsignal_t signalID, long i, cObject* details)
{
    Enter_Method_Silent();

}


void dev::substituteParams(std::string &content)
{
    std::ostringstream buffer;
    buffer << "PARAMS=\"";

    buffer << "driver_mode=" << par("DRIVER_MODE").longValue() << " ";
    buffer << "firmware_path=$PWD/firmware/" << " ";
    buffer << "onebox_zone_enabled=0x10001" << " ";
    buffer << "ta_aggr=" << par("TA_AGGR").longValue() << " ";
    buffer << "skip_fw_load=" << par("SKIP_FW_LOAD").longValue() << " ";
    buffer << "fw_load_mode=" << par("FW_LOAD_MODE").longValue() << " ";
    buffer << "sdio_clock=" << par("SDIO_CLOCK_SPEED").longValue() << " ";
    buffer << "enable_antenna_diversity=" << par("RSI_ANTENNA_DIVERSITY").longValue() << " ";
    buffer << "coex_mode=" << par("COEX_MODE").longValue() << " ";
    // buffer << "ps_handshake_mode=" << par("HANDSHAKE_MODE").longValue() << " ";
    buffer << "obm_ant_sel_val=" << par("ANT_SEL_VAL").longValue() << " ";
    buffer << "wlan_rf_power_mode=" << par("WLAN_RF_PWR_MODE").longValue() << " ";
    buffer << "bt_rf_power_mode=" << par("BT_RF_PWR_MODE").longValue() << " ";
    buffer << "zigb_rf_power_mode=" << par("ZIGB_RF_PWR_MODE").longValue() << " ";
    buffer << "country_code=" << par("SET_COUNTRY_CODE").longValue() << " ";

    buffer << "\"";

    const std::string& from = "# [params]";
    size_t start_pos = content.find(from);
    if(start_pos == std::string::npos)
        error("cannot find %s in the string!", from.c_str());
    content.replace(start_pos, from.length(), buffer.str().c_str());
}

}

