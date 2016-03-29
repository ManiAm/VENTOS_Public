/****************************************************************************/
/// @file    SniffBluetooth.h
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author  second author name
/// @date    Feb 2016
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

#ifndef SNIFFBLUETOOTH
#define SNIFFBLUETOOTH

// BT specifications:
// https://www.bluetooth.com/specifications/adopted-specifications

#include <BaseApplLayer.h>
#include "TraCI_Commands.h"
#include <01_SniffEthernet.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

// service discovery
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>

// un-defining ev!
// why? http://stackoverflow.com/questions/24103469/cant-include-the-boost-filesystem-header
#undef ev
#include "boost/filesystem.hpp"
#define ev  (*cSimulation::getActiveEnvir())

namespace VENTOS {

class BTdevEntry
{
public:
    std::string name;       // BT device name
    std::string timeStamp;  // BT device last detection time

    BTdevEntry(std::string str1, std::string str2)
    {
        this->name = str1;
        this->timeStamp = str2;
    }
};


class SniffBluetooth : public BaseApplLayer
{
public:
    virtual ~SniffBluetooth();
    virtual void finish();
    virtual void initialize(int);
    virtual void handleMessage(cMessage *);

protected:
    void executeFirstTimeStep();
    void executeEachTimestep();

    void getLocalDevs();
    void print_dev_info(struct hci_dev_info *di);
    std::string cmd_name(int);
    std::string cmd_class(int);
    std::string cmd_class(uint8_t dev_class[3]);
    std::string cmd_company(int);

    void cmd_up(int hdev);
    void cmd_down(int hdev);
    bool isDown(int hdev);
    void piscan(int hdev, std::string scan);

    const std::string currentDateTime();

    void cmd_cmd(int dev_id, uint8_t ogf, uint16_t ocf, std::string payload);
    void hex_dump(std::string pref, int width, unsigned char *buf, int len);

private:
    void loadCachedDevices();
    void saveCachedDevices();

    void scan(int dev_id, int len);

    void serviceDiscovery(std::string, uint16_t = 0);

protected:
    // variables
    TraCI_Commands *TraCI;
    SniffEthernet *EtherPtr;

private:
    // NED variables
    bool listLocalDevices;
    bool BT_on;

    boost::filesystem::path cached_BT_devices_filePATH;

    // stores all BT devices (cached and newly detected)
    std::map<std::string /*BT address*/, BTdevEntry> allBTdevices;

    const std::map<unsigned int, std::string> ProtocolUUIDs =
    {
            {1,"SDP"},
            {2,"UDP"},
            {3,"RFCOMM"},
            {4,"TCP"},
            {5,"TCS_BIN"},
            {6,"TCS_AT"},
            {7,"ATT"},
            {8,"OBEX"},
            {9,"IP"},
            {10,"FTP"},
            {12,"HTTP"},
            {14,"WSP"},
            {15,"BNEP"},
            {16,"UPNP"},
            {17,"HIDP"},
            {18,"HCRP_CTRL"},
            {20,"HCRP_DATA"},
            {22,"HCRP_NOTE"},
            {23,"AVCTP"},
            {25,"AVDTP"},
            {27,"CMTP"},
            {29,"UDI"},
            {30,"MCAP_CTRL"},
            {31,"MCAP_DATA"},
            {256,"L2CAP"},
    };

    const std::map<unsigned int, std::string> majors =
    {
            {0,"Misc"},
            {1,"Computer"},
            {2,"Phone"},
            {3,"Net_Access_Point"},
            {4,"Audio/Video"},
            {5,"Peripheral"},
            {6,"Imaging"},
            {7,"Wearable"},
            {8,"Toy"},
            {9,"Health"},
            {63,"Uncategorized"},
    };

    const std::map<unsigned int, std::string> computers =
    {
            {0,"Uncategorized"},
            {1,"Desktop"},
            {2,"Server"},
            {3,"Laptop"},
            {4,"Handheld"},
            {5,"Palm-size"},
            {6,"Wearable"},
            {7,"Tablet"},
    };

    const std::map<unsigned int, std::string> phones =
    {
            {0,"Uncategorized"},
            {1,"Cellular"},
            {2,"Cordless"},
            {3,"Smartphone"},
            {4,"Wired_modem_or_voice_gateway"},
            {5,"Common_ISDN_access"},
    };

    const std::map<unsigned int, std::string> av =
    {
            {0,"Uncategorized"},
            {1,"Wearable_Headset_Device"},
            {2,"Hands-free_Device"},
            {3,"Reserved"},
            {4,"Microphone"},
            {5,"Loudspeaker"},
            {6,"Headphones"},
            {7,"Portable_Audio"},
            {8,"Car_audio"},
            {9,"Set-top_box"},
            {10,"HiFi_Audio_Device"},
            {11,"VCR"},
            {12,"Video_Camera"},
            {13,"Camcorder"},
            {14,"Video_Monitor"},
            {15,"Video_Display_and_Loudspeaker"},
            {16,"Video_Conferencing"},
            {17,"Reserved"},
            {18,"Gaming/Toy"},
    };

    const std::map<unsigned int, std::string> peripheral =
    {
            {0,"Uncategorized"},
            {1,"Joystick"},
            {2,"Gamepad"},
            {3,"Remote_control"},
            {4,"Sensing_device"},
            {5,"Digitizer_tablet"},
            {6,"Card_Reader"},
            {7,"Digital_Pen"},
            {8,"Handheld_scanner"},
            {9,"Handheld_gestural_input"},
    };

    const std::map<unsigned int, std::string> wearable =
    {
            {0,"Uncategorized"},
            {1,"Wristwatch"},
            {2,"Pager"},
            {3,"Jacket"},
            {4,"Helmet"},
            {5,"Glasses"},
    };

    const std::map<unsigned int, std::string> toys =
    {
            {0,"Uncategorized"},
            {1,"Robot"},
            {2,"Vehicle"},
            {3,"Doll/Action_figure"},
            {4,"Controller"},
            {5,"Game"},
    };

    const std::map<unsigned int, std::string> health =
    {
            {0,"Uncategorized"},
            {1,"Blood_Pressure_Monitor"},
            {2,"Thermometer"},
            {3,"Weighing_Scale"},
            {4,"Glucose_Meter"},
            {5,"Pulse_Oximeter"},
            {6,"Heart/Pulse_Rate_Monitor"},
            {7,"Health_Data_Display"},
            {8,"Step_Counter"},
            {9,"Body_Composition_Analyzer"},
            {10,"Peak_Flow_Monitor"},
            {11,"Medication_Monitor"},
            {12,"Knee_Prosthesis"},
            {13,"Ankle_Prosthesis"},
            {14,"Generic_Health_Manager"},
            {15,"Personal_Mobility_Device"},
    };
};

}

#endif
