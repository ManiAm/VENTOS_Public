
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
