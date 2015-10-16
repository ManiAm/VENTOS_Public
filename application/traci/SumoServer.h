/****************************************************************************/
/// @file    SumoServer.h
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author  second author name
/// @date    August 2013
///
/****************************************************************************/
// VENTOS, Vehicular Network Open Simulator; see http:?
// Copyright (C) 2013-2015
/****************************************************************************/
//
// This file is part of VENTOS.
// VENTOS is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#ifndef SUMOSERVER_H_
#define SUMOSERVER_H_

#include <BaseApplLayer.h>
#include "boost/algorithm/string.hpp"

#undef ev
#include "boost/filesystem.hpp"
#define ev  (*cSimulation::getActiveEnvir())


namespace VENTOS {

class SumoServer : public BaseApplLayer
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
        std::string SUMO_CMD_FileName;
        std::string SUMO_GUI_FileName;

        boost::filesystem::path VENTOS_FullPath;
        boost::filesystem::path SUMO_Binary_FullPath;
        boost::filesystem::path SUMO_CMD_Binary_FullPath;
        boost::filesystem::path SUMO_GUI_Binary_FullPath;

        void createServerSocket();
};

}

#endif
