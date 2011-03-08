#include "addserver.h"
#include "ui_addserver.h"

AddServer::AddServer(QSettings* settings, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddServer)
{
  ui->setupUi(this);
  ui->lineEdit->setText(settings->value("Global/uTorrentIPs").toString().arg(""));
}

AddServer::~AddServer()
{
  delete ui;
}

QString AddServer::getIp()
{
  return ui->lineEdit->text();
}
