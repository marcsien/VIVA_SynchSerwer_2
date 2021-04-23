#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ustawieniaserwsynaptic.h"
#include <QDebug>
#include <QString>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlError>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QPixmap>
#include <QDateTime>
#include <QDesktopServices>
#include <QInputDialog>
#include <QFileDialog>
#include <qsqlquery.h>
#include <QSqlError>
#include <QThread>
#include <QSystemTrayIcon>
#include <QCloseEvent>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    log("Uruchomienie aplikacji");
    mSystemTrayIcon = new QSystemTrayIcon(this);
    mSystemTrayIcon->setVisible(true);
    mSystemTrayIcon->setIcon(QIcon(QPixmap((":/img/img/wp-reset-icon.png"))));
    //mSystemTrayIcon->set
    mSystemTrayIcon->connect(mSystemTrayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this,
                        SLOT(restorewindow(QSystemTrayIcon::ActivationReason)));

    if(!wczytajustawienia())
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Błąd!");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("Problem z wczytaniem danych konfiguracyjnych Synaptic, lub dane są nieprawidłowe");
        msgBox.setInformativeText("Sprawdź ustawienia : Admin -> Dane serwera Synaptic ");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
        setnofilesyna();
    }

    if(!wczytajustawieniamdb())
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Błąd!");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("Problem z wczytaniem danych konfiguracyjnych SynapticStock.mdb, lub dane są nieprawidłowe");
        msgBox.setInformativeText("Sprawdź ustawienia : Admin -> Plik SynapticStock.mdb ");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
        setnofileaccess();
    }

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()),
          this, SLOT(update()));
    timer->setInterval(interval);


    if(startrepl())
    {
        log("Replikacja rozpoczęta");
    }
    else
    {
        log("Replikacja zatrzymana");
    }
}

MainWindow::~MainWindow()
{
    synaptic.close();
    log("Zamknięcie aplikacji");
    delete ui;
}

void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::WindowStateChange)
    {
        if (isMinimized())
        {
            // it was minimised
            mSystemTrayIcon->showMessage(tr("Zminimalizowano"),tr("Aplikacja będzie pracowała w ukryciu, aby wyłączyć wybierz Admin -> Zamknij aplikację"));
            hide();
            event->ignore();
        }
        else
        {
            // it's normal or maximised
            show();
        }
    }

    return QMainWindow::changeEvent(event);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    log("Próba zamknięcia aplikacji");

    bool ok;
    QString text = QInputDialog::getText(this, tr("Podaj hasło"),
                                         tr("Podaj hasło:"), QLineEdit::Password,
                                         QDir::home().dirName(), &ok);
    if (ok && !text.isEmpty() && text == passwd)
    {
        event->accept();
    }
    else
    {
        event->ignore();
    }

}

void MainWindow::restorewindow(QSystemTrayIcon::ActivationReason RW)
{
    if(RW == QSystemTrayIcon::DoubleClick)
    {
        showMaximized();
        activateWindow();
        raise();
    }
}

QString MainWindow::getserIP()
{
    return IPserweraSQL;
}

bool MainWindow::wczytajustawienia()
{
    QFile file;
    file.setFileName("s!y@n#a$.set");
    QStringList settings;
    if(file.open(QIODevice::ReadOnly))
    {
        QTextStream in(&file);
        while (!in.atEnd()) {
              QString line = in.readLine();
              settings.append(line);
         }

        if(settings.count() == 5)
        {
            IPserweraSQL = settings.at(0);
            nazwabazy = settings.at(1);
            login = settings.at(2);
            haslo = settings.at(3);
            port = settings.at(4);
            log("Wczytano dane konfiguracyjne z pliku : " + QCoreApplication::applicationDirPath() + "/s!y@n#a$.set");

            if(connecttosynaptic())
            {
                file.close();
                disconnectsynaptic();
                return true;
            }
            else
            {
                file.close();
                return false;
            }
        }
    }
    else
    {
        log("Brak pliku konfiguracyjnego : " + QCoreApplication::applicationDirPath() + "/s!y@n#a$.set");
        return false;
    }
    return false;
}

bool MainWindow::checkconnectiontosynaptic()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC");

    db.setDatabaseName(QString("DRIVER={%1};SERVER=%2;"
                                  "DATABASE=%3;UID=%4;PWD=%5;"
                                  "Trusted_Connection=%6;"
                               "Port = %7;"
                               "Connection Timeout=2")
                          .arg("SQL SERVER")
                          .arg(IPserweraSQL)
                          .arg(nazwabazy)
                          .arg(login)
                          .arg(haslo)
                          .arg("No")
                          .arg(port));

    if (db.open())
    {
        lastconnerror = db.lastError().text();
        db.close();
        return true;
    }
    else
    {
        lastconnerror = db.lastError().text();
        db.close();
        return false;
    }


}

bool MainWindow::zapisdanychsynadopliku()
{
    QFile file;
    file.setFileName("s!y@n#a$.set");
    if(file.open(QIODevice::ReadWrite))
    {

        file.resize(0);
        QTextStream out(&file);
        out << IPserweraSQL << endl;
        out << nazwabazy << endl;
        out << login << endl;
        out << haslo << endl;
        out << port << endl;
        file.close();
        return true;

    }
    else
    {
        return false;
    }
}

bool MainWindow::connecttosynaptic()
{
    synaptic = QSqlDatabase::addDatabase("QODBC","polaczeniesynaptic");

    synaptic.setDatabaseName(QString("DRIVER={%1};SERVER=%2;"
                                     "DATABASE=%3;UID=%4;PWD=%5;"
                                     "Trusted_Connection=%6;"
                                     "Port = %7;"
                                     "Connection Timeout=2")
                                                            .arg("SQL SERVER")
                                                            .arg(IPserweraSQL)
                                                            .arg(nazwabazy)
                                                            .arg(login)
                                                            .arg(haslo)
                                                            .arg("No")
                                                            .arg(port));

    if (synaptic.open())
    {
        lastconnerror = synaptic.lastError().text();
        setconnectedtodb();
        log("Połączono z bazą danych");
        return true;

    }
    else
    {
        lastconnerror = synaptic.lastError().text();
        qDebug() << "nie polaczono db" << lastconnerror << QSqlDatabase::drivers();
        setnoconnecttodb();
        log("Nie połączono z bazą danych" + lastconnerror);
        return true;
    }
}

QString MainWindow::datateraz()
{
    QString data = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

    return data;
}

void MainWindow::log(QString input)
{
    QString tera = datateraz();
    ui->textEditLog->append(tera + " : " + input);
    QFile file;
    file.setFileName("log.txt");
    if(file.open(QIODevice::ReadWrite | QIODevice::Append))
    {
        QTextStream out(&file);
        out << tera + " : " + input << endl;
        file.close();
    }
    else
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Błąd!");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("Błąd zapisu logów do pliku.");
        msgBox.setInformativeText(file.errorString());
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
    }
}

void MainWindow::setnofilesyna()
{
    QPixmap pixmapTarget(":/img/img/nofilesyna.png");
    pixmapTarget = pixmapTarget.scaled(110, 110, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->labelSynapConnection->setPixmap(pixmapTarget);
    ui->labelSyna1->setText("Brak pliku konfiguracyjnego");
}

void MainWindow::setnoconnecttodb()
{
    QPixmap pixmapTarget(":/img/img/disconnected.png");
    pixmapTarget = pixmapTarget.scaled(110, 110, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->labelSynapConnection->setPixmap(pixmapTarget);
    ui->labelSyna1->setText("Brak połączenia z Synaptic");
}

void MainWindow::setconnectedtodb()
{
    QPixmap pixmapTarget(":/img/img/connected.png");
    pixmapTarget = pixmapTarget.scaled(110, 110, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->labelSynapConnection->setPixmap(pixmapTarget);
    ui->labelSyna1->setText("Połączony z Synaptic");
}

void MainWindow::setnofileaccess()
{
    QPixmap pixmapTarget(":/img/img/nofilesyna.png");
    pixmapTarget = pixmapTarget.scaled(110, 110, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->labelAccessMdb->setPixmap(pixmapTarget);
    ui->labelMdb2->setText("Brak pliku konfiguracyjnego");

}

void MainWindow::setnoconnecttomdb()
{
    QPixmap pixmapTarget(":/img/img/noconnectmdb.png");
    pixmapTarget = pixmapTarget.scaled(110, 110, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->labelAccessMdb->setPixmap(pixmapTarget);
    ui->labelMdb2->setText("Brak połączenia z MDB");
}

void MainWindow::setconnectedtomdb()
{
    QPixmap pixmapTarget(":/img/img/mdbok.png");
    pixmapTarget = pixmapTarget.scaled(110, 110, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->labelAccessMdb->setPixmap(pixmapTarget);
    ui->labelMdb2->setText("Połączony z MDB");
}

bool MainWindow::connecttomdb()
{
    access = QSqlDatabase::addDatabase("QODBC","polaczeniemdb");
    QString mdbname = QString("DRIVER={Microsoft Access Driver (*.mdb)};FIL={MS Access};DBQ=%1").arg(filemdbname);
    access.setDatabaseName(mdbname);

    if (access.open())
    {
        lastconnerror = access.lastError().text();
        setconnectedtomdb();
        log("Połączono z plikiem SynapticStock.mdb");
        conncetiondb = true;
        return true;

    }
    else
    {
        lastconnerror = access.lastError().text();
        qDebug() << "nie polaczono db" << lastconnerror << QSqlDatabase::drivers();
        setnoconnecttomdb();
        log("Nie połączono z plikiem SynapticStock.mdb" + lastconnerror);
        conncetiondb = false;
        return true;
    }
}

bool MainWindow::zapisdanychmdbdopliku()
{
    QFile file;
    file.setFileName("m!d@b#.set");
    if(file.open(QIODevice::ReadWrite))
    {
        file.resize(0);
        QTextStream out(&file);
        qDebug()<<filemdbname;
        out << filemdbname;
        file.close();
        return true;
    }
    else
    {
        return false;
    }
}

bool MainWindow::wczytajustawieniamdb()
{
    QFile file;
    file.setFileName("m!d@b#.set");
    QStringList settings;
    if(file.open(QIODevice::ReadOnly))
    {
        QTextStream in(&file);
        while (!in.atEnd()) {
              QString line = in.readLine();
              settings.append(line);
         }

        if(settings.count() == 1)
        {
            filemdbname = settings.at(0);

            log("Wczytano dane konfiguracyjne z pliku : " + QCoreApplication::applicationDirPath() + "/m!d@b#.set");

            if(connecttomdb())
            {
                file.close();
                disconnectaccess();
                return true;
            }
            else
            {
                file.close();
                return false;
            }
        }
    }
    else
    {
        log("Brak pliku konfiguracyjnego : " + QCoreApplication::applicationDirPath() + "/m!d@b#.set");
        return false;
    }
    return false;
}

void MainWindow::setreplicationyes()
{
    QPixmap pixmapTarget(":/img/img/yes.png");
    pixmapTarget = pixmapTarget.scaled(110, 110, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->labelSynchStatus->setPixmap(pixmapTarget);
    ui->labelState2->setText("Replikacja działa");
}

void MainWindow::setreplicationno()
{
    QPixmap pixmapTarget(":/img/img/no.png");
    pixmapTarget = pixmapTarget.scaled(110, 110, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->labelSynchStatus->setPixmap(pixmapTarget);
    ui->labelState2->setText("Replikacja zatrzymana");
}

bool MainWindow::disconnectsynaptic()
{
    synaptic.close();
    if (synaptic.isOpen())
    {
        setconnectedtodb();
        return false;
    }
    else
    {
        setnoconnecttodb();
        return true;
    }
}

bool MainWindow::disconnectaccess()
{
    access.close();
    if (access.isOpen())
    {
        setconnectedtomdb();
        return false;
    }
    else
    {
        setnoconnecttomdb();
        return true;
    }
}

QString MainWindow::godzinateraz()
{
    QString hour = QDateTime::currentDateTime().toString("HH");
    return hour;
}

bool MainWindow::startrepl()
{
    timer->start();
    if(timer->remainingTime() > 0)
    {
        setreplicationyes();
        return true;
    }
    else
    {
        return false;
    }
}

bool MainWindow::stoprepl()
{
    timer->stop();
    if(timer->remainingTime() < 0)
    {
        setreplicationno();
        return true;
    }
    else
    {
        return false;
    }
}


void MainWindow::update()
{

    if(godzinateraz() == "06")
    {
        on_actionStany_magazynowe_triggered();
        on_actionWysy_ki_triggered();
        on_actionWysy_ki_6am_triggered();
        on_actionStany_magazynowe_6am_triggered();
    }
    else
    {
        on_actionStany_magazynowe_triggered();
        on_actionWysy_ki_triggered();
    }
}


void MainWindow::on_actionDane_serwera_triggered()
{

    log("Próba zmiany danych serwera Synaptic");

    bool ok;
    QString text = QInputDialog::getText(this, tr("Podaj hasło"),
                                         tr("Podaj hasło:"), QLineEdit::Password,
                                         QDir::home().dirName(), &ok);
    if (ok && !text.isEmpty() && text == passwd)
    {
        log("Autoryzacja pomyślna");
        Ustawieniaserwsynaptic * ust = new Ustawieniaserwsynaptic(this,IPserweraSQL,nazwabazy,login,haslo,port);
        if(ust->exec()==QDialog::Accepted)
        {
            IPserweraSQL = ust->getserwerIP();
            nazwabazy = ust->getdbname();
            login = ust->getlogin();
            haslo = ust->getpasswd();
            port = ust->getport();
            if(connecttosynaptic())
            {
                QMessageBox msgBox;
                msgBox.setWindowTitle("Sukces!");
                msgBox.setIcon(QMessageBox::Information);
                msgBox.setText("Połączenie z serwerem udane.");
                msgBox.setInformativeText("Dane serwera zostały zmienione");
                msgBox.setStandardButtons(QMessageBox::Ok);
                msgBox.exec();
                zapisdanychsynadopliku();
                log("Zmieniono dane serwera Synaptic");
            }
            else
            {
                QMessageBox msgBox;
                msgBox.setWindowTitle("Niepowodzenie!");
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.setText("Problem z połączeniem.");
                msgBox.setInformativeText(lastconnerror);
                msgBox.setStandardButtons(QMessageBox::Ok);
                msgBox.exec();
                log("Dane serwera Synaptic nieprawidłowe");
                on_actionDane_serwera_triggered();
            }
            disconnectsynaptic();

        }
        else
        {
            QMessageBox msgBox;
            msgBox.setWindowTitle("Anulowano!");
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText("Anulowane przez użytkownika.");
            msgBox.setInformativeText("Dane serwera nie zostały zmienione");
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.exec();
            log("Anulowano zmianę danych serwera Synaptic");
        }
    }
    else
    {
        log("Autoryzacja nie powiodła się");
    }


}

void MainWindow::on_pushButton_clicked()
{
    ui->textEditLog->clear();
}

void MainWindow::on_textEditLog_textChanged()
{
    ui->textEditLog->moveCursor(QTextCursor::End);
}


void MainWindow::on_pushButtonLog_clicked()
{
    //QDesktopServices::openUrl(QUrl("file:///C:/Documents and Settings/All Users/Desktop", QUrl::TolerantMode));
    QDesktopServices::openUrl(QUrl(QCoreApplication::applicationDirPath() + "/log.txt"));
}

void MainWindow::on_actionPlik_SynapticStock_mdb_triggered()
{
    log("Próba zmiany ścieżki dostępu do pliku SynapticStock.mdb");

    bool ok;
    QString text = QInputDialog::getText(this, tr("Podaj hasło"),
                                         tr("Podaj hasło:"), QLineEdit::Password,
                                         QDir::home().dirName(), &ok);
    if (ok && !text.isEmpty() && text == passwd)
    {
        log("Autoryzacja pomyślna");
        QFileDialog dlg(this, tr("Otwórz plik SynapticStock.mdb"));
        dlg.setAcceptMode(QFileDialog::AcceptOpen);
        if (dlg.exec())
        {
            filemdbname = dlg.selectedFiles().at(0);
            if(connecttomdb())
            {
                QMessageBox msgBox;
                msgBox.setWindowTitle("Sukces!");
                msgBox.setIcon(QMessageBox::Information);
                msgBox.setText("Plik SynapticStock.mdb prawidłowo wczytany");
                msgBox.setInformativeText("Ścieżka dostępu do SynapticStock.mdb zmieniona");
                msgBox.setStandardButtons(QMessageBox::Ok);
                msgBox.exec();
                zapisdanychmdbdopliku();
                log("Zmieniono ścieżkę dostępu do pliku SynapticStock.mdb");
            }
            else
            {
                QMessageBox msgBox;
                msgBox.setWindowTitle("Niepowodzenie!");
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.setText("Problem z wczytaniem pliku SynapticStock.mdb");
                msgBox.setInformativeText("");
                msgBox.setStandardButtons(QMessageBox::Ok);
                msgBox.exec();
                log("Ścieżka dostępu do pliku SynapticStock.mdb niepoprawna");
                on_actionPlik_SynapticStock_mdb_triggered();
            }
            disconnectaccess();

        }
        else
        {
            log("Anulowano zmianę ścieżki dostępu do pliku SynapticStock.mdb");
        }
    }
    else
    {
        log("Autoryzacja nie powiodła się");
    }
}

void MainWindow::on_pushButtonStart_clicked()
{
    if(startrepl())
    {
        log("Replikacja rozpoczęta");
    }
    else
    {
        log("Uruchomienie replikacji nie powiodło się");
    }
}

void MainWindow::on_actionStany_magazynowe_triggered()
{
    //connecttosynaptic();
    QApplication::setOverrideCursor(Qt::WaitCursor);
    if(connecttosynaptic())
    {
        if(connecttomdb())
        {
            QSqlQuery query(access);
            QString querytext = "delete * from dbo_Wyrg";
            query.prepare(querytext);
            if(query.exec())
            {
                //log("Wykonano pomyślnie zapytanie: " + querytext);
                QSqlQuery query1(synaptic);
                QString query1text = "select * from Wyrg";
                query1.prepare(query1text);
                qApp->processEvents();
                if(query1.exec())
                {
                    while (query1.next())
                    {
                        qApp->processEvents();
                        QString tmp = query1.value(0).toString();
                        QSqlQuery * query2 = new QSqlQuery(access);
                        query2->prepare(QString("insert into dbo_Wyrg (JSName,ProductCount,VivaId) values ('%1','%2','%3')")
                                       .arg(query1.value(0).toString())
                                       .arg(query1.value(1).toString())
                                       .arg(query1.value(2).toString()));
                        if(!query2->exec())
                        {
                            log("Bład wykonywania zapytania " + QString("insert into dbo_Wyrg (JSName,ProductCount,VivaId) values ('%1','%2','%3')")
                                .arg(query1.value(0).toString())
                                .arg(query1.value(1).toString())
                                .arg(query1.value(2).toString()) +" Treść błędu: "+ query2->lastError().text());
                        }
                        delete query2;

                    }
                    log("Stany magazynowe zaktualizowane");
                }
                else
                {
                    log("Błąd wykonywania zapytania " + query1text + " opis błędu: " + query1.lastError().text());
                }

            }
            else
            {
                log("Błąd wykonywania zapytania " + querytext + " opis błędu: " + query.lastError().text());
            }

            disconnectaccess();
        }
        else
        {
            log("Brak połączenia z Access, Stany magazynowe nie zaktualizowane");
            setnoconnecttomdb();
        }

        disconnectsynaptic();
    }
    else
    {
        log("Brak połączenia z Synaptic, Stany magazynowe nie zaktualizowane");
        setnoconnecttodb();
    }
    QApplication::restoreOverrideCursor();
}

void MainWindow::on_actionWysy_ki_triggered()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    if(connecttosynaptic())
    {
        if(connecttomdb())
        {
            QSqlQuery query(access);
            QString querytext = "delete * from dbo_WyrobyGotoweWZ";
            query.prepare(querytext);
            if(query.exec())
            {
                //log("Wykonano pomyślnie zapytanie: " + querytext);
                QSqlQuery query1(synaptic);
                QString query1text = "select * from WyrobyGotoweWZ";
                query1.prepare(query1text);
                qApp->processEvents();
                if(query1.exec())
                {
                    while (query1.next())
                    {
                        qApp->processEvents();
                        QString tmp = query1.value(0).toString();
                        QSqlQuery * query2 = new QSqlQuery(access);
                        query2->prepare(QString("insert into dbo_WyrobyGotoweWZ (JSName,OutCount,VivaId) values ('%1','%2','%3')")
                                       .arg(query1.value(0).toString())
                                       .arg(query1.value(1).toString())
                                       .arg(query1.value(2).toString()));
                        if(!query2->exec())
                        {
                            log("Bład wykonywania zapytania " +

                                QString("insert into dbo_WyrobyGotoweWZ (JSName,OutCount,VivaId) values ('%1','%2','%3')")
                                .arg(query1.value(0).toString())
                                .arg(query1.value(1).toString())
                                .arg(query1.value(2).toString())

                                +" Treść błędu: "+ query2->lastError().text());
                        }
                        delete query2;

                    }
                    log("Wysyłki zaktualizowane");
                }
                else
                {
                    log("Błąd wykonywania zapytania " + query1text + " opis błędu: " + query1.lastError().text());
                }

            }
            else
            {
                log("Błąd wykonywania zapytania " + querytext + " opis błędu: " + query.lastError().text());
            }
            disconnectaccess();
        }
        else
        {
            log("Brak połączenia z Access, Wysyłki nie zaktualizowane");
            setnoconnecttomdb();
        }
        disconnectsynaptic();
    }
    else
    {
        log("Brak połączenia z Synaptic, Wysyłki nie zaktualizowane");
        setnoconnecttodb();
    }
    QApplication::restoreOverrideCursor();
}

void MainWindow::on_actionPo_cz_z_Synaptic_triggered()
{

    if(synaptic.open())
    {
        disconnectsynaptic();
        log("Połączenie z Synaptic działa");
    }
    else
    {
        log("Połączenie z Synaptic nie udane");
    }
}

void MainWindow::on_actionPo_cz_z_Access_triggered()
{
    if(access.open())
    {
        disconnectaccess();
        log("Połączenie z Access działa");
    }
    else
    {
        log("Połączenie z Access nie udane");
    }
}

void MainWindow::on_actionWczytaj_ustawienia_Synaptic_triggered()
{
    wczytajustawienia();
}

void MainWindow::on_actionWczytaj_ustawienia_Access_triggered()
{
    wczytajustawieniamdb();
}

void MainWindow::on_pushButtonStop_clicked()
{
    if(stoprepl())
    {
         log("Replikacja zakończona");
    }
    else
    {
        log("Zatrzymanie replikacji nie powiodło się");
    }
}

void MainWindow::on_actionStany_magazynowe_6am_triggered()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    if(connecttosynaptic())
    {
        if(connecttomdb())
        {
            QSqlQuery query(access);
            QString querytext = "delete * from dbo_Wyrg6am";
            query.prepare(querytext);
            if(query.exec())
            {
                //log("Wykonano pomyślnie zapytanie: " + querytext);
                QSqlQuery query1(synaptic);
                QString query1text = "select * from Wyrg";
                query1.prepare(query1text);
                qApp->processEvents();
                if(query1.exec())
                {
                    while (query1.next())
                    {
                        qApp->processEvents();
                        QString tmp = query1.value(0).toString();
                        QSqlQuery * query2 = new QSqlQuery(access);
                        query2->prepare(QString("insert into dbo_Wyrg6am (JSName,ProductCount,VivaId) values ('%1','%2','%3')")
                                       .arg(query1.value(0).toString())
                                       .arg(query1.value(1).toString())
                                       .arg(query1.value(2).toString()));
                        if(!query2->exec())
                        {
                            log("Bład wykonywania zapytania " + QString("insert into dbo_Wyrg6am (JSName,ProductCount,VivaId) values ('%1','%2','%3')")
                                .arg(query1.value(0).toString())
                                .arg(query1.value(1).toString())
                                .arg(query1.value(2).toString()) +" Treść błędu: "+ query2->lastError().text());
                        }
                        delete query2;

                    }
                    log("Stany magazynowe zaktualizowane na 6am");
                }
                else
                {
                    log("Błąd wykonywania zapytania " + query1text + " opis błędu: " + query1.lastError().text());
                }

            }
            else
            {
                log("Błąd wykonywania zapytania " + querytext + " opis błędu: " + query.lastError().text());
            }
            disconnectaccess();
        }
        else
        {
            log("Brak połączenia z Access, Stany magazynowe 6am nie zaktualizowane");
            setnoconnecttomdb();
        }
        disconnectsynaptic();
    }
    else
    {
        log("Brak połączenia z Synaptic, Stany magazynowe 6am nie zaktualizowane");
        setnoconnecttodb();
    }
    QApplication::restoreOverrideCursor();
}

void MainWindow::on_actionWysy_ki_6am_triggered()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    if(connecttosynaptic())
    {
        if(connecttomdb())
        {
            QSqlQuery query(access);
            QString querytext = "delete * from dbo_WyrobyGotoweWZ6am";
            query.prepare(querytext);
            if(query.exec())
            {
                //log("Wykonano pomyślnie zapytanie: " + querytext);
                QSqlQuery query1(synaptic);
                QString query1text = "select * from WyrobyGotoweWZ";
                query1.prepare(query1text);
                qApp->processEvents();
                if(query1.exec())
                {
                    while (query1.next())
                    {
                        qApp->processEvents();
                        QString tmp = query1.value(0).toString();
                        QSqlQuery * query2 = new QSqlQuery(access);
                        query2->prepare(QString("insert into dbo_WyrobyGotoweWZ6am (JSName,OutCount,VivaId) values ('%1','%2','%3')")
                                       .arg(query1.value(0).toString())
                                       .arg(query1.value(1).toString())
                                       .arg(query1.value(2).toString()));
                        if(!query2->exec())
                        {
                            log("Bład wykonywania zapytania " +

                                QString("insert into dbo_WyrobyGotoweWZ6am (JSName,OutCount,VivaId) values ('%1','%2','%3')")
                                .arg(query1.value(0).toString())
                                .arg(query1.value(1).toString())
                                .arg(query1.value(2).toString())

                                +" Treść błędu: "+ query2->lastError().text());
                        }
                        delete query2;
                        //stad
                    }
                    log("Wysyłki zaktualizowane na 6am");
                }
                else
                {
                    log("Błąd wykonywania zapytania " + query1text + " opis błędu: " + query1.lastError().text());
                }

            }
            else
            {
                log("Błąd wykonywania zapytania " + querytext + " opis błędu: " + query.lastError().text());
            }
            disconnectaccess();
        }
        else
        {
            log("Brak połączenia z Access, Wysyłki 6am nie zaktualizowane");
            setnoconnecttomdb();
        }
        disconnectsynaptic();
    }
    else
    {
        log("Brak połączenia z Synaptic, Wysyłki 6am nie zaktualizowane");
        setnoconnecttodb();
    }
    QApplication::restoreOverrideCursor();
}

void MainWindow::on_actionZamknij_program_triggered()
{
    this -> close();
}
