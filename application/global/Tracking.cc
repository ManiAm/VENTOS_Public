
#include "Tracking.h"

Define_Module(VENTOS::Tracking);

namespace VENTOS {

Tracking::~Tracking()
{

}


void Tracking::initialize(int stage)
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

        Signal_executeFirstTS = registerSignal("executeFirstTS");
        simulation.getSystemModule()->subscribe("executeFirstTS", this);

        on = par("on").boolValue();

        zoom = par("zoom").doubleValue();
        if(zoom < 0)
            error("zoom value is not correct!");

        initialWindowsOffset = par("initialWindowsOffset").doubleValue();
        if(initialWindowsOffset < 0)
            error("Initial Windows Offset value is not correct!");

        trackingInterval = par("trackingInterval").doubleValue();
        if(trackingInterval <= 0)
            error("Tracking interval should be positive!");

        mode = par("mode").longValue();
        trackingV = par("trackingV").stdstringValue();
        trackingLane = par("trackingLane").stringValue();
        windowsOffset = par("windowsOffset").doubleValue();

        updataGUI = new cMessage("updataGUI", 1);
    }
}


void Tracking::finish()
{

}


void Tracking::receiveSignal(cComponent *source, simsignal_t signalID, long i)
{
    Enter_Method_Silent();

    if(signalID == Signal_executeFirstTS)
    {
        Tracking::Start();
    }
}


void Tracking::Start()
{
    // todo:
    // use TraCI command, to also check if we are really running in GUI mode

    if(!on)
        return;

    // zoom-in GUI
    TraCI->commandSetGUIZoom(zoom);

    // adjust Windows initially
    TraCI->commandSetGUIOffset(initialWindowsOffset, 0.);

    TrackingGUI();
}


void Tracking::handleMessage(cMessage *msg)
{
    if (msg == updataGUI)
    {
        TrackingGUI();
    }
}


void Tracking::TrackingGUI()
{
    if(mode == 1)
    {
        // get vehicle position in SUMO coordinates
        Coord co = TraCI->commandGetVehiclePos(trackingV);

        if(co.x > 0)
            TraCI->commandSetGUIOffset(co.x, co.y);
    }
    else if(mode == 2)
    {
        // get a list of vehicles on this lane!
        list<string> myList = TraCI->commandGetLaneVehicleList(trackingLane.c_str());

        if(!myList.empty())
        {
            // get iterator to the end
            list<string>::iterator it = myList.end();

            // iterator pointing to the last element
            --it;

            // first inserted vehicle on this lane
            string lastVehicleId = *it;

            Coord lastVehiclePos = TraCI->commandGetVehiclePos(lastVehicleId);

            // get GUI windows boundary
            vector<double> windowsFrame = TraCI->commandGetGUIBoundry();

            // vehicle goes out of frame?
            if(lastVehiclePos.x > windowsFrame[2] || lastVehiclePos.y > windowsFrame[3])
            {
                TraCI->commandSetGUIOffset(windowsFrame[0] + windowsOffset, 0);
            }
        }
    }
    else
    {
        error("not a valid mode!");
    }

    scheduleAt(simTime() + trackingInterval, updataGUI);
}

}

