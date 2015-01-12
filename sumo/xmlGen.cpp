#include<iostream>
#include<fstream>
#include "../rapidxml-1.13/rapidxml.hpp"
#include "../rapidxml-1.13/rapidxml_utils.hpp"
#include<stdlib.h>
#include<time.h>
using namespace std;
    

bool tryOpen(string fileName)
{
  ifstream file(fileName.c_str());
  if(file.good())
  {
    cout << "File " << fileName << " already exists.  Overwrite? (y/n): ";
    char c;
    cin >> c;
    if(c == 'y')
    {
      return 1;
    }
    else
    {
      return 0;
    }
  }
  file.close();
  return 1;
}

using namespace rapidxml;
vector<string> getEdgeNames(string netName)
{
  vector<string> edgeNames;

  file <> xmlFile(netName.c_str());
  xml_document<> doc;
  xml_node<> *node;
  doc.parse<0>(xmlFile.data());
  for(node = doc.first_node()->first_node("edge"); node; node = node->next_sibling("edge"))
    edgeNames.push_back(node->first_attribute()->value());
  return edgeNames;
}

vector<string> getNodeNames(string netName)
{
  vector<string> nodeNames;
  file <> xmlFile(netName.c_str());
  xml_document<> doc;
  xml_node<> *node;
  doc.parse<0>(xmlFile.data());
  for(node = doc.first_node()->first_node("junction"); node; node = node->next_sibling("junction"))
    nodeNames.push_back(node->first_attribute()->value());
  return nodeNames;
}


void bulkVehicles()
{
    cout << "Enter a subdirectory containing hello.net.xml.  This is where Vehicles###.xml will be placed: ";
    string dir;
    cin >> dir;

    cout << "Create how many vehicles?: ";
    int vCount;
    cin >> vCount;

    cout << "Over how many seconds?: ";
    double duration;
    cin >> duration;
    
    string netName = dir + "/hello.net.xml";
    string vName = dir + "/Vehicles" + vCount + ".xml";
    ifstream netFile(netName.c_str());
    if(!netFile.good())
    {
      cout << netName << " does not exist!";
      return;
    }
    if(tryOpen(vName))
    {
      srand(time(NULL));
      vector<string> edgeNames = getEdgeNames(netName);
      vector<string> nodeNames = getNodeNames(netName);

      ofstream vFile(vName.c_str());
      vFile << "<vehicles>" << endl;
      for(int i = 1; i <= vCount; i++)
      {
        string edge = edgeNames[rand() % edgeNames.size()];
        string node = nodeNames[rand() % nodeNames.size()]; 
        vFile << "   <vehicle id=\"v" << i << "\" type=\"TypeManual\" origin=\"" << edge << "\" destination=\"" << node << "\" depart=\"" << i * duration / vCount << "\" />" << endl;
      }
      vFile << "</vehicles>" << endl;
      vFile.close();
    }
}

void buildXML(string fileName, string xmlType, string xmlTypePlural, string *proper, string *xml, int size)
{
  if(tryOpen(fileName))
  {
    ofstream file(fileName.c_str());
    file << "<" << xmlTypePlural << ">" << endl;

    bool stop = 0;
    while(!stop)
    {
      string input[size];
      cout << "Build an " << xmlType << ", or enter 'q' at any time to finish:" << endl;
      for(int i = 0; i < size; i++)
      {
        cout << proper[i] << ": ";
        cin >> input[i];
        if(input[i] == "q")
        {
          stop = true;
          break;
        }
      }
      if(!stop)
      {
        file << "    <" << xmlType << " ";
        for(int i = 0; i < size; i++)
        {
          file << xml[i] << "=\"" << input[i] << "\" ";
        }
        file << "/>" << endl;
      }
    }
    
    file << "</" << xmlTypePlural << ">" << endl;
  }
}

int main(int argc, char* argv[])
{
  string input;

  if(argc == 1)
  {
    cout << "1: Generate edges" << endl
         << "2: Generate nodes" << endl
         << "3: Generate individual vehicles" << endl
         << "4: Generate many random vehicles" << endl
         << "Enter a number: ";
    cin >> input;
  }
  else
  {
    input = argv[1];
  }

  if(input == "1")
  {
    string values[6] = {"Edge ID","From Node","To Node","Priority","Number of Lanes","Speed Limit"};
    string xml[6] = {"id", "from", "to", "priority", "numLanes", "speed"};
    buildXML("edges.xml", "edge", "edges", values, xml, 6);
  }
  else if(input == "2")
  {
    string values[4] = {"Node ID", "X Coordinate", "Y Coordinate", "Type"};
    string xml[4] = {"id", "x", "y", "type"};
    buildXML("nodes.xml", "node", "nodes", values, xml, 4);
  }
  else if(input == "3")
  {
    string values[5] = {"Name", "Type", "Origin Lane", "Destination Node", "Depart Time"};
    string xml[5] = {"id", "type", "origin", "destination", "depart"};
    buildXML("vehicles.xml", "vehicle", "vehicles", values, xml, 5);
  }
  else if(input == "4")
  {
    bulkVehicles();

  }
  else
  {
    cout << "Improper arguments!  Use -n, -e, or -v for nodes, edges, or vehicles." << endl;
  }
  return 0;
}



