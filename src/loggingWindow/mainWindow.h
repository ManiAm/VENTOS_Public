/****************************************************************************/
/// @file    mainWindow.h
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

#ifndef MAINWINDOW
#define MAINWINDOW

#include <gtkmm/window.h>
#include <gtkmm/button.h>
#include <gtkmm/box.h>
#include <gtkmm/notebook.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/textview.h>

//#include <gtkmm.h>

namespace VENTOS {

class mainWindow : public Gtk::Window
{

public:
    mainWindow();
    virtual ~mainWindow();

protected:
    // signal handler
    void on_button_quit();

private:
    void start_TCP_server();
    void listenToClient();
    void addTab(std::string);
    void writeStr(std::string, std::string &);
    void flushStr();

protected:
    std::map<std::string /*category name*/, std::ostream *> vLogStreams;
    int newsockfd;

    // widgets
    Gtk::Box m_VBox;
    Gtk::Notebook m_Notebook;
    Gtk::ButtonBox m_ButtonBox;
    Gtk::Button m_Button_Quit;
};

}

#endif
