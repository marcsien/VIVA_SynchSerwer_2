#ifndef USTAWIENIASERWSYNAPTIC_H
#define USTAWIENIASERWSYNAPTIC_H

#include <QDialog>

namespace Ui {
class Ustawieniaserwsynaptic;
}

class Ustawieniaserwsynaptic : public QDialog
{
    Q_OBJECT

public:
    explicit Ustawieniaserwsynaptic(QWidget *parent = nullptr,QString nazwaserwera = nullptr,QString nazwabazy = nullptr,QString nazwauzytkownika = nullptr,QString haslo = nullptr, QString port = nullptr);
    ~Ustawieniaserwsynaptic();
    QString getserwerIP();
    QString getdbname();
    QString getlogin();
    QString getpasswd();
    QString getport();

private:
    Ui::Ustawieniaserwsynaptic *ui;
};

#endif // USTAWIENIASERWSYNAPTIC_H
