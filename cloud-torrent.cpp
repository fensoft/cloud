#include "cloud.h"
#include "ui_cloud.h"
#include "QSettings"
#include "QDebug"
#include "QMenu"
#include <QTimer>
#include <QHttp>
#include <QBuffer>
#include <QList>
#include <QFile>
#include <QtGui>
#include "addserver.h"

void Cloud::torrent_startApp()
{
  int first_time = 0;

  http = new QHttp();
  http->setHost("localhost", setGlobal->value("Global/uTorrentPort").toInt());
  http->setUser(setGlobal->value("Global/uTorrentAdminLogin").toString(), setGlobal->value("Global/uTorrentAdminPassword").toString());
  //connect(http, SIGNAL(done(bool)), this, SLOT(ut_get_file_done_timer()));

  httpStatus = new QHttp();
  httpStatus->setHost("localhost", setGlobal->value("Global/uTorrentPort").toInt());
  httpStatus->setUser(setGlobal->value("Global/uTorrentAdminLogin").toString(), setGlobal->value("Global/uTorrentAdminPassword").toString());
  //connect(httpStatus, SIGNAL(done(bool)), this, SLOT(ut_status_done_timer()));

  int reset = !QFile::exists("utorrent_configured");
  if (reset == 1)
  {
    QFile("utorrent_configured").open(QFile::WriteOnly);
    QFile::remove("utorrent/resume.dat");
    QFile::remove("utorrent/settings.dat");
    QFile::copy("utorrent/resume.dat.orig", "utorrent/resume.dat");
    QFile::copy("utorrent/settings.dat.orig", "utorrent/settings.dat");
    if (setGlobal->value("Global/uTorrent").toInt() == 1)
    {
      QStringList qsl;
      qsl << "/directory" << QApplication::applicationDirPath() << QString("%1\\games.torrent").arg(QApplication::applicationDirPath()) << "/MINIMIZED";
    }
    first_time = 1;
  }
  QProcess::startDetached(QCoreApplication::applicationDirPath() + "/" + setGlobal->value("Global/uTorrentExe").toString(), QStringList("/minimized"));
}

void Cloud::torrent_showApp()
{

}

void Cloud::torrent_setState(torrent_state i)
{
  TRACE(5, "");
  QBuffer* httpBufLocal = new QBuffer(this);
  httpBufLocal->open(QIODevice::ReadWrite);
  QHttp* httpget = new QHttp();
  httpget->setHost("localhost", setGlobal->value("Global/uTorrentPort").toInt());
  httpget->setUser(setGlobal->value("Global/uTorrentAdminLogin").toString(), setGlobal->value("Global/uTorrentAdminPassword").toString());

  qDebug() << "dl_start_torrent" << currentItem;
  QStringList sl;
  for (int i = 0; i != games.count(); i++)
    sl << games.at(i).hash;
  sl.removeDuplicates();
  for (int i = 0; i != sl.count(); i++)
  {
    QString test = QString("/gui/?action=%1&hash=%2&").arg(i==torrent_start?"start":"stop").arg(sl.at(i));
    httpget->get(test, httpBufLocal);
  }
  TRACE(5, "");
}

void Cloud::torrent_setPrio(torrent_prio prio, int gameid, QString ip)
{
  TRACE(5, "");
  QBuffer* httpBufLocal = new QBuffer(this);
  httpBufLocal->open(QIODevice::ReadWrite);
  QHttp* httpget = new QHttp();
  httpget->setHost(ip, setGlobal->value("Global/uTorrentPort").toInt());
  httpget->setUser(setGlobal->value("Global/uTorrentAdminLogin").toString(), setGlobal->value("Global/uTorrentAdminPassword").toString());

  qDebug() << "dl_start" << currentItem;
  int item = gameid;
  QMap<QString, dl> dllist = torrent_getInfo(hashes);

  for (int j = 0; j < games.at(item).files.count(); j++)
  {
    QString test = QString("/gui/?action=setprio&hash=%3&p=%2&f=%1").arg(dllist[games.at(item).files.at(j)].pos)
                   .arg(prio)
                   .arg(games.at(item).hash);
    qDebug() << ip << test;
    qDebug() << dllist.keys();
    httpget->get(test, httpBufLocal);
  }
  TRACE(5, "");
}

void Cloud::torrent_setPrioAll(torrent_prio prio, int gameid)
{
  TRACE(5, "");
  for (int i = 1; i != 254; i++)
    torrent_setPrio(prio, gameid, QString("%1%2").arg(setGlobal->value("Global/uTorrentIPs").toString()).arg(i));
  TRACE(5, "");
}

QMap<QString, dl> Cloud::torrent_getInfo(QStringList hashes)
{
  TRACE(10, "");
  QMap<QString, dl> result;
  static QBuffer httpBuf;
  foreach (QString hash, hashes)
  {
    httpBuf.open(QIODevice::ReadWrite | QIODevice::Truncate);
    http->get(QString("/gui/?action=getfiles&hash=%1").arg(hash), &httpBuf);
    QEventLoop loop;
    connect(http, SIGNAL(done(bool)), &loop, SLOT(quit()));
    loop.exec();
    httpBuf.seek(0);
    int i = 0;
    while (!httpBuf.atEnd())
    {
      char buf[1025];
      //qDebug() <<
      httpBuf.readLine(buf, 1024);
      QString bufS(buf);
      if (bufS.size() && (bufS[0] == '[' || bufS[1] == '['))
      {
        bufS.replace(QRegExp("^,*"), "");
        QStringList sl = bufS.split(',');
        if (sl.size() == 4)
        {
          //TRACE(5, "");
          //TRACE(10, "");
          sl[0].replace("[\"", "").replace("[", "").replace("\"", "");
          sl[3].replace("]", "");
          dl c;
          c.current = c.max = 0L;
          c.current = QVariant(sl[2]).toLongLong();
          c.max = QVariant(sl[1]).toLongLong();
          c.state = QString(sl[3]).toInt();
          c.pos = i;
          i++;
          if (i > 70)
          {
            qDebug() << "error";
          }
          result.insert(sl[0], c);
        }
      }
    }
    //qDebug() << result.keys();
  }
  TRACE(10, "");
  return result;
}

stats Cloud::torrent_getStats()
{
  stats result;
  result.estimated = -1;
  result.down = result.up = 0;
  static QBuffer httpBufStatus;
  httpBufStatus.open(QIODevice::ReadWrite | QIODevice::Truncate);
  httpStatus->get(QString("/gui/?list=1"), &httpBufStatus);
  current_torrent = (current_torrent + 1) % hashes.size();
  QEventLoop loop;
  connect(httpStatus, SIGNAL(done(bool)), &loop, SLOT(quit()));
  loop.exec();
  //connect(httpStatus, SIGNAL(done(bool)), this, SLOT(ut_status_done_timer()));
  TRACE(10, "");
  httpBufStatus.seek(0);
  while (!httpBufStatus.atEnd())
  {
    char buf[1024];
    if (httpBufStatus.readLine(buf, 1024) == 0)
      break;
    QString bufS(buf);
    bufS.replace("[", "");
    bufS.replace("]", "");
    bufS.replace("\"", "");
    QStringList sl = bufS.split(',');
    if (sl.size() == 19)
    {
      result.down = QVariant(sl[8]).toLongLong() / 104857L;
      result.up = QVariant(sl[9]).toLongLong() / 104857L;
      result.estimated = QVariant(sl[10]).toInt() / 60;
      //fixme : add some widgets
    }
  }
  return result;
}