/****************************************************************************/
/// @file    mainWindow.cc
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author  second author name
/// @date    May 2016
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

#include "mainWindow.h"
#include <iostream>
#include "debugStream.h"

#include <stdexcept>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace VENTOS {

mainWindow::mainWindow() : m_VBox(Gtk::ORIENTATION_VERTICAL),
        m_Button_Quit("_Close", true)
{
    set_title("Log window");
    set_border_width(1);
    set_default_size(550, 400 /*height*/);
    set_icon_from_file("log_128.png");

    add(m_VBox);

    // create the Notebook
    m_Notebook.set_border_width(1);

    // add notebook to VBox
    m_VBox.pack_start(m_Notebook);

    // add a quit button at the bottom
    m_VBox.pack_start(m_ButtonBox, Gtk::PACK_SHRINK);
    m_ButtonBox.pack_start(m_Button_Quit, Gtk::PACK_SHRINK);
    m_Button_Quit.signal_clicked().connect(sigc::mem_fun(*this, &mainWindow::on_button_quit) );

    show_all_children();

    start_TCP_server();

    std::thread thd = std::thread([=]() mutable {
        listenToClient();

        // we are done
        ::close(newsockfd);
    });

    thd.detach();
}


mainWindow::~mainWindow()
{

}


void mainWindow::on_button_quit()
{
    hide();
}


void mainWindow::start_TCP_server()
{
    std::cout << "    (logWindow) starting the TCP server. \n";
    std::cout.flush();

    // create a socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        throw std::runtime_error("ERROR opening socket");

    // clear address structure
    struct sockaddr_in serv_addr;
    bzero((char *) &serv_addr, sizeof(serv_addr));

    /* setup the host_addr structure for use in bind call */
    // server byte order
    serv_addr.sin_family = AF_INET;

    // automatically be filled with current host's IP address
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    // convert short integer value for port must be converted into network byte order
    serv_addr.sin_port = htons(45676);

    // This bind() call will bind  the socket to the current IP address on port, portno
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        throw std::runtime_error("ERROR on binding");

    // This listen() call tells the socket to listen to the incoming connections.
    listen(sockfd, 0 /*backlog*/);

    struct sockaddr_in cli_addr;
    socklen_t clilen = sizeof(cli_addr);

    // The accept() returns a new socket file descriptor for the accepted connection.
    // So, the original socket file descriptor can continue to be used
    // for accepting new connections while the new socker file descriptor is used for
    // communicating with the connected client.
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0)
        throw std::runtime_error("ERROR on accept");

    std::cout << "    (logWindow) incoming connection from " << inet_ntoa(cli_addr.sin_addr) << " port " << ntohs(cli_addr.sin_port) << ". \n";
    std::cout.flush();

    // close the welcoming socket
    ::close(sockfd);
}


void mainWindow::listenToClient()
{
    std::cout << "    (logWindow) waiting for requests ... \n\n";
    std::cout.flush();

    while(true)
    {
        char buffer[1000];
        bzero(buffer, 1000);

        int n = read(newsockfd, buffer, 999);
        if (n < 0)
            throw std::runtime_error("ERROR reading from socket");
        else if(n == 0)
            break;

        // tokenize the buffer
        std::string line (buffer);
        std::vector<std::string> strs;
        std::string delimiter = "||";
        size_t pos = 0;
        std::string token;
        while ((pos = line.find(delimiter)) != std::string::npos)
        {
            token = line.substr(0, pos);
            strs.push_back(token);
            line.erase(0, pos + delimiter.length());
        }
        strs.push_back(line);

        if(strs.size() <= 1)
        {
            send(newsockfd, "msg error!", 10, 0);
            throw std::runtime_error("Received msg is not formated correctly!");
        }

        if(strs[0] == "1")
            addTab(strs[1]);
        else if(strs[0] == "2")
            writeStr(strs[1], strs[2]);
        else if(strs[0] == "3")
            flushStr();
        else
            throw std::runtime_error("Invalid command number!");

        // send() function sends the 7 bytes of the string to the new socket
        send(newsockfd, "got it!", 7, 0);
    }
}


void mainWindow::addTab(std::string category)
{
    auto it = vLogStreams.find(category);
    if(it != vLogStreams.end())
        throw std::runtime_error("category already exists!");

    // create a ScrolledWindow
    Gtk::ScrolledWindow *m_ScrolledWindow = new Gtk::ScrolledWindow();
    // only show the scroll bars when they are necessary:
    m_ScrolledWindow->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

    // add the TextView inside ScrolledWindow
    Gtk::TextView *m_TextView = new Gtk::TextView();
    m_ScrolledWindow->add(*m_TextView);

    // create Notebook pages
    m_Notebook.append_page(*m_ScrolledWindow, category.c_str());

    // create a text buffer for text view
    Glib::RefPtr<Gtk::TextBuffer> m_refTextBuffer = Gtk::TextBuffer::create();
    m_refTextBuffer->set_text("");
    m_TextView->set_buffer(m_refTextBuffer);

    // re-direct stream to the m_refTextBuffer
    debugStream *buff = new debugStream(m_refTextBuffer);
    std::ostream *out = new std::ostream(buff);

    vLogStreams[category] = out;

    show_all_children();
}


void mainWindow::writeStr(std::string category, std::string &msg)
{
    auto it = vLogStreams.find(category);
    if(it == vLogStreams.end())
        throw std::runtime_error("category does not exist!");

    *(it->second) << msg;
}


void mainWindow::flushStr()
{
    for(auto &i : vLogStreams)
        (i.second)->flush();
}


}
