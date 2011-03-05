#include "bootloader.h"
#include "ui_bootloader.h"
#include "qDebug.h"
#include <QFile>
#include <QDir>
#include <QTimer>

BootLoader::BootLoader(QSettings* set, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BootLoader)
{
    ui->setupUi(this);
    settings = set;
}

BootLoader::~BootLoader()
{
    delete ui;
}

void BootLoader::processExisting()
{
  QEventLoop loop;
  QTimer timer;
  int ip = sender()->property("ip").toInt();

  if (ip != 0 && httpBuf[ip]->data().contains(settings->value("Global/MasterKeyword").toByteArray()))
  {
    qDebug() << "got sgt from " << ip;
    QStringList sl = QString(httpBuf[ip]->data()).split("\r\n");
    sl.removeAll("");
    sl.removeAll(settings->value("Global/MasterKeyword").toString());
    QHttp* http;
    nbmac++;
    ui->nbmac->setText(QVariant(nbmac).toString());
    httpBuf[ip]->close();
    foreach (QString cur, sl)
    {
      http = new QHttp();
      http->setHost(settings->value("Global/uTorrentIPs").toString().arg(ip), 80);
      QObject::connect(http, SIGNAL(done(bool)), &loop, SLOT(quit()));
      nbfil++;
      ui->nbfil->setText(QVariant(nbfil).toString());
      ui->inProgress->setText(cur);
      QFile file;
      file.setFileName(settings->value("Global/Database").toString() + "/" + cur);
      file.open(QIODevice::WriteOnly);
      http->get("/" + cur, &file);
      loop.exec();
      timer.stop();
      if (file.pos() == 0) //FIXME
      {
        qDebug() << "empty... redl...";
        sl << cur;
      }
      file.close();
    }
  }
  ui->progressBar->setValue(1+ui->progressBar->value());
  if (ui->progressBar->value() == 254)
    QTimer::singleShot(33, this, SLOT(close()));
}

void BootLoader::process()
{
  nbfil = nbmac = 0;
  QStringList toremove;
  if (settings->value("Global/CleanDatabase").toBool())
  {
    qDebug() << QCoreApplication::applicationFilePath()+ "/" + settings->value("Global/Database").toString();
    QStringList fl = QDir(QCoreApplication::applicationDirPath()+ "/" + settings->value("Global/Database").toString()).entryList();
    foreach(QString f, fl)
    {
      qDebug() << "removing" << f;
      QFile::remove(QCoreApplication::applicationDirPath()+ "/" + settings->value("Global/Database").toString() + "/" + f);
    }
  }

  ui->progressBar->setValue(0);
  ui->progressBar->setMaximum(254);
  for (int ip = 1; ip < 255; ip++)
  {
    QHttp* http = new QHttp();
    http->setProperty("ip", ip);
    http->setHost(settings->value("Global/uTorrentIPs").toString().arg(ip), 80);
    httpBuf[ip] = new QBuffer();
    QTimer* timer = new QTimer();
    http->get("/" + settings->value("Global/Distfile").toString(), httpBuf[ip]);
    QObject::connect(http, SIGNAL(done(bool)), this, SLOT(processExisting()));
    timer->setSingleShot(true);
    connect(timer, SIGNAL(timeout()), this, SLOT(processExisting()));
    timer->start(1000);
  }
}
