
#include "mainWindow.h"

int main(int argc, char* argv[])
{
    auto app = Gtk::Application::create(argc, argv, "org.gtkmm.example");

    VENTOS::mainWindow logWindow;

    // Shows the window and returns when it is closed.
    return app->run(logWindow, argc, argv);
}
