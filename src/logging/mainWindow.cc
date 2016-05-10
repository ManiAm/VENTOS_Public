
#include "mainWindow.h"
#include <iostream>
#include "debugStream.h"

namespace VENTOS {

mainWindow::mainWindow() :
        m_VBox(Gtk::ORIENTATION_VERTICAL),
        m_Button_Quit("_Close", true)
{
    set_title("Log window");
    set_border_width(1);
    set_default_size(400, 200);
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
}


mainWindow::~mainWindow()
{

}


void mainWindow::on_button_quit()
{
    hide();
}


std::ostream* mainWindow::addTab(std::string category)
{
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
    m_refTextBuffer = Gtk::TextBuffer::create();
    m_refTextBuffer->set_text("");
    m_TextView->set_buffer(m_refTextBuffer);

    // re-direct stream to the m_refTextBuffer
    debugStream *buff = new debugStream(m_refTextBuffer);
    std::ostream *out = new std::ostream(buff);

    show_all_children();

    return out;
}

}
