
#include "mainWindow.h"
#include <debugStream.h>
#include <cassert>

namespace VENTOS {

mainWindow::mainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui_MainWindow)
{
    ui->setupUi(this);
}


mainWindow::~mainWindow()
{
    delete ui;
}


void mainWindow::addTab(std::string category)
{
    // find the 'tab widget' in the Qt window
    QTabWidget * tabWidget1 = this->findChild<QTabWidget *>("tabWidget1");
    assert(tabWidget1);

    // add a new tab
    QWidget *tab = new QWidget();
    tab->setObjectName(category.c_str());
    tabWidget1->addTab(tab, QString());
    tabWidget1->setTabText(tabWidget1->indexOf(tab), QApplication::translate("MainWindow", category.c_str(), 0));

    // set the layout of tab to vertical
    QVBoxLayout *verticalLayout = new QVBoxLayout(tab);
    verticalLayout->setSpacing(6);
    verticalLayout->setContentsMargins(11, 11, 11, 11);
    verticalLayout->setObjectName("verticalLayout");

    // add a new 'text edit' inside the tab
    QTextEdit *textEdit = new QTextEdit(tab);
    textEdit->setObjectName(category.c_str());
    textEdit->setAcceptDrops(false);
    textEdit->setReadOnly(true);
    verticalLayout->addWidget(textEdit);

    //    tabWidget11->setCurrentIndex(1);
}


void mainWindow::redirectStream(std::ostringstream *&stream, std::string category)
{
    // find the 'text edit' in the Qt window
    QTextEdit * textEdit = this->findChild<QTextEdit *>(category.c_str());
    assert(textEdit);

    // re-direct stream to textEdit
    QDebugStream qout(*stream, textEdit);
}

}
