
#ifndef MAINWINDOW
#define MAINWINDOW

#include <gtkmm/window.h>
#include <gtkmm/button.h>
#include <gtkmm/box.h>
#include <gtkmm/notebook.h>
#include <gtkmm/label.h>
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

    std::ostream* addTab(std::string);

protected:
    //Signal handlers:
    void on_button_quit();

protected:
    Glib::RefPtr<Gtk::TextBuffer> m_refTextBuffer;

    // widgets
    Gtk::Box m_VBox;
    Gtk::Notebook m_Notebook;
    Gtk::ButtonBox m_ButtonBox;
    Gtk::Button m_Button_Quit;
};

}

#endif
