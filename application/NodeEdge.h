#ifndef NODEEDGE_H
#define NODEEDGE_H

#include<vector>

using namespace std;

class EdgePair;
class Edge;

class Node  //Node in our graph, points to a bunch of other edges and nodes
{
public:
    //Pathing variables
    bool visited;   //If we've
    double curCost;
    Edge* best;
    vector<Edge*> edges;

    //Node variables
    string id;
    double x;
    double y;
    string type;

    Node(string idVal, double xVal, double yVal, string typeVal);
    Node();
    bool operator<(Node &rhs);   //For priority queue sorting
    bool operator>(Node &rhs);
    ostream& operator<<(ostream& os);
};

class Lane
{
public:
    string id;
    double length;
    Lane(string i, double l);
};

class Edge  //Edge in our graph.  Will need more data later
{
public:

    //Node variables
    string id;
    Node* from;
    Node* to;
    int priority;
    int numLanes;

    //Weighting variables
    double speed;
    double length;

    double origCost;
    double lastCost;
    vector<Lane> lanes;
    double updateLength();

    Edge(string idVal, Node* fromVal, Node* toVal, int priorityVal, int numLanesVal, double speedVal);  //This will need more arguments
    double getCost();
    ostream& operator<<(ostream& os);
};

bool NodeIDSort(const Node &n1, const Node &n2);//Internal prototypes
bool EdgeIDSort(const Edge &n1, const Edge &n2);
ostream& operator<<(ostream& os, Node &rhs);
ostream& operator<<(ostream& os, Edge &rhs);


template<typename T, typename R>    //Recursive call from the below function
T* binarySearch(vector<T> *v, R idVal, int minIndex, int maxIndex)
{
    if(maxIndex < minIndex)
    {
        cout << "binarySearch failed to find the specified object!"; // DEBUG
        return NULL;
    }
    int index = (minIndex + maxIndex) / 2;                      //Cut the search in half
    if((*v)[index].id > idVal)                                  //If we're smaller, check the bottom half
        return binarySearch(v, idVal, minIndex, index - 1);
    else if((*v)[index].id < idVal)                             //If we're larger, check the top half
        return binarySearch(v, idVal, index + 1, maxIndex);
    else                                                        //Otherwise, we're equal.  Return.
        return &((*v)[index]);
}

template<typename T, typename R>    //Handy generic binary search for a sorted vector of objects of type T and keys of type R.  Needs implementations of < and >
T* binarySearch(vector<T> *v, R idVal)
{
    return binarySearch(v, idVal, 0, v->size());
}

class EdgePair  //Replacement for C++'s Pair() class, more readable, modifiable
{
public:
    Node* node; //Destination node
    Edge* edge; //Edge taken to get their

    EdgePair();
    EdgePair(Edge* e, Node* n); //Builds an edgepair without pathing info
    bool operator<(EdgePair &rhs);
    bool operator>(EdgePair &rhs);
};

#endif
