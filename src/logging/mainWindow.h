#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ui_mainWindow.h"
#include <sstream>

namespace VENTOS {

class mainWindow : public QMainWindow
{
    //Q_OBJECT

public:
    explicit mainWindow(QWidget *parent = 0);
    ~mainWindow();

    void addTab(std::string);
    void redirectStream(std::ostringstream *&, std::string);

private:
    Ui_MainWindow *ui;
};

}

#endif // MAINWINDOW_H
