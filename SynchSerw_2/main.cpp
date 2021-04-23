#include "mainwindow.h"

#include <QApplication>
#include <QTimer>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowTitle("SynchSerw_2");
    w.setWindowIcon(QIcon(QPixmap((":/img/img/wp-reset-icon.png"))));
    w.show();
    return a.exec();
}
