#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QTimer>
#include <QSystemTrayIcon>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QSystemTrayIcon ;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QString getserIP();
    QTimer *timer;


private slots:
    void on_actionDane_serwera_triggered();

    void on_pushButton_clicked();

    void on_textEditLog_textChanged();


    void on_pushButtonLog_clicked();

    void on_actionPlik_SynapticStock_mdb_triggered();

    void on_pushButtonStart_clicked();

    void on_actionStany_magazynowe_triggered();

    void on_actionWysy_ki_triggered();

    void on_actionPo_cz_z_Synaptic_triggered();

    void on_actionPo_cz_z_Access_triggered();

    void on_actionWczytaj_ustawienia_Synaptic_triggered();

    void on_actionWczytaj_ustawienia_Access_triggered();

    void on_pushButtonStop_clicked();

    void on_actionStany_magazynowe_6am_triggered();

    void on_actionWysy_ki_6am_triggered();

    void on_actionZamknij_program_triggered();

public slots:
    void update();
    void changeEvent(QEvent *event);
    void closeEvent(QCloseEvent *event);
    void restorewindow(QSystemTrayIcon::ActivationReason RW);

private:
    Ui::MainWindow *ui;
    bool wczytajustawienia();
    QString IPserweraSQL = "192.168.101.15";
    QString nazwabazy = "SynapticTest";
    QString login = "synaptic";
    QString haslo = "Nopass0";
    QString passwd = "VivaMedia";//do weryfikacji zmiany ustawien
    bool checkconnectiontosynaptic();
    QString lastconnerror = "Brak bledu";
    QString port = "2060";
    bool zapisdanychsynadopliku();
    QSqlDatabase synaptic;
    QSqlDatabase access;
    bool connecttosynaptic();
    QString datateraz();
    void log(QString input);
    QString filemdbname;
    bool conncetiondb = false;
    void setnofilesyna();
    void setnoconnecttodb();
    void setconnectedtodb();
    void setnofileaccess();
    void setnoconnecttomdb();
    void setconnectedtomdb();
    bool connecttomdb();
    bool zapisdanychmdbdopliku();
    bool wczytajustawieniamdb();
    int interval = 600000; //600000
    void setreplicationyes();
    void setreplicationno();
    QSystemTrayIcon *mSystemTrayIcon;
    bool disconnectsynaptic();
    bool disconnectaccess();
    QString godzinateraz();
    bool startrepl();
    bool stoprepl();

};
#endif // MAINWINDOW_H

