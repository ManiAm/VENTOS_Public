

class data : public cObject, noncopyable
{
  public:
    char name[10];

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
  char name1[10];
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
	char name1[10];
    int nodeID;  // is used to sort the vector (used as a key)

    char name2[10];
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
