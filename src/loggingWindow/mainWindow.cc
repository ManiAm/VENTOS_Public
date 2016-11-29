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
#include "thread"
#include "debugStream.h"

#include <stdexcept>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "boost/filesystem.hpp"

namespace VENTOS {

mainWindow::mainWindow(std::string filePath)
{
    boost::filesystem::path full_path(filePath);
    std::string logoPath = (full_path.parent_path() / "log_128.png").string();

    set_title("Log window");
    set_border_width(1);
    set_default_size(1200, 550 /*height*/);
    set_icon_from_file(logoPath.c_str());

    Gtk::Box *m_VBox = new Gtk::Box(Gtk::ORIENTATION_VERTICAL);
    add(*m_VBox);

    // create the Notebook and add it to m_VBox
    m_Notebook = new Gtk::Notebook();
    m_Notebook->set_border_width(1);
    m_VBox->pack_start(*m_Notebook);

    // create a ButtonBox and add it to m_VBox
    Gtk::ButtonBox *m_ButtonBox = new Gtk::ButtonBox();
    m_VBox->pack_start(*m_ButtonBox, Gtk::PACK_SHRINK);

    // create a quit button and add it to m_ButtonBox
    Gtk::Button *m_Button_Quit = new Gtk::Button("_Close", true);
    m_ButtonBox->pack_start(*m_Button_Quit, Gtk::PACK_SHRINK);

    // emit signal on clicking the button
    m_Button_Quit->signal_clicked().connect(sigc::mem_fun(*this, &mainWindow::on_button_quit) );

    // create a dispatcher and connect it to processCMD
    m_Dispatcher = new Glib::Dispatcher();
    m_Dispatcher->connect(sigc::mem_fun(*this, &mainWindow::processCMD));

    show_all_children();

    start_TCP_server();

    std::thread thd(&mainWindow::listenToClient, this, this);
    thd.detach();
}


mainWindow::~mainWindow()
{
    if(newsockfd)
        ::close(newsockfd);
}


void mainWindow::start_TCP_server()
{
    try
    {
        std::cout << "    (logWindow) starting the TCP server. \n";
        std::cout.flush();

        // create a socket
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0)
            throw std::runtime_error("ERROR opening socket");

        // the purpose of SO_REUSEADDR/SO_REUSEPORT is to allow to reuse the port even if the process crash or been killed
        int optval = 1;
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));

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
        // only one client (VENTOS) is allowed to connect to this TCP server
        ::close(sockfd);
    }
    catch(const std::exception& ex)
    {
        // silently ignore the exceptions
        //std::cout << std::endl << ex.what() << std::endl;
        //std::cout.flush();

        return;
    }
}


void mainWindow::listenToClient(mainWindow *windowPtr)
{
    std::cout << "    (logWindow) waiting for log messages from VENTOS ... \n";
    std::cout.flush();

    try
    {
        while(true)
        {
            // receive the message length
            uint32_t rcvDataLength = 0;
            int n = ::recv(newsockfd, &rcvDataLength, sizeof(uint32_t), MSG_NOSIGNAL);
            if (n < 0)
                throw std::runtime_error("ERROR reading msg size from socket");
            else if(n == 0)
                break;

            rcvDataLength = ntohl(rcvDataLength);

            char rx_buffer[rcvDataLength+1];
            bzero(rx_buffer, rcvDataLength+1);

            n = ::recv(newsockfd, rx_buffer, rcvDataLength, MSG_NOSIGNAL);
            if (n < 0)
                throw std::runtime_error("ERROR reading msg from socket");
            else if(n == 0)
                break;

            {
                std::unique_lock<std::mutex> lck(mtx);

                // updating the received command string
                rx_cmd = std::string(rx_buffer);

                // call the dispatcher in mainWindow -- this will call 'processCMD' method
                windowPtr->m_Dispatcher->emit();

                // wait here for mainWindow to notify us
                cv.wait(lck);
            }

            if(response == "")
                throw std::runtime_error("response msg is empty!");

            // sending the response
            n = ::send(newsockfd, response.c_str(), response.size(), MSG_NOSIGNAL);
            if (n < 0)
                throw std::runtime_error("ERROR sending response to socket");

            response = "";  // reset response
        }

        // we are done
        ::close(newsockfd);
    }
    catch(const std::exception& ex)
    {
        // silently ignore the exceptions
        //std::cout << std::endl << ex.what() << std::endl;
        //std::cout.flush();

        return;
    }
}


void mainWindow::processCMD()
{
    // check answer by DanielKO: http://stackoverflow.com/questions/19142368/what-happens-if-i-call-wait-on-a-notified-condition-variable
    std::lock_guard<std::mutex> lck(mtx);

    try
    {
        if(rx_cmd == "")
            throw std::runtime_error("Received msg is empty!");

        // tokenize the buffer
        std::vector<std::string> strs;
        std::string delimiter = "||";
        size_t pos = 0;
        std::string token;
        while ((pos = rx_cmd.find(delimiter)) != std::string::npos)
        {
            token = rx_cmd.substr(0, pos);
            strs.push_back(token);
            rx_cmd.erase(0, pos + delimiter.length());
        }
        strs.push_back(rx_cmd);

        if(strs.size() <= 1)
            throw std::runtime_error("Received msg is not formated correctly!");

        if(strs[0] == std::to_string(CMD_ADD_TAB))
            addTab(strs[1]);
        else if(strs[0] == std::to_string(CMD_ADD_SUB_TEXTVIEW))
            addSubTextView(strs[1], strs[2]);
        else if(strs[0] == std::to_string(CMD_INSERT_TXT))
            writeStr(strs[1], strs[2], strs[3]);
        else if(strs[0] == std::to_string(CMD_FLUSH))
            flushStr(strs[1], strs[2]);
        else
            throw std::runtime_error("Invalid command number!");
    }
    catch(const std::exception& ex)
    {
        response = ex.what();

        // notify listenToClient thread to proceed
        cv.notify_one();

        return;
    }

    response = "ok!";

    // notify listenToClient thread to proceed
    cv.notify_one();
}


void mainWindow::addTab(std::string category)
{
    auto it = vLogStreams.find(std::make_pair(category, "default"));
    if(it != vLogStreams.end())
        throw std::runtime_error("addTab: category/subcategory pair already exists!");

    // creating a horizontal box and add it to notebook
    Gtk::Box *m_VBox_tx = new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 10);
    m_VBox_tx->set_homogeneous(true);
    m_Notebook->append_page(*m_VBox_tx, category.c_str());

    // save m_VBox_tx for later access
    notebookBox[category] = m_VBox_tx;

    // create a new textview and add it to the box
    Gtk::ScrolledWindow *m_ScrolledWindow = createTextView(category, "default");
    m_VBox_tx->pack_start(*m_ScrolledWindow, Gtk::PACK_EXPAND_WIDGET);

    show_all_children();
}


void mainWindow::addSubTextView(std::string category, std::string subcategory)
{
    auto it = vLogStreams.find(std::make_pair(category, subcategory));
    if(it != vLogStreams.end())
        throw std::runtime_error("addSubTextView: category/subcategory pair already exists!");

    // find the horizontal box
    auto itt = notebookBox.find(category);
    if(itt == notebookBox.end())
        throw std::runtime_error("cannot find the box object!");

    // create a new textview and add it to the box
    Gtk::ScrolledWindow *m_ScrolledWindow = createTextView(category, subcategory);
    (itt->second)->pack_start(*m_ScrolledWindow, Gtk::PACK_EXPAND_WIDGET);

    show_all_children();
}


Gtk::ScrolledWindow * mainWindow::createTextView(std::string category, std::string subcategory)
{
    // creating a ScrolledWindow
    Gtk::ScrolledWindow *m_ScrolledWindow = new Gtk::ScrolledWindow();
    // only show the scroll bars when they are necessary:
    m_ScrolledWindow->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    // add shadow to the border
    m_ScrolledWindow->set_shadow_type(Gtk::SHADOW_ETCHED_IN);

    // add the TextView inside ScrolledWindow
    Gtk::TextView *m_TextView = new Gtk::TextView();

    // change default font throughout the m_TextView
    Pango::FontDescription fdesc;
    fdesc.set_family("monospace");
    fdesc.set_size(10 * PANGO_SCALE);
    m_TextView->override_font(fdesc);

    // make the text view not editable
    m_TextView->set_editable(false);

    m_ScrolledWindow->add(*m_TextView);

    // create a text buffer mark to scroll the last inserted line into view
    Glib::RefPtr<Gtk::TextBuffer> m_refTextBuffer = m_TextView->get_buffer();
    m_refTextBuffer->create_mark("last_line", m_refTextBuffer->end(), /* left_gravity= */ true);

    // making my own stream buffer
    debugStream *buff = new debugStream(m_TextView);
    // re-direct ostream to my own stream buffer
    std::ostream *out = new std::ostream(buff);

    // save the associated stream
    vLogStreams[std::make_pair(category, subcategory)] = out;

    return m_ScrolledWindow;
}


void mainWindow::writeStr(std::string category, std::string subcategory, std::string &msg)
{
    auto it = vLogStreams.find(std::make_pair(category, subcategory));
    if(it == vLogStreams.end())
        throw std::runtime_error("writeStr: category/subcategory pair does not exist!");

    *(it->second) << msg;
}


void mainWindow::flushStr(std::string category, std::string subcategory)
{
    auto it = vLogStreams.find(std::make_pair(category, subcategory));
    if(it == vLogStreams.end())
        throw std::runtime_error("writeStr: category/subcategory pair does not exist!");

    (it->second)->flush();
}


void mainWindow::on_button_quit()
{
    hide();
}

}
