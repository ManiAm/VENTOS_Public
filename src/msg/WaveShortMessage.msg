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

namespace Veins;

packet WaveShortMessage {
	int wsmVersion = 0;    // Version of the Wave Short Message
	int securityType = 0;  // Determine which security mechanism was used
	int channelNumber;     // Channel Number on which this packet was sent
	int dataRate = 1;      // Data rate with which this packet was sent
	int priority = 3;      // Power Level with which this packet was sent
	int psid = 0;          // Unique number to identify the service
	string psc = "Service with some Data"; // Provider Service Context
	int wsmLength;         // Length of Wave Short Message
	unsigned char data[];  // Data of Wave Short Message
}
