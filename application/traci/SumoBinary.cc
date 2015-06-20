/****************************************************************************/
/// @file    SumoBinary.cc
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

#include "SumoBinary.h"
#include <curl/curl.h>

namespace VENTOS {

Define_Module(VENTOS::SumoBinary);

void SumoBinary::initialize(int stage)
{
    if(stage == 0)
    {
        SUMO_GUI_FileName = par("SUMO_GUI_FileName").stringValue();
        SUMO_CMD_FileName = par("SUMO_CMD_FileName").stringValue();

        SUMO_GUI_URL = par("SUMO_GUI_URL").stringValue();
        SUMO_CMD_URL = par("SUMO_CMD_URL").stringValue();
        SUMO_Version_URL = par("SUMO_Version_URL").stringValue();

        VENTOS_FullPath = cSimulation::getActiveSimulation()->getEnvir()->getConfig()->getConfigEntry("network").getBaseDirectory();
        SUMO_Binary_FullPath = VENTOS_FullPath / "sumoBinary";

        SUMO_GUI_Binary_FullPath = SUMO_Binary_FullPath / SUMO_GUI_FileName;
        SUMO_CMD_Binary_FullPath = SUMO_Binary_FullPath / SUMO_CMD_FileName;

        update = par("update").boolValue();

        // todo: check each day!
        // a way to turn this feature off
        checkIfBinaryExists();
    }
}


void SumoBinary::finish()
{

}


void SumoBinary::handleMessage(cMessage *msg)
{

}


void SumoBinary::checkIfBinaryExists()
{
    // both binaries are missing
    if( !boost::filesystem::exists( SUMO_CMD_Binary_FullPath ) && !exists( SUMO_GUI_Binary_FullPath ) )
    {
        std::cout << "\nNo SUMO binaries found in " << SUMO_Binary_FullPath.string() << endl;
        std::cout << "Do you want to download the latest SUMO binaries? [y/n]: ";
        std::string answer;
        std::cin >> answer;
        boost::algorithm::to_lower(answer);

        if(answer == "y")
        {
            downloadBinary(SUMO_GUI_FileName, SUMO_GUI_Binary_FullPath.string(), SUMO_GUI_URL);
            downloadBinary(SUMO_CMD_FileName, SUMO_CMD_Binary_FullPath.string(), SUMO_CMD_URL);
        }
        else
            std::cout << "Ok! have fun." << endl;
    }
    // only GUI binary is missing
    else if( boost::filesystem::exists( SUMO_CMD_Binary_FullPath ) && !exists( SUMO_GUI_Binary_FullPath ) )
    {
        std::cout << "\nSUMO GUI binary is missing in " << SUMO_Binary_FullPath.string() << endl;
        std::cout << "Do you want to download it ? [y/n]: ";
        std::string answer;
        std::cin >> answer;
        boost::algorithm::to_lower(answer);

        if(answer == "y")
            downloadBinary(SUMO_GUI_FileName, SUMO_GUI_Binary_FullPath.string(), SUMO_GUI_URL);

        checkIfNewerVersionExists(SUMO_CMD_FileName, SUMO_CMD_Binary_FullPath.string(), SUMO_CMD_URL);
    }
    // only CMD binary is missing
    else if( !boost::filesystem::exists( SUMO_CMD_Binary_FullPath ) && exists( SUMO_GUI_Binary_FullPath ) )
    {
        std::cout << "\nSUMO CMD binary is missing in " << SUMO_Binary_FullPath.string() << endl;
        std::cout << "Do you want to download it ? [y/n]: ";
        std::string answer;
        std::cin >> answer;
        boost::algorithm::to_lower(answer);

        if(answer == "y")
            downloadBinary(SUMO_CMD_FileName, SUMO_CMD_Binary_FullPath.string(), SUMO_CMD_URL);

        checkIfNewerVersionExists(SUMO_GUI_FileName, SUMO_GUI_Binary_FullPath.string(), SUMO_GUI_URL);
    }
    // both binaries exists, check if newer version is available?
    else
    {
        checkIfNewerVersionExists(SUMO_GUI_FileName, SUMO_GUI_Binary_FullPath.string(), SUMO_GUI_URL);
        checkIfNewerVersionExists(SUMO_CMD_FileName, SUMO_CMD_Binary_FullPath.string(), SUMO_CMD_URL);
    }
}


size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

void SumoBinary::downloadBinary(std::string binaryName, std::string filePath, std::string url)
{
    std::cout << "Downloading " << binaryName << " ... ";
    std::cout.flush();

    FILE *fp = fopen(filePath.c_str(), "wb");
    if(fp == NULL)
    {
        fprintf(stderr, " failed! (%s)\n", "writing error! Do you have permission?");
        return;
    }

    CURL *curl = curl_easy_init();

    if (curl == NULL)
    {
        fprintf(stderr, " failed! (%s)\n", "error in curl_easy_init");
        fclose(fp);
        return;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    // we tell libcurl to follow redirection
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 6);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

    CURLcode res = curl_easy_perform(curl);
    if(res != CURLE_OK)
    {
        fprintf(stderr, " failed! (%s)\n", curl_easy_strerror(res));
        fclose(fp);
        return;
    }
    else
    {
        std::cout << " done!" << endl;
        fclose(fp);
        makeExecutable(binaryName, filePath);
    }

    curl_easy_cleanup(curl);
}


void SumoBinary::makeExecutable(std::string binaryName, std::string filePath)
{
    std::cout << "Making " << binaryName << " executable ... ";
    std::cout.flush();

    char command[100];
    sprintf(command, "chmod +x %s", filePath.c_str());

    FILE* pipe = popen(command, "r");
    if (!pipe)
    {
        std::cout << "failed! (can not open pipe)" << endl;
        return;
    }

    char buffer[128];
    std::string result = "";
    while(!feof(pipe))
    {
        if(fgets(buffer, 128, pipe) != NULL)
            result += buffer;
    }
    pclose(pipe);

    std::cout << " done!" << endl;
}


static std::string remoteVer;

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    remoteVer.append((const char*)contents, realsize);
    return realsize;
}

// get remote file and save it in memory
int SumoBinary::getRemoteVersion()
{
    remoteVer.clear();

    CURL *curl = curl_easy_init();
    if (curl == NULL)
    {
        fprintf(stderr, " failed! (%s)\n", "error in curl_easy_init");
        return -1;
    }

    curl_easy_setopt(curl, CURLOPT_URL, SUMO_Version_URL.c_str());
    // we tell libcurl to follow redirection
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 6);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    /* send all data to this function  */
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);

    CURLcode res = curl_easy_perform(curl);
    if(res != CURLE_OK)
    {
        fprintf(stderr, " failed! (%s)\n", curl_easy_strerror(res));
        return -1;
    }
    else
        return 1;

    curl_easy_cleanup(curl);
}


void SumoBinary::checkIfNewerVersionExists(std::string binaryName, std::string filePath, std::string url)
{
    if(!update)
        return;

    std::cout << "Checking for " << binaryName << "'s update ... ";
    std::cout.flush();

    // get the local version
    char command[100];
    sprintf(command, "%s -V", filePath.c_str());

    FILE* pipe = popen(command, "r");
    if (!pipe)
    {
        std::cout << "failed! (can not open pipe)" << endl;
        return;
    }

    char buffer[128];
    std::string result = "";
    while(!feof(pipe))
    {
        if(fgets(buffer, 128, pipe) != NULL)
            result += buffer;
    }
    pclose(pipe);

    // get the first line of the output
    boost::char_separator<char> sep("\n");
    boost::tokenizer< boost::char_separator<char> > tokens(result, sep);
    std::string firstLine;
    for(boost::tokenizer< boost::char_separator<char> >::iterator beg=tokens.begin(); beg!=tokens.end();++beg)
    {
        firstLine = (*beg).c_str();
        break;
    }

    std::size_t pos = firstLine.find("dev-SVN-");
    std::string localVer = firstLine.substr(pos);

    // now get the remote version
    int val = getRemoteVersion();

    if(val == -1)
        return;

    // get the first line of remoteVer
    boost::char_separator<char> sep2("\n");
    boost::tokenizer< boost::char_separator<char> > tokens2(remoteVer, sep2);
    for(boost::tokenizer< boost::char_separator<char> >::iterator beg=tokens2.begin(); beg!=tokens2.end();++beg)
    {
        remoteVer = (*beg).c_str();
        break;
    }

    if( localVer.compare(remoteVer) == 0 )
        std::cout << "Up to date!" << endl;
    else if( localVer.compare(remoteVer) < 0 )
    {
        std::cout << "\nYour current version is " << localVer << endl;
        std::cout << "Do you want to update to version " << remoteVer << " ? [y/n]: ";
        std::string answer;
        std::cin >> answer;
        boost::algorithm::to_lower(answer);

        if(answer == "y")
            downloadBinary(binaryName, filePath, url);
    }
}

} // namespace


