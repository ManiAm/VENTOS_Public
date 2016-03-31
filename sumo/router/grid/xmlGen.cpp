#include<iostream>
#include<fstream>
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
          if(input[i] != "null")
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
    cout << "Enter -n, -e, or -v to generate nodes, edges, or vehicles: ";
    cin >> input;
  }
  else
  {
    input = argv[1];
  }

  if(input == "-e")
  {
    string values[6] = {"Edge ID","From Node","To Node","Priority","Number of Lanes","Speed Limit"};
    string xml[6] = {"id", "from", "to", "priority", "numLanes", "speed"};
    buildXML("edges.xml", "edge", "edges", values, xml, 6);
  }
  else if(input == "-n")
  {
    string values[4] = {"Node ID", "X Coordinate", "Y Coordinate", "Type (traffic_light or null)"};
    string xml[4] = {"id", "x", "y", "type"};
    buildXML("nodes.xml", "node", "nodes", values, xml, 4);
  }
  else if(input == "-v")
  {
    string values[5] = {"Name", "Type", "Origin Lane", "Destination Node", "Depart Time"};
    string xml[5] = {"id", "type", "origin", "destination", "depart"};
    buildXML("vehicles.xml", "vehicle", "vehicles", values, xml, 5);
  }
  else
  {
    cout << "Improper arguments!  Use -n, -e, or -v for nodes, edges, or vehicles." << endl;
  }
  return 0;
}



