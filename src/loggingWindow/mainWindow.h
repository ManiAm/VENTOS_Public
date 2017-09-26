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

#include <mutex>              // std::mutex, std::unique_lock
#include <condition_variable> // std::condition_variable

#include <gtkmm/window.h>
#include <gtkmm/button.h>
#include <gtkmm/box.h>
#include <gtkmm/notebook.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/textview.h>
#include <glibmm/dispatcher.h>

//#include <gtkmm.h>

namespace VENTOS {

enum logWindowCMD
{
    CMD_SYNTAX_HIGHLIGHTING,
    CMD_ADD_TAB,
    CMD_ADD_SUB_TEXTVIEW,
    CMD_INSERT_TXT,
    CMD_FLUSH,
};

class mainWindow : public Gtk::Window
{
protected:
    // widgets on main window
    Gtk::Box *m_VBox = NULL;
    Gtk::Notebook *m_Notebook = NULL;
    Gtk::ButtonBox *m_ButtonBox = NULL;
    Gtk::Button *m_Button_Quit = NULL;
    Glib::Dispatcher *m_Dispatcher = NULL;

    std::mutex mtx;
    std::condition_variable cv;
    std::string response = "";
    std::string syntaxHighlighting = "";

    std::map< std::pair<std::string /*tab*/, std::string /*pane*/>, std::ostream *> vLogStreams;
    std::map< std::string, Gtk::Box *> notebookBox;
    int newsockfd = -1;
    std::string rx_cmd = "";

    bool debugActive = 0;

public:
    mainWindow(std::string filePath, std::string title, uint8_t systemLogLevel);
    virtual ~mainWindow();

protected:
    // signal handler
    void on_button_quit();

private:
    void start_TCP_server();
    void listenToClient(mainWindow *);

    void processCMD();
    void addTab(std::string, std::string);
    Gtk::Label * addTextFormatting(std::string);
    Gtk::ScrolledWindow * createTextView(std::string, std::string);
    void addSubTextView(std::string, std::string);
    void writeStr(std::string, std::string, std::string &);
    void flushStr(std::string, std::string);

    void freeResources();
};

}

#endif
