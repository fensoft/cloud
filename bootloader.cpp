#include "bootloader.h"
#include "ui_bootloader.h"
#include "qDebug.h"
#include <QFile>
#include <QDir>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include "qjson/src/serializer.h"
#include "qjson/src/parser.h"

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
  //QTimer timer;
  QString cip = sender()->property("ip").toString();

  if (!processed.contains(cip))
  {
    if (!cip.isEmpty() && httpBuf[cip]->data().contains(settings->value("Global/MasterKeyword").toByteArray()))
    {
      ui->textBrowser->append(QString("Connected to %1").arg(cip));
      qDebug() << "got sgt from " << cip;
      QStringList sl = QString(httpBuf[cip]->data()).split("\r\n");
      sl.removeAll("");
      sl.removeAll(settings->value("Global/MasterKeyword").toString());
      QHttp* http;
      nbmac++;
      ui->nbmac->setText(QVariant(nbmac).toString());
      httpBuf[cip]->close();
      foreach (QString cur, sl)
      {
        http = new QHttp();
        http->setHost(cip, 80);
        QObject::connect(http, SIGNAL(done(bool)), &loop, SLOT(quit()));
        nbfil++;
        ui->nbfil->setText(QVariant(nbfil).toString());
        ui->inProgress->setText(cur);
        QFile file;
        file.setFileName(settings->value("Global/Database").toString() + "/" + cur);
        file.open(QIODevice::WriteOnly);
        http->get("/" + cur, &file);
        QTimer* timer = new QTimer();
        timer->setSingleShot(true);
        connect(timer, SIGNAL(timeout()), &loop, SLOT(quit()));
        //timer.start(settings->value("Global/AutoDelay").toInt());
        loop.exec();
        timer->stop();
        if (file.pos() == 0) //FIXME
        {
          qDebug() << "empty... redl...";
          sl << cur;
        }
        file.close();
      }
    }
    processed << cip;
  }
  ui->progressBar->setValue(1+ui->progressBar->value());
  if (ui->progressBar->value() == ips.size())
    QTimer::singleShot(33, this, SLOT(close()));
}

void BootLoader::process()
{
//  QString xtsi;
//  {
//    QNetworkAccessManager manager;
//    QNetworkRequest request(QUrl("http://localhost:9091/transmission/rpc"));
//    request.setRawHeader("User-Agent", "User Agent");
//    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
//    QEventLoop loop;
//    connect(&manager, SIGNAL(finished(QNetworkReply *)), &loop, SLOT(quit()));
//    QNetworkReply *reply = manager.post(request, "");
//    loop.exec();
//    if (reply->error() == QNetworkReply::UnknownContentError)
//    {
//      QRegExp re("X-Transmission-Session-Id: ([a-zA-Z0-9]*)");
//      re.indexIn(reply->readAll());
//      xtsi = re.cap(1);
//    }
//  }

//  {
//    QNetworkAccessManager* manager = new QNetworkAccessManager();
//    QNetworkRequest request(QUrl("http://localhost:9091/transmission/rpc&test=1"));
//    request.setRawHeader("User-Agent", "User Agent");
//    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
//    request.setRawHeader("X-Transmission-Session-Id", xtsi.toAscii());
//    QVariantMap vars;
//    vars.insert("method", "session-get");
//    //vars.insert("arguments", "version");
//    QJson::Serializer serializer;
//    QByteArray json = serializer.serialize(vars);
//    qDebug() << json;

//    QString PostVariable = json;
//    QEventLoop loop;
//    connect(manager, SIGNAL(finished(QNetworkReply *)), &loop, SLOT(quit()));
//    QNetworkReply *reply = manager->post(request, PostVariable.toUtf8());
//    loop.exec();
//    QString jsonresult;
//    if (reply->error() == QNetworkReply::NoError)
//    {
//      jsonresult = QString(reply->readAll());
//      //qDebug() << jsonresult;
//    }
//    else
//      qDebug() << "error" << reply->error() << reply->readAll() << reply->errorString();

//    QJson::Parser parser;
//    bool ok;
//    QVariantMap result2 = parser.parse (jsonresult.toAscii(), &ok).toMap();
//    QVariantMap result = result2["arguments"].toMap();
//    foreach (QString v, result.keys())
//    {
//      qDebug() << v << "=>" << result[v];
//    }

//    loop.exec();
//  }

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
  QStringList filters;
  filters << (QString("*.") + settings->value("Global/TorrentDescriptionExt").toString()) << "*.png" << "*.torrent";
  QStringList fl = QDir(QCoreApplication::applicationDirPath()+ "/" + settings->value("Global/WebserverRoot").toString()).entryList(filters);
  QFile dist;
  dist.setFileName(settings->value("Global/WebserverRoot").toString() + "/" + settings->value("Global/Distfile").toString());
  dist.open(QIODevice::WriteOnly);
  dist.write((settings->value("Global/MasterKeyword").toString() + "\r\n").toAscii());
  foreach(QString f, fl)
  {
    qDebug() << "listing" << f;
    dist.write((f + "\r\n").toAscii());
  }
  dist.close();

  ui->progressBar->setValue(0);

  ips << settings->value("Global/MasterServers").toStringList();
  if (settings->value("Global/AutoMasterServers").toBool())
  {
    for (int ip = 1; ip < 255; ip++)
    {
      ips << settings->value("Global/uTorrentIPs").toString().arg(ip);
    }
  }

  ips.removeDuplicates();
  QStringList ips2 = ips;
  foreach (QString ip, ips2)
    for (int i = 1; i < settings->value("Global/MasterServersRetry").toInt(); i++)
      ips << ip;

  ui->progressBar->setMaximum(ips.size());
  foreach (QString cip, ips)
  {
    QHttp* http = new QHttp();
    http->setProperty("ip", cip);
    http->setHost(cip, 80);
    httpBuf[cip] = new QBuffer();
    QTimer* timer = new QTimer();
    http->get("/" + settings->value("Global/Distfile").toString(), httpBuf[cip]);
    QObject::connect(http, SIGNAL(done(bool)), this, SLOT(processExisting()));
    timer->setSingleShot(true);
    connect(timer, SIGNAL(timeout()), this, SLOT(processExisting()));
    QEventLoop loop;
    QTimer::singleShot(settings->value("Global/AutoDelay").toInt(), &loop, SLOT(quit()));
    loop.exec();
    timer->start(settings->value("Global/AutoTimeout").toInt());
  }
}
