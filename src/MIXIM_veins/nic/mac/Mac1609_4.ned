//
// Copyright (C) 2011 David Eckhoff <eckhoff@cs.fau.de>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// This program is free software; you can redistribute it and/or modify
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

package VENTOS.src.MIXIM_veins.nic.mac;

import VENTOS.src.baseAppl.BaseLayer;


// Base module for every mac layer module
moduleinterface IWirelessMac
{
    parameters:
        @class(BaseMacLayer);
        
        double headerLength @unit(bit);  // length of the MAC packet header (in bits)
        string address;                  // MAC address as hex string (12 hex digits), or "auto". "auto" values will be replaced by a generated MAC address in init stage 1.
        
    gates:
        input upperLayerIn;     // from upper layer
        output upperLayerOut;   // to uppe layer
        input upperControlIn;   // control from upper layer
        output upperControlOut; // control to upper layer
        input lowerLayerIn;     // from lower layer
        output lowerLayerOut;   // to lower layer
        input lowerControlIn;   // control from lower layer
        output lowerControlOut; // control to lower layer
}


// Base module for every mac layer module
simple BaseMacLayer extends BaseLayer like IWirelessMac
{
    parameters:
        @class(BaseMacLayer);
        bool   coreDebug = default(false);   // debug switch
        double headerLength @unit(bit);      // length of the MAC packet header (in bits)
        string address = default("auto");    // MAC address as hex string (12 hex digits), or "auto". "auto" values will be replaced by a generated MAC address in init stage 1.
}


//
// Manages timeslots for CCH and SCH listening and sending.
//
// @author Christopher Saloman
// @author David Eckhoff : rewrote complete model
//
// @see BaseWaveApplLayer
// @see Nic80211p
// @see PhyLayer80211p
//
simple Mac1609_4 extends BaseMacLayer
{
    parameters:
        @class(Veins::Mac1609_4);
        // signal informing interested application about channel busy state
        @signal[sigChannelBusy](type=bool);
        // signal informing interested application about a collision
        @signal[sigCollision](type=bool);
        
        // cycle between an SCH and the CCH (or stay on CCH all the time)
        bool useServiceChannel = default(false);
        
        // the service channel this maclayer listens on when started
        int serviceChannel = default(1);
        
        // debug switch
        bool debug = default(false);
        
        // length of MAC header, header is already added at 1609_4
        headerLength @unit(bit) = default(0 bit);
        
        // bit rate 
        double bitrate @unit(bps) = default(6 Mbps);
        
        // maximum artificial asynchronization between cars to avoid synchronization effects
        double syncOffset @unit(s) = default(0.0003s);
        
        // tx power [mW]
        double txPower = default(20mW) @unit(mW);
        
        // the maximum queue size of an EDCA queue in the MAC. 0 for unlimited. Queue strategy is "drop if full"
        int queueSize = default(0);
        
        bool record_stat = default(false);
}
