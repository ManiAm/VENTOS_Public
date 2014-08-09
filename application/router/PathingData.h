#ifndef NODEEDGE_H
#define NODEEDGE_H

#include<vector>
#include<map>
#include<iostream>

using namespace std;

namespace VENTOS {

class Edge;

class Node;

class PathingData
{
public:
    Node* toNode;
    vector<double>* probabilities;  //p
    vector<double>* timeToEnd;      //T
    vector<double>* labels;         //gamma
                                    //f = traffic light cost

    PathingData(Node* n)
    {
        toNode = n;
        probabilities = new vector<double>;
        timeToEnd = new vector<double>;
        labels = new vector<double>;
    }
    ~PathingData()
    {
        delete probabilities;
        delete timeToEnd;
        delete labels;
    }
};

template<typename T, typename R>    //Recursive call from the below function
T* binarySearch(vector<T*> &v, R idVal, int minIndex, int maxIndex)
{
    if(maxIndex < minIndex)
    {
        cout << "binarySearch failed to find the specified object!"; // DEBUG
        return NULL;
    }
    int index = (minIndex + maxIndex) / 2;                      //Cut the search in half
    if(v[index]->id > idVal)                                  //If we're smaller, check the bottom half
        return binarySearch(v, idVal, minIndex, index - 1);
    else if(v[index]->id < idVal)                             //If we're larger, check the top half
        return binarySearch(v, idVal, index + 1, maxIndex);
    else                                                        //Otherwise, we're equal.  Return.
        return v[index];
}

template<typename T, typename R>    //Handy generic binary search for a sorted vector of objects of type T and keys of type R.  Needs implementations of < and >
T* binarySearch(vector<T*> &v, R idVal)
{
    return binarySearch(v, idVal, 0, v.size());
}

}

#endif
