
cplusplus {{
#include <Coord.h>
}}

class noncobject Coord;

packet WaveShortMessage 
{
	//Version of the Wave Short Message
	int wsmVersion = 0;
	//Determine which security mechanism was used
	int securityType = 0;
	//Channel Number on which this packet was sent
	int channelNumber;
	//Data rate with which this packet was sent
	int dataRate = 1;
	//Power Level with which this packet was sent
	int priority = 3;
	//Unique number to identify the service
	int psid = 0;
	//Provider Service Context
	string psc = "Service with some Data";
	//Length of Wave Short Message
	int wsmLength;
	//Data of Wave Short Message
	string wsmData = "Some Data";

	string sender;
	string recipient;
	Coord pos;
	double speed;
	double accel;
	double maxDecel;
	string lane;
	int platoonID;
	int platoonDepth;
	
	// int serial = 0;
    // simtime_t timestamp = 0;
}
