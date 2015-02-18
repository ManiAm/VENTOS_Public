
#ifndef TRACIMOBILITYMOD_H
#define TRACIMOBILITYMOD_H

#include <veins/modules/mobility/traci/TraCIMobility.h>

namespace VENTOS {

class TraCIMobilityMod : public Veins::TraCIMobility
{
	public:
        TraCIMobilityMod() { }
        ~TraCIMobilityMod() { }

        virtual void initialize(int);
		virtual void updateDisplayString();

	protected:
		std::string moduleName;
		std::string bikeModuleName;
		std::string pedModuleName;
};

}



#endif

