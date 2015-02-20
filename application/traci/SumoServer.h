
#ifndef SUMOSERVER_H_
#define SUMOSERVER_H_

#include <BaseApplLayer.h>
#include "boost/algorithm/string.hpp"
#include <boost/tokenizer.hpp>

#undef ev
#include "boost/filesystem.hpp"
#define ev  (*cSimulation::getActiveEnvir())

using namespace boost::filesystem;
using namespace boost;
using namespace std;

namespace VENTOS {

class SumoServer : public BaseModule
{
	public:
		virtual ~SumoServer() {};
        virtual void initialize(int stage);
        virtual int numInitStages() const
        {
            return 3;  // stage 0, 1, 2
        }
        virtual void handleMessage(cMessage *msg);
        virtual void finish();

	protected:
        // NED variables
        bool update;
        string SUMO_CMD_FileName;
        string SUMO_GUI_FileName;

        boost::filesystem::path VENTOS_FullPath;
        boost::filesystem::path SUMO_Binary_FullPath;
        boost::filesystem::path SUMO_CMD_Binary_FullPath;
        boost::filesystem::path SUMO_GUI_Binary_FullPath;

        void createServerSocket();
};

}

#endif
