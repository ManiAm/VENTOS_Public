#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ui_mainWindow.h"

namespace VENTOS {

class mainWindow : public QMainWindow
{
    //Q_OBJECT

public:
    explicit mainWindow(QWidget *parent = 0);
    ~mainWindow();

private:
    Ui_MainWindow *ui;
};

}

#endif // MAINWINDOW_H
