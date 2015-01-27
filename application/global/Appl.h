#ifndef Appl_H
#define Appl_H

#include <list>
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


// for beacons
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


class PlnManeuver : public cObject, noncopyable
{
  public:
      char from[20];
      char to[20];
      char maneuver[30];

      PlnManeuver(const char *str1, const char *str2, const char *str3)
      {
          strcpy(this->from, str1);
          strcpy(this->to, str2);
          strcpy(this->maneuver, str3);
      }
};

class TimeData : public cObject, noncopyable
{
  public:
      string vName;
      int time;
      bool end;

      TimeData(string vName, int time, bool end)
      {
          this->vName = vName;
          this->time = time;
          this->end = end;
      }
};


}

#endif

