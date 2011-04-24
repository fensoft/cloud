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
  for (int i = 1; i != games.count(); i++)
    torrent_setPrio(torrent_prio_no, i);
}

void Cloud::dl_start_low_all()
{
  torrent_setPrioAll(torrent_prio_low, currentItem);
}

void Cloud::dl_start_all()
{
  torrent_setPrioAll(torrent_prio_normal, currentItem);
}

void Cloud::dl_stop_all()
{
  torrent_setPrioAll(torrent_prio_no, currentItem);
}

void Cloud::dl_start_low()
{
  torrent_setPrio(torrent_prio_low, currentItem);
}

void Cloud::dl_start()
{
  torrent_setPrio(torrent_prio_normal, currentItem);
}

void Cloud::dl_stop()
{
  torrent_setPrio(torrent_prio_no, currentItem);
}

void Cloud::dl_start_torrent()
{
  torrent_setState(torrent_start);
}

void Cloud::dl_stop_torrent()
{
  torrent_setState(torrent_stop);
}

void Cloud::ui_refresh()
{
  TRACE(10, "");
  if (games.size() == 0)
    return;


  ui_list_update(torrent_getInfo(hashes));

  stats st = torrent_getStats();
  if (st.estimated != -1)
    statusBar()->showMessage(tr("Up %1,%2 Mb/s\tDl %3,%4 Mb/s\tTime : %5 min").arg(st.down/10).arg(st.down%10).arg(st.up/10).arg(st.up%10).arg(st.estimated));
  else
    statusBar()->showMessage(tr("Offline"));

  if (!unzip.isRunning())
  {
    int item = current_v;
    if (item == -1)
      return;
    if (games.at(item).files.size())
    {
      if (games.at(item).files.size() == 0 || QFile::exists(games.at(item).files[0]))
        unzip.setZip("", games.at(item).files);
      else
        unzip.setZip(games.at(item).location, games.at(item).files);
      unzip.setDest(setGlobal->value("Global/Folder").toString());
      unzip.setSizeBuf(setGlobal->value("Global/UnzipBufSize").toInt());
      unzip.start(QThread::HighPriority);
      //ut_get_file_done();
    }
    else
      refreshUnzipProgressEnd();
    current_i = 0;
  }

  TRACE(10, "");
}

void Cloud::ui_list_update(QMap<QString, dl> dllist)
{
  int local_changed = 0;

  TRACE(10, "");
  /*if (dllist.size() > 5 && first_time == 1)
  {
    first_time = 0;
    dl_reset();
  }*/
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
    total.state = 0;
    {
      int item = h;

      dl total2;
      total2.current = total2.max = total2.state = 0;
      for (int i = 0; i != games.at(item).files.count(); i++)
      {
        QString filename = games.at(item).files.at(i);
        total2.current += dllist[filename].current;
        total2.max += dllist[filename].max;
        total.state = dllist[filename].state;
      }
      total.current = total2.current / 1048576L;
      total.max = total2.max / 1048576L;
    }
    //dl_progress(h, &total);

    QString fol = games.at(item).folder.isEmpty() ? games.at(item).name : games.at(item).folder;
    QString desc = QString("%1/%2/" + setGlobal->value("Global/DescFile").toString()).arg(setGlobal->value("Global/Folder").toString(), fol);
    //TRACE(10, "");
    int downloadfinished = total.state && total.current == total.max;
    int mustinstall = total.state >= 2 && total.current == total.max;
    int lowprio = total.state == 1;
    int descfilepresent = QFile::exists(desc);
    int extractfinished = (current_v == -1);
    int shortcutcreated = installed.contains(item) == true;
    int installinprogress = (current_v == item);
    //TRACE(10, "");
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
    timer_ui->setInterval(100);
  }
  else
  {
    timer_ui->setInterval((timer_ui->interval() == 1000) ? 1000 : (timer_ui->interval() + 100));
  }
  //dl_get();
  TRACE(10, "");
}
