

// used to send data from CA to magic cars
class data : public cObject, noncopyable
{
  public:
    char name[10];

    data(const char *str)
    {
        strcpy(this->name, str);
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
