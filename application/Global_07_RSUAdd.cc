
#include "Global_07_RSUAdd.h"

namespace VENTOS {

Define_Module(VENTOS::RSUAdd);


RSUAdd::~RSUAdd()
{

}


void RSUAdd::initialize(int stage)
{
    if(stage ==0)
    {
        // get the ptr of the current module
        nodePtr = FindModule<>::findHost(this);
        if(nodePtr == NULL)
            error("can not get a pointer to the module.");

        // get a pointer to the TraCI module
        cModule *module = simulation.getSystemModule()->getSubmodule("TraCI");
        TraCI = static_cast<TraCI_Extend *>(module);

        boost::filesystem::path SUMODirectory = simulation.getSystemModule()->par("SUMODirectory").stringValue();
        boost::filesystem::path VENTOSfullDirectory = cSimulation::getActiveSimulation()->getEnvir()->getConfig()->getConfigEntry("network").getBaseDirectory();
        SUMOfullDirectory = VENTOSfullDirectory / SUMODirectory;   // home/mani/Desktop/VENTOS/sumo/CACC_Platoon

        on = par("on").boolValue();
        mode = par("mode").longValue();
    }
}


void RSUAdd::handleMessage(cMessage *msg)
{

}


deque<RSUEntry*> RSUAdd::commandReadRSUsCoord(string RSUfilePath)
{
    file<> xmlFile( RSUfilePath.c_str() );        // Convert our file to a rapid-xml readable object
    xml_document<> doc;                           // Build a rapidxml doc
    doc.parse<0>(xmlFile.data());                 // Fill it with data from our file
    xml_node<> *node = doc.first_node("RSUs");    // Parse up to the "RSUs" declaration

    for(node = node->first_node("poly"); node; node = node->next_sibling())
    {
        string RSUname = "";
        string RSUtype = "";
        string RSUcoordinates = "";
        int readCount = 1;

        // For each node, iterate over its attributes until we reach "shape"
        for(xml_attribute<> *attr = node->first_attribute(); attr; attr = attr->next_attribute())
        {
            if(readCount == 1)
            {
                RSUname = attr->value();
            }
            else if(readCount == 2)
            {
                RSUtype = attr->value();
            }
            else if(readCount == 3)
            {
                RSUcoordinates = attr->value();
            }

            readCount++;
        }

        // tokenize coordinates
        int readCount2 = 1;
        double x;
        double y;
        char_separator<char> sep(",");
        tokenizer< char_separator<char> > tokens(RSUcoordinates, sep);

        for(tokenizer< char_separator<char> >::iterator beg=tokens.begin(); beg!=tokens.end();++beg)
        {
            if(readCount2 == 1)
            {
                x = atof( (*beg).c_str() );
            }
            else if(readCount2 == 2)
            {
                y = atof( (*beg).c_str() );
            }

            readCount2++;
        }

        // add it into queue (with TraCI coordinates)
        RSUEntry *entry = new RSUEntry(RSUname.c_str(), x, y);
        RSUs.push_back(entry);
    }

    return RSUs;
}


Coord *RSUAdd::commandGetRSUsCoord(unsigned int index)
{
    if( RSUs.size() == 0 )
        error("No RSUs have been initialized!");

    if( index < 0 || index >= RSUs.size() )
        error("index out of bound!");

    Coord *point = new Coord(RSUs[index]->coordX, RSUs[index]->coordY);
    return point;
}


void RSUAdd::commandAddCirclePoly(string name, string type, const TraCIColor& color, Coord *center, double radius)
{
    list<TraCICoord> circlePoints;

    // Convert from degrees to radians via multiplication by PI/180
    for(int angleInDegrees = 0; angleInDegrees <= 360; angleInDegrees = angleInDegrees + 10)
    {
        double x = (double)( radius * cos(angleInDegrees * 3.14 / 180) ) + center->x;
        double y = (double)( radius * sin(angleInDegrees * 3.14 / 180) ) + center->y;

        circlePoints.push_back(TraCICoord(x, y));
    }

    // create polygon in SUMO
    TraCI->getCommandInterface()->addPolygon(name, type, color, 0, 1, circlePoints);
}


void RSUAdd::Add()
{
    // if dynamic adding is off, return
    if (!on)
        return;

    if(mode == 1)
    {
        Scenario1();
    }
}


void RSUAdd::Scenario1()
{
    // ####################################
    // Step 1: read RSU locations from file
    // ####################################

    string RSUfile = par("RSUfile").stringValue();
    boost::filesystem::path RSUfilePath = SUMOfullDirectory / RSUfile;

    // check if this file is valid?
    if( !exists( RSUfilePath ) )
    {
        error("RSU file does not exist in %s", RSUfilePath.string().c_str());
    }

    deque<RSUEntry*> RSUs = commandReadRSUsCoord(RSUfilePath.string());

    // ################################
    // Step 2: create RSUs into OMNET++
    // ################################

    int NoRSUs = RSUs.size();

    cModule* parentMod = getParentModule();
    if (!parentMod) error("Parent Module not found");

    cModuleType* nodeType = cModuleType::get("c3po.ned.RSU");

    // We only create RSUs in OMNET++ without moving them to
    // the correct coordinate
    for(int i = 0; i < NoRSUs; i++)
    {
        cModule* mod = nodeType->create("RSU", parentMod, NoRSUs, i);
        mod->finalizeParameters();
        mod->getDisplayString().updateWith("i=device/antennatower");
        mod->buildInside();
        mod->scheduleStart(simTime());
        mod->callInitialize();
    }

    // ##################################################
    // Step 3: create a polygon for radio circle in SUMO
    // ##################################################

    // get the radius of an RSU
    cModule *module = simulation.getSystemModule()->getSubmodule("RSU", 0);
    double radius = atof( module->getDisplayString().getTagArg("r",0) );

    for(int i = 0; i < NoRSUs; i++)
    {
        Coord *center = new Coord(RSUs[i]->coordX, RSUs[i]->coordY);
        commandAddCirclePoly(RSUs[i]->name, "RSU", TraCIColor::fromTkColor("blue"), center, radius);
    }
}


void RSUAdd::finish()
{


}
}

