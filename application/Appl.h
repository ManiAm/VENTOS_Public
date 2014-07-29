#ifndef Appl_H
#define Appl_H

#include<list>
#include <msg/Messages_m.h>

using namespace std;

namespace VENTOS {

class systemData : public cObject, noncopyable
{

    string edge;
    string node;
    string sender;
    int requestType;
    string recipient;
    list<string> edgeList;

public:
    systemData(string e, string n, string s, int r, string re)
    {
        edge = e;
        node = n;
        sender = s;
        requestType = r;
        recipient = re;
    }

    systemData(string e, string n, string s, int r, string re, list<string> el)
    {
        edge = e;
        node = n;
        sender = s;
        requestType = r;
        recipient = re;
        edgeList = el;
    }
    string getEdge()
    {
        return edge;
    }
    string getNode()
    {
        return node;
    }
    const char* getSender()
    {
        return sender.c_str();
    }
    int getRequestType()
    {
        return requestType;
    }
    const char* getRecipient()
    {
        return recipient.c_str();
    }
    list<string> getInfo()
    {
        return edgeList;
    }
};

class data : public cObject, noncopyable
{
  public:
    char name[20];

    data(const char *str)
    {
        strcpy(this->name, str);
    }
};


class MacStat : public cObject, noncopyable
{
  public:
    vector<long> vec;

    MacStat( vector<long> v)
    {
        vec.swap(v);
    }
};


class MacStatEntry
{
public:
  char name[20];
  int nodeID;  // is used to sort the vector (used as a key)
  double time;
  vector<long> MacStatsVec;

  MacStatEntry(const char *str, int id, double t, vector<long> v)
  {
      strcpy(this->name, str);
      this->nodeID = id;
      this->time = t;
      MacStatsVec.swap(v);
  }
};


class NodeEntry
{
  public:
	char name1[20];
    int nodeID;  // is used to sort the vector (used as a key)

    char name2[20];
    int count;
	simtime_t time;

	NodeEntry(const char *str1, const char *str2, int id, int n, simtime_t t)
	{
		strcpy(this->name1, str1);
        this->nodeID = id;

        strcpy(this->name2, str2);
        this->count = n;
		this->time = t;
	}
};


class VehicleData
{
  public:
    int index;
    double time;

    char vehicleName[20];
    char vehicleType[20];

    char lane[20];
    double pos;

    double speed;
    double accel;
    double gap;
    double timeGap;

    VehicleData(int i, double d1,
                 const char *str1, const char *str2,
                 const char *str3, double d2,
                 double d3, double d4, double d5, double d6)
    {
        this->index = i;
        this->time = d1;

        strcpy(this->vehicleName, str1);
        strcpy(this->vehicleType, str2);

        strcpy(this->lane, str3);
        this->pos = d2;

        this->speed = d3;
        this->accel = d4;
        this->gap = d5;
        this->timeGap = d6;
    }
};


class LoopDetector
{
  public:
    char detectorName[20];
    char vehicleName[20];
    double entryTime;
    double leaveTime;
    double entrySpeed;
    double leaveSpeed;

    LoopDetector( const char *str1, const char *str2, double entryT, double leaveT, double entryS, double leaveS )
    {
        strcpy(this->detectorName, str1);
        strcpy(this->vehicleName, str2);

        this->entryTime = entryT;
        this->leaveTime = leaveT;

        this->entrySpeed = entryS;
        this->leaveSpeed = leaveS;
    }
};


class RSUEntry
{
  public:
      char name[20];
      double coordX;
      double coordY;

      RSUEntry(const char *str, double x, double y)
      {
          strcpy(this->name, str);
          this->coordX = x;
          this->coordY = y;
      }
};


class NearestVehicle
{
  public:
      char name[20];
      int depth;
      double dist;

      NearestVehicle(const char *str, int n, double x)
      {
          strcpy(this->name, str);
          this->depth = n;
          this->dist = x;
      }
};


class CurrentVehicleState : public cObject, noncopyable
{
  public:
      char name[20];
      char state[40];

      CurrentVehicleState(const char *str1, const char *str2)
      {
          strcpy(this->name, str1);
          strcpy(this->state, str2);
      }
};


class CurrentPlnMsg : public cObject, noncopyable
{
  public:
      PlatoonMsg *msg;
      char type[30];

      CurrentPlnMsg(PlatoonMsg *m, const char *str)
      {
          this->msg = m;
          strcpy(this->type, str);
      }
};


class plnManagement
{
  public:
      double time;
      char sender[20];
      char receiver[20];
      char type[30];
      char sendingPlnID[20];
      char receivingPlnID[20];

      plnManagement(double t, const char *str1, const char *str2, const char *str3, const char *str4, const char *str5)
      {
          this->time = t;

          strcpy(this->sender, str1);
          strcpy(this->receiver, str2);
          strcpy(this->type, str3);
          strcpy(this->sendingPlnID, str4);
          strcpy(this->receivingPlnID, str5);
      }
};
}

#endif

