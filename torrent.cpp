#include "torrent.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

Torrent::Torrent(QSettings* setGlobal_, QList<game>* games_, QStringList* hashes_, QObject *parent) :
    QObject(parent)
{
  setGlobal = setGlobal_;
  games = games_;
  hashes = hashes_;
}

void Torrent::startApp()
{
  TRACE(5, "");
  int first_time = 0;

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
      qsl << "/directory" << QApplication::applicationDirPath() << QString("%1\\games->torrent").arg(QApplication::applicationDirPath()) << "/MINIMIZED";
    }
    first_time = 1;
  }
  QProcess::startDetached(QCoreApplication::applicationDirPath() + "/" + setGlobal->value("Global/uTorrentExe").toString(), QStringList("/minimized"));
  TRACE(5, "");
}

void Torrent::showApp()
{
  QProcess::startDetached(setGlobal->value("Global/uTorrentExe").toString(), QStringList("/bringtofront"));
}

void Torrent::setState(torrent_state state)
{
  TRACE(5, "");
  QBuffer* httpBufLocal = new QBuffer(this);
  httpBufLocal->open(QIODevice::ReadWrite);
  QHttp* httpget = new QHttp();
  httpget->setHost("localhost", setGlobal->value("Global/uTorrentPort").toInt());
  httpget->setUser(setGlobal->value("Global/uTorrentAdminLogin").toString(), setGlobal->value("Global/uTorrentAdminPassword").toString());

  //qDebug() << "dl_start_torrent" << currentItem;
  QStringList sl;
  for (int i = 0; i != games->count(); i++)
    sl << games->at(i).hash;
  sl.removeDuplicates();
  QString wtdo;
  if (state==torrent_start)
    wtdo = "start";
  if (state==torrent_stop)
    wtdo = "stop";
  for (int i = 0; i != sl.count(); i++)
  {
    QString test = QString("/gui/?action=%1&hash=%2&").arg(wtdo).arg(sl.at(i));
    httpget->get(test, httpBufLocal);
  }
  TRACE(5, "");
}

void Torrent::setPrio(torrent_prio prio, int gameid, QString ip)
{
  TRACE(5, "");
  QBuffer* httpBufLocal = new QBuffer(this);
  httpBufLocal->open(QIODevice::ReadWrite);
  QHttp* httpget = new QHttp();
  httpget->setHost(ip, setGlobal->value("Global/uTorrentPort").toInt());
  httpget->setUser(setGlobal->value("Global/uTorrentAdminLogin").toString(), setGlobal->value("Global/uTorrentAdminPassword").toString());

  //qDebug() << "dl_start" << currentItem;
  int item = gameid;
  QMap<QString, dl> dllist = getInfo(*hashes);

  for (int j = 0; j < games->at(item).files.count(); j++)
  {
    QString test = QString("/gui/?action=setprio&hash=%3&p=%2&f=%1").arg(dllist[games->at(item).files.at(j)].pos)
                   .arg(prio)
                   .arg(games->at(item).hash);
    qDebug() << ip << test;
    qDebug() << dllist.keys();
    httpget->get(test, httpBufLocal);
  }
  TRACE(5, "");
}

void Torrent::setPrioAll(torrent_prio prio, int gameid)
{
  TRACE(5, "");
  for (int i = 1; i != 254; i++)
    setPrio(prio, gameid, QString("%1%2").arg(setGlobal->value("Global/uTorrentIPs").toString()).arg(i));
  TRACE(5, "");
}

QMap<QString, dl> Torrent::getInfo(QStringList hashes)
{
  //TRACE(10, "");
  QMap<QString, dl> result;
  /*static */QBuffer httpBuf;
  foreach (QString hash, hashes)
  {
    QNetworkAccessManager manager;
    QEventLoop loop;
    connect(&manager, SIGNAL(finished(QNetworkReply*)), &loop, SLOT(quit()));
    //qDebug() << "...";
    QString surl = QString("http://%3@localhost:%1%2").arg(setGlobal->value("Global/uTorrentPort").toInt())
        .arg(QString("/gui/?action=getfiles&hash=%1").arg(hash))
        .arg(QString("admin:admin"));
    //qDebug() << surl;
    QUrl url = QUrl(surl);
    //QUrl("http://admin:admin@localhost:1337/gui/?action=getfiles&hash=3023DD766169B49F402F248F66B783BD90E6AECB");
    QNetworkRequest request = QNetworkRequest(url);
    QNetworkReply* reply = manager.get(request);
    loop.exec();

    int i = 0;
    while (reply->bytesAvailable())
    {
      QString line = reply->readLine();
      //qDebug() << line;
      line.replace(QRegExp("^,*"), "");
      QStringList sl = QString(line).split(',');
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
        if (i > 512)
        {
          qDebug() << "error";
        }
        sl[0].replace("\\\\", "/");
        result.insert(sl[0], c);
      }
      /*else
        qDebug() << "skip";*/
    }
    //qDebug() << result.size();
    /*httpBuf.open(QIODevice::ReadWrite | QIODevice::Truncate);
    QHttp http;
    http.setHost("localhost", );
    http.setUser();
    http.get(QString("/gui/?action=getfiles&hash=%1").arg(hash), &httpBuf);
    QEventLoop loop;
    connect(&http, SIGNAL(done(bool)), &loop, SLOT(quit()));
    loop.exec();
    httpBuf.seek(0);
    QByteArray res = httpBuf.readAll();
    qDebug() << http.errorString();
    qDebug() << res;
    qDebug() << httpBuf.size();
    return result;
    int i = 0;
    while (!httpBuf.atEnd())
    {
      char buf[32768];
      //qDebug() <<
      httpBuf.readLine(buf, 32767);
      buf[32767] = 0;
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
    //qDebug() << result.keys();*/
  }
  //TRACE(10, "");
  return result;
}

stats Torrent::getStats()
{
  stats result;
  result.estimated = -1;
  result.down = result.up = 0;
  static QBuffer httpBufStatus;
  httpBufStatus.open(QIODevice::ReadWrite | QIODevice::Truncate);

  QHttp httpStatus;
  httpStatus.setHost("127.0.0.1", setGlobal->value("Global/uTorrentPort").toInt());
  httpStatus.setUser(setGlobal->value("Global/uTorrentAdminLogin").toString(), setGlobal->value("Global/uTorrentAdminPassword").toString());

  httpStatus.get(QString("/gui/?list=1"), &httpBufStatus);
  //current_torrent = (current_torrent + 1) % hashes->size(); //FIXME ?
  QEventLoop loop;
  connect(&httpStatus, SIGNAL(done(bool)), &loop, SLOT(quit()));
  loop.exec();
  //connect(httpStatus, SIGNAL(done(bool)), this, SLOT(ut_status_done_timer()));
  //TRACE(10, "");
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
