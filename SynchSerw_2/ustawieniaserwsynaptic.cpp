#include "ustawieniaserwsynaptic.h"
#include "ui_ustawieniaserwsynaptic.h"
#include <QDebug>
#include <QString>

Ustawieniaserwsynaptic::Ustawieniaserwsynaptic(QWidget *parent,QString IPserwera,QString nazwabazy,QString nazwauzytkownika,QString haslo,QString port) :
    QDialog(parent),
    ui(new Ui::Ustawieniaserwsynaptic)
{
    ui->setupUi(this);
    ui->lineEditIPSerwera->setText(IPserwera);
    ui->lineEditBaza->setText(nazwabazy);
    ui->lineEditLogin->setText(nazwauzytkownika);
    ui->lineEditPassword->setText(haslo);
    ui->lineEditPort->setText(port);
}

Ustawieniaserwsynaptic::~Ustawieniaserwsynaptic()
{
    delete ui;
}

QString Ustawieniaserwsynaptic::getserwerIP()
{
    return ui->lineEditIPSerwera->text();
}

QString Ustawieniaserwsynaptic::getdbname()
{
    return ui->lineEditBaza->text();
}

QString Ustawieniaserwsynaptic::getlogin()
{
    return ui->lineEditLogin->text();
}

QString Ustawieniaserwsynaptic::getpasswd()
{
    return ui->lineEditPassword->text();
}

QString Ustawieniaserwsynaptic::getport()
{
    return ui->lineEditPort->text();
}
