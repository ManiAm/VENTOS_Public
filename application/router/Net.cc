#include "Net.h"


namespace VENTOS {

Net::~Net()
{

}

Net::Net(string netFile)
{
    transitions = new map<string, vector<int>* >;
    turnTypes = new map<string, char>;

    Statistics* statPtr = FindModule<Statistics*>::findGlobalModule();

    file <> xmlFile(netFile.c_str());   //Make a new rapidXML document to parse
    xml_document<> doc;
    xml_node<> *node;
    doc.parse<0>(xmlFile.data());
    for(node = doc.first_node()->first_node("junction"); node; node = node->next_sibling("junction"))
    {   //For every node
        xml_attribute<> *attr = node->first_attribute();    //Read all the attributes in
        string id = attr->value();

        attr = attr->next_attribute();
        string type = attr->value();

        attr = attr->next_attribute();
        double x = atof(attr->value());

        attr = attr->next_attribute();
        double y = atof(attr->value());

        attr = attr->next_attribute();          //Get its list of incoming lanes
        string incLanesString = attr->value();
        vector<string>* incLanes = new vector<string>;               //And break them into a vector of lanes
        stringstream ssin(incLanesString);
        while(ssin.good())
        {
            string temp;
            ssin >> temp;
            if(temp.length() > 0)
                incLanes->push_back(temp);
        }

        TrafficLight *tl = NULL;
        if(type == "traffic_light") //If we're looking at a traffic light
        {
            //Maybe check if the traffic light already exists before building a new one?  Depends on if multiple intersections can go to the same logic
            for(xml_node<> *tlNode = doc.first_node()->first_node("tlLogic"); tlNode; tlNode = tlNode->next_sibling("tlLogic"))
            {   //Search through all the tlLogic, find the linked one
                if(tlNode->first_attribute()->value() == id)
                {
                    xml_attribute<> *tlAttr = tlNode->first_attribute();    //Read its attributes in
                    string tlid = tlAttr->value();
                    tlAttr = tlAttr->next_attribute();
                    string tltype = tlAttr->value();
                    tlAttr = tlAttr->next_attribute();
                    string programID = tlAttr->value();
                    tlAttr = tlAttr->next_attribute();
                    double tloffset = atof(tlAttr->value());

                    vector<Phase*>* phasesVec = new vector<Phase*>; //Read its list of phases in
                    for(xml_node<> *phaseNode = tlNode->first_node("phase"); phaseNode; phaseNode = phaseNode->next_sibling("phase"))
                    {
                        double duration = atof(phaseNode->first_attribute()->value());
                        string state = phaseNode->first_attribute()->next_attribute()->value();
                        Phase *ph = new Phase(duration, state);
                        phasesVec->push_back(ph);   //Build and link a new phase for each attribute
                    }

                    tl = new TrafficLight(tlid, tltype, programID, tloffset, phasesVec);    //And build the traffic light with all this info
                    TLs.push_back(tl);
                    break;
                }//if matching traffic light
            }//for each traffic light
        }//if traffic light

        Node *n = new Node(id, x, y, type, incLanes, tl); //Build it
        nodes.push_back(n);    // Add it to the router's node vector
    }
    sort(nodes.begin(), nodes.end(), NodeIDSort);   //Sort them, for binary searches later on

    for(node = doc.first_node()->first_node("edge"); node; node = node->next_sibling("edge"))
    {   //For every edge
        xml_attribute<> *attr = node->first_attribute();    //Read the basic attributes in
        string id = attr->value();
        attr = attr->next_attribute();
        string fromVal = attr->value();
        attr = attr->next_attribute();
        string toVal = attr->value();
        attr = attr->next_attribute();
        int priority = atoi(attr->value());

        vector<Lane*>* lanesVec = new vector<Lane*>;    //For every lane on that edge
        for(xml_node<> *lane = node->first_node(); lane; lane = lane->next_sibling())
        {
            attr = lane->first_attribute(); //Read the lane's attributes in
            string laneid = attr->value();
            attr = attr->next_attribute()->next_attribute();
            double speed = atof(attr->value());
            attr = attr->next_attribute();
            double length = atof(attr->value());
            Lane* l = new Lane(laneid, speed, length);  //Build the lane
            lanesVec->push_back(l); //Add it to a vector of this edge's lanes
        }
        Node* from = binarySearch(nodes, fromVal);  //Get a pointer to the start node
        Node* to = binarySearch(nodes, toVal);      //Get a pointer to the end node
        Edge* e = new Edge(id, from, to, priority, lanesVec, &(*(statPtr->edgeHistograms.find(id))).second);
        from->outEdges.push_back(e);   //Add the edge to the start node's list
        edges.push_back(e);         //And to the list of all edges
    }   //For every edge
    sort(edges.begin(), edges.end(), EdgeIDSort); //Sort the edges for our binary searches later
    for(vector<Edge*>::iterator it = edges.begin(); it != edges.end(); it++)
        (*it)->to->inEdges.push_back(*it);

    //Make a new mapping from string to int vector.  strings will be the start and end lanes, and lanes will be the lane numbers from start than connect them.
    for(node = doc.first_node()->first_node("connection"); node; node = node->next_sibling("connection"))
    {   //For every connection
        xml_attribute<> *attr = node->first_attribute();    //Read in the start and end edge ids, and the lane number of this instance.
        string e1 = attr->value();
        attr = attr->next_attribute();
        string e2 = attr->value();
        attr = attr->next_attribute();
        int lane = atoi(attr->value());
        string key = e1 + e2;       //Key is the concatenation of both IDs.
        attr = attr->next_attribute()->next_attribute(); //Skip the tolane field
        //cout << "For key " << key << endl;

        //cout << "Read " << e1 << " " << e2 << " " << lane << endl;
        if((string)attr->name() == "tl")    //Read the tl attributes if necessary
        {
            if(transitions->find(key) == transitions->end()) //If this vector doesn't yet exist
                (*transitions)[key] = new vector<int>;  //Create an empty vector in place

            TrafficLight* tl = binarySearch(TLs, attr->value());    //Find the associated traffic light

            attr = attr->next_attribute();
            int linkIndex = atoi(attr->value());

            for(unsigned int i = 0; i < tl->phases->size(); i++)
            {
                Phase* phase = (*tl->phases)[i];
                if(phase->state[linkIndex] != 'r')
                {
                    (*transitions)[key]->push_back(i);
                }
            }
        }
        attr = attr->next_attribute();
        (*turnTypes)[key] = attr->value()[0];

    }
}

} // end of namespace
