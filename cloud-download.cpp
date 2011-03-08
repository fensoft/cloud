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

void Cloud::dl_reset()
{
  TRACE(5, "");
  for (int i =1; i != 255; i++)
  {
    currentItem = i;
    dl_set_prio(0);
  }
  TRACE(5, "");
}

void Cloud::dl_start_low_all()
{
  TRACE(5, "");
  for (int i = 1; i != 254; i++)
    dl_set_prio(1, QString("%1%2").arg(setGlobal->value("Global/uTorrentIPs").toString()).arg(i));
  TRACE(5, "");
}

void Cloud::dl_start_all()
{
  TRACE(5, "");
  for (int i = 1; i != 254; i++)
    dl_set_prio(2, QString("%1%2").arg(setGlobal->value("Global/uTorrentIPs").toString()).arg(i));
  TRACE(5, "");
}

void Cloud::dl_stop_all()
{
  TRACE(5, "");
  for (int i = 1; i != 254; i++)
    dl_set_prio(0, QString("%1%2").arg(setGlobal->value("Global/uTorrentIPs").toString()).arg(i));
  TRACE(5, "");
}

void Cloud::dl_start_low()
{
  dl_set_prio(1);
}

void Cloud::dl_start()
{
  dl_set_prio(2);
}

void Cloud::dl_stop()
{
  dl_set_prio(0);
}

void Cloud::dl_start_torrent()
{
  dl_startstop_torrent("start");
}

void Cloud::dl_stop_torrent()
{
  dl_startstop_torrent("stop");
}


void Cloud::dl_startstop_torrent(QString start)
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
    QString test = QString("/gui/?action=%1&hash=%2&").arg(start).arg(sl.at(i));
    httpget->get(test, httpBufLocal);
  }
  TRACE(5, "");
}

void Cloud::dl_set_prio(int prio, QString ip)
{
  TRACE(5, "");
  QBuffer* httpBufLocal = new QBuffer(this);
  httpBufLocal->open(QIODevice::ReadWrite);
  QHttp* httpget = new QHttp();
  httpget->setHost(ip, setGlobal->value("Global/uTorrentPort").toInt());
  httpget->setUser(setGlobal->value("Global/uTorrentAdminLogin").toString(), setGlobal->value("Global/uTorrentAdminPassword").toString());

  qDebug() << "dl_start" << currentItem;
  int item = currentItem;
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

void Cloud::dl_get()
{
  ut_get_file();
}

void Cloud::ut_get_file()
{
  TRACE(10, "");
  if (games.size() == 0)
    return;
  {
    foreach (QString hash, hashes)
    {
      httpBuf.open(QIODevice::ReadWrite | QIODevice::Truncate);
      http->get(QString("/gui/?action=getfiles&hash=%1").arg(hash), &httpBuf);
      QEventLoop loop;
      connect(http, SIGNAL(done(bool)), &loop, SLOT(quit()));
      loop.exec();
      ut_get_file_done();
    }
  }
  httpBufStatus.open(QIODevice::ReadWrite | QIODevice::Truncate);
  httpStatus->get(QString("/gui/?list=1"), &httpBufStatus);
  current_torrent = (current_torrent + 1) % hashes.size();
  TRACE(10, "");
}

void Cloud::ut_get_file_done_timer()
{
  TRACE(10, "");
  changed = ((changed >= setGlobal->value("Global/SleepRefreshTime").toInt()) ? (setGlobal->value("Global/SleepRefreshTime").toInt()) : (changed *2));
  QTimer::singleShot(changed, this, SLOT(ut_get_file_done()));
  TRACE(10, "");
}

void Cloud::ut_status_done_timer()
{
  TRACE(10, "");
  QTimer::singleShot(changed, this, SLOT(ut_status_done()));
  TRACE(10, "");
}

void Cloud::ut_status_done()
{
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
      long long r = QVariant(sl[8]).toLongLong() / 104857L;
      long long e = QVariant(sl[9]).toLongLong() / 104857L;
      int t = QVariant(sl[10]).toInt() / 60;
      //fixme : add some widgets
      statusBar()->showMessage(tr("Up %1,%2 Mb/s\tDl %3,%4 Mb/s\tTime : %5 min").arg(r/10).arg(r%10).arg(e/10).arg(e%10).arg(t));
    }
  }
  TRACE(10, "");
}

void Cloud::ut_get_file_done()
{
  TRACE(10, "");
  int local_changed = 0;
  httpBuf.seek(0);
  int i = 0;
  while (!httpBuf.atEnd())
  {
    TRACE(10, "");
    char buf[1025];
    qDebug() << httpBuf.readLine(buf, 1024);
    QString bufS(buf);
    TRACE(10, bufS);
    if (bufS.size() && (bufS[0] == '[' || bufS[1] == '['))
    {
      bufS.replace(QRegExp("^,*"), "");
      TRACE(10, "");
      QStringList sl = bufS.split(',');
      TRACE(10, "");
      if (sl.size() == 4)
      {
        //TRACE(5, "");
        TRACE(10, "");
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
        dllist.insert(sl[0], c);
      }
    }
  }
  TRACE(10, "");
  if (dllist.size() > 5 && first_time == 1)
  {
    first_time = 0;
    dl_reset();
  }
  TRACE(10, dllist.keys());
  for (int h = 0; h < games.size(); h++)
  {
    int item = h;
    if (item == -1)
      continue;
    if (games.at(item).name.isEmpty())
    {
      continue;
    }
    dl total;
    dl_progress(h, &total);

    QString fol = games.at(item).folder.isEmpty() ? games.at(item).name : games.at(item).folder;
    QString desc = QString("%1/%2/" + setGlobal->value("Global/DescFile").toString()).arg(setGlobal->value("Global/Folder").toString(), fol);
    TRACE(10, "");
    int downloadfinished = total.state && total.current == total.max;
    int mustinstall = total.state >= 2 && total.current == total.max;
    int lowprio = total.state == 1;
    int descfilepresent = QFile::exists(desc);
    int extractfinished = (current_v == -1);
    int shortcutcreated = installed.contains(item) == true;
    int installinprogress = (current_v == item);
    TRACE(10, "");
    if (setGlobal->value("Global/uTorrent").toInt() == 1)
    {
      if (total.state == 0)
      {
        SET_ITEM(items[item], QString("%1").arg(games.at(item).name), tr("Paused"), 99, Qt::darkGray);
        continue;
      }

      if (installinprogress)
      {
        //SET_ITEM(items[item], QString("%1").arg(games.at(item).name), "install...", 30, Qt::cyan);
        continue;
      }

      if (shortcutcreated)
      {
        SET_ITEM(items[item], QString("%1").arg(games.at(item).name), "", 1, Qt::white);
        continue;
      }

      if (/*extractfinished || */descfilepresent)
      {
        SET_ITEM(items[item], QString("%1").arg(games.at(item).name), tr("Shortcut"), 30, Qt::cyan);
        installed << item;
        add_shortcuts(item);
        continue;
      }
    }

    if (setGlobal->value("Global/uTorrent") == 1)
    {
      if (downloadfinished)
      {
        if (!extractfinished)
        {
          SET_ITEM(items[item], QString("%1").arg(games.at(item).name), tr("Waiting..."), 40, Qt::cyan);
          continue;
        }
        else
        {
          if (mustinstall)
          {
            SET_ITEM(items[item], QString("%1").arg(games.at(item).name), tr("Install"), 30, Qt::cyan);
            current_v = h;
            current_i = 0;
            //refresh_log();
            continue;
          }
          else
          {
            SET_ITEM(items[item], QString("%1").arg(games.at(item).name), tr("Finished"), 50, Qt::cyan);
            continue;
          }
        }
      }


      if (total.state)
      {
        if (lowprio)
          SET_ITEM(items[item], QString("%1").arg(games.at(item).name), tr("Downloading... %2/%3").arg((unsigned long)total.current).arg((unsigned long)total.max), 55, Qt::gray);
        else
          SET_ITEM(items[item], QString("%1").arg(games.at(item).name), tr("Downloading... %2/%3").arg((unsigned long)total.current).arg((unsigned long)total.max), 45, Qt::cyan);
        continue;
      }
    }
    if (setGlobal->value("Global/uTorrent") == 0)
    {
      SET_ITEM(items[item], QString("%1").arg(games.at(item).name), tr("Not installed"), 60, Qt::darkGray);
      continue;
    }
    SET_ITEM(items[item], QString("%1").arg(games.at(item).name), tr("Error"), 100, Qt::red);
  }

  if (local_changed == 1)
  {
    qDebug() << "changed";
    ui->list->sortItems(0, Qt::AscendingOrder);
    ui->list->sortItems(2, Qt::AscendingOrder);
    ui->list->resizeColumnToContents(0);
    ui->list->resizeColumnToContents(1);
  }
  //dl_get();
  TRACE(10, "");
}

void Cloud::dl_progress(int h, dl* total2)
{
  TRACE(10, "");
  int item = h;

  dl total;
  total.current = total.max = total.state = 0;
  for (int i = 0; i != games.at(item).files.count(); i++)
  {
    QString filename = games.at(item).files.at(i);
    total.current += dllist[filename].current;
    total.max += dllist[filename].max;
    total2->state = dllist[filename].state;
  }
  total2->current = total.current / 1048576L;
  total2->max = total.max / 1048576L;
  TRACE(10, "");
}
