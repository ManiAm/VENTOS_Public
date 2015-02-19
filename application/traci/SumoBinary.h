
#ifndef CONNECTIONEXTEND_H_
#define CONNECTIONEXTEND_H_

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

class SumoBinary : public BaseModule
{
	public:
		virtual ~SumoBinary() {};
        virtual void initialize(int stage);
        virtual void handleMessage(cMessage *msg);
        virtual void finish();

	protected:
        string SUMO_CMD_FileName;
        string SUMO_GUI_FileName;

        string SUMO_GUI_URL;
        string SUMO_CMD_URL;
        string SUMO_Version_URL;

        boost::filesystem::path VENTOS_FullPath;
        boost::filesystem::path SUMO_Binary_FullPath;

        boost::filesystem::path SUMO_CMD_Binary_FullPath;
        boost::filesystem::path SUMO_GUI_Binary_FullPath;

        void checkIfBinaryExists();
        void downloadBinary(string, string, string);
        void makeExecutable(string, string);
        void checkIfNewerVersionExists(string, string, string);
        int getRemoteVersion();
};

}

#endif
