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

package VENTOS.src.MIXIM_veins.nic.phy;


// Wireless physical layer module interface.
moduleinterface IWirelessPhy
{        
    gates:
        input upperLayerIn; 	// from the MAC layer
        output upperLayerOut; 	// to the MAC layer
        
        input upperControlIn; 	// control from the MAC layer        
        output upperControlOut; 	// control to the MAC layer
        
        input radioIn; // for sendDirect from other physical layers
}


// Adaptation of the PhyLayer class for 802.11p.
//
// @see BaseWaveApplLayer
// @see Nic80211p
// @see Mac1609_4
//
simple PhyLayer80211p like IWirelessPhy
{
    parameters:
        @class(Veins::PhyLayer80211p);
        
        bool coreDebug   = default(false); //debug switch for core framework
        
        bool emulationActive = default(false);
        
        bool recordStats = default(false); //enable/disable tracking of statistics (eg. cOutvectors)
        int headerLength = default(46 bit) @unit(bit); //defines the length of the phy header (SIGNAL without PREAMBLE)
        
        bool usePropagationDelay = default(true);		//Should transmission delay be simulated?
        bool useThermalNoise = default(true);			//should thermal noise be considered?
        double thermalNoise = default(-110dBm) @unit(dBm);	//the strength of the thermal noise [dBm]
        
        double sensitivity = default(-89dBm) @unit(dBm);	//The sensitivity of the physical layer [dBm]
        
        //# switch times [s]:
        double timeRXToTX       = default(0) @unit(s); // Elapsed time to switch from receive to send state
        double timeRXToSleep    = default(0) @unit(s); // Elapsed time to switch from receive to sleep state
        
        double timeTXToRX       = default(0) @unit(s); // Elapsed time to switch from send to receive state
        double timeTXToSleep    = default(0) @unit(s); // Elapsed time to switch from send to sleep state
        
        double timeSleepToRX    = default(0) @unit(s); // Elapsed time to switch from sleep to receive state
        double timeSleepToTX    = default(0) @unit(s); // Elapsed time to switch from sleep to send state
        
        int initialRadioState   = default(0);   // State the radio is initially in (0=RX, 1=TX, 2=Sleep) 
        
        double radioMinAtt = default(1.0); //radios gain factor (attenuation) while receiving
        double radioMaxAtt = default(0.0); //radios gain factor (attenuation) while not receiving
        
        int nbRadioChannels = default(1);  // Number of available radio channels. Defaults to single channel radio
        int initialRadioChannel = default(0);  // Initial radio channel
        
        //defines the CCA threshold
        double ccaThreshold @unit(dBm) = default(-65 dBm);
        
        //enables/disables collection of statistics about collision. notice that
        //enabling this feature increases simulation time
        bool collectCollisionStatistics = default(false);
        
        //decides whether aborting the simulation or not if the MAC layer
        //requires phy to transmit a frame while currently receiveing another
        bool allowTxDuringRx = default(false);
        
        bool record_stat = default(false);
        bool record_frameTxRx = default(false);
        
    gates:
        input upperLayerIn; 	// from the MAC layer
        output upperLayerOut; 	// to the MAC layer
        
        input upperControlIn; 	// control from the MAC layer        
        output upperControlOut; // control to the MAC layer
        
        input radioIn; // for sendDirect from other physical layers
}
