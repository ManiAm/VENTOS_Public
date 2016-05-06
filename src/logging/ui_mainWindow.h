#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui>

namespace VENTOS {

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    QVBoxLayout *verticalLayout;
    QTabWidget *tabWidget1;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");

        MainWindow->resize(400, 300);
        MainWindow->setAutoFillBackground(false);
        MainWindow->setStyleSheet(QString::fromUtf8(""));
        MainWindow->setUnifiedTitleAndToolBarOnMac(true);
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0));

        QIcon icon;
        icon.addFile(QString::fromUtf8("log_128.png"), QSize(), QIcon::Normal, QIcon::Off);
        MainWindow->setWindowIcon(icon);

        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName("centralWidget");
        MainWindow->setCentralWidget(centralWidget);

        verticalLayout = new QVBoxLayout(centralWidget);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName("verticalLayout");

        tabWidget1 = new QTabWidget(centralWidget);
        tabWidget1->setObjectName("tabWidget1");
        verticalLayout->addWidget(tabWidget1);

        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName("menuBar");
        menuBar->setGeometry(QRect(0, 0, 400, 25));
        MainWindow->setMenuBar(menuBar);

        mainToolBar = new QToolBar(MainWindow);
        mainToolBar->setObjectName("mainToolBar");
        MainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);

        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName("statusBar");
        MainWindow->setStatusBar(statusBar);

        QMetaObject::connectSlotsByName(MainWindow);
    }
};

}

#endif // UI_MAINWINDOW_H
