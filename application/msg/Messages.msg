
cplusplus {{
#include "modules/messages/WaveShortMessage_m.h"

#include <Coord.h>

#include <deque>
typedef std::deque<std::string> stringQueue;

#include <list>
typedef std::list<std::string> stringList;
}}

class WaveShortMessage;

class noncobject Coord;
class noncobject stringQueue;
class noncobject stringList;

namespace VENTOS;

packet BeaconVehicle extends WaveShortMessage 
{
	string sender;
	string recipient;
	Coord pos;
	double speed;
	double accel;
	double maxDecel;
	string lane;
	string platoonID;
	int platoonDepth;
};


packet BeaconBicycle extends WaveShortMessage 
{
	string sender;
	string recipient;
	Coord pos;
	double speed;
	double accel;
	double maxDecel;
	string lane;
	string platoonID;
	int platoonDepth;
};


packet BeaconPedestrian extends WaveShortMessage 
{
	string sender;
	string recipient;
	Coord pos;
	double speed;
	double accel;
	double maxDecel;
	string lane;
	string platoonID;
	int platoonDepth;
};


packet BeaconRSU extends WaveShortMessage 
{
	string sender;
	string recipient;
	Coord pos;
};


packet DummyMsg extends WaveShortMessage 
{
    string payload;
};


packet LaneChangeMsg extends WaveShortMessage 
{
    string sender;
	string recipient;
    stringQueue laneChange;  
};


packet PlatoonMsg extends WaveShortMessage 
{
    string sender;
	string recipient;
    int type;
    string sendingPlatoonID;
    string receivingPlatoonID;
    double dblValue;
    string strValue;
    stringQueue queueValue;    
};


packet SystemMsg extends WaveShortMessage
{
    string sender;
	string recipient;
    int requestType;	//0 = pathing request
    string edge;
    int target;
};


packet RouterMsg extends WaveShortMessage
{
  	string recipient;
  	stringList info; 
};


