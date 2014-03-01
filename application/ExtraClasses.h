

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
    std::vector<long> vec;

    MacStat( std::vector<long> v)
    {
        vec.swap(v);
    }
};


class MacStatEntry
{
public:
  char name1[20];
  int nodeID;  // is used to sort the vector (used as a key)
  simtime_t time;
  std::vector<long> MacStatsVec;

  MacStatEntry(const char *str, int id, simtime_t t, std::vector<long> v)
  {
      strcpy(this->name1, str);
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

