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

void Cloud::appExit()
{
  QProcess* p = new QProcess();
  p->start(setGlobal->value("Global/CloseApp").toString(),
           QStringList(QString(
               QCoreApplication::applicationDirPath() + "/" + setGlobal->value("Global/uTorrentExe").toString()
               ).replace("/", "\\")));
  p = new QProcess();
  p->start(setGlobal->value("Global/CloseApp").toString(),
           QStringList(QString(
               QCoreApplication::applicationDirPath() + "/" + setGlobal->value("Global/uTorrentExe").toString()
               ).replace("/", "\\")));
  p = new QProcess();
  QStringList sl;
  sl << "/F" << QString(QCoreApplication::applicationDirPath() + "/" + setGlobal->value("Global/Webserver").toString()).replace("/", "\\");

  p->start(setGlobal->value("Global/CloseApp").toString(), sl);
  close();
}

void Cloud::show_utorrent()
{
  QProcess::startDetached(setGlobal->value("Global/uTorrentExe").toString(), QStringList("/bringtofront"));
}

void Cloud::readIni(QString ini)
{
  QString ret;
  qDebug() << "begin readIni";
  QSettings* set = new QSettings(setGlobal->value("Global/Database").toString() + "/" + ini, QSettings::IniFormat);
  qDebug() << "read";
  for (int i = 1; i <= 255; i++)
  {
    QString item = QString("item%1").arg(i);
    if (!set->value(item + "/name").toString().isEmpty())
    {
      game g;
      g.name = set->value(item + "/name").toString();
      g.folder = set->value(item + "/folder").toString();
      ret = g.hash = set->value("global/hash").toString();
      g.icon = set->value(item + "/icon").toString();
      int i = 1;
      while (!set->value(item + QString("/file%1").arg(i)).toString().isEmpty())
      {
        g.files << set->value(item + QString("/file%1").arg(i)).toString();
        i++;
      }
      qDebug() << "added one game";
      if (!g.name.isEmpty())
        games << g;
    }
  }
  qDebug() << "end readIni";
  hashes << ret;
}

void Cloud::updateContent()
{
  TRACE(5, "");
  ui->list->setContextMenuPolicy(Qt::CustomContextMenu);
  for (int i = 0; i < games.size(); i++)
  {
    {
      qDebug() << "will add";
      QTreeWidgetItem* lwitem = new QTreeWidgetItem();
      lwitem->setText(0, games.at(i).name);
      QFont f = lwitem->font(0);
      f.setBold(true);
      f.setPixelSize(14);
      lwitem->setFont(0, f);
      lwitem->setIcon(0, QIcon(QString(setGlobal->value("Global/Database").toString() + "/%1").arg(games.at(i).icon)));
      lwitem->setData(0, Qt::UserRole, i);
      lwitem->setData(0, Qt::UserRole + 1, 1);
      ui->list->addTopLevelItem(lwitem);
      items[i] = lwitem;
    }
  }
  TRACE(5, "");
}

void Cloud::on_list_customContextMenuRequested(QPoint pos)
{
  QTreeWidgetItem* qtwi = ui->list->itemAt(pos);
  if (qtwi)
  {
    qDebug() << "custom" << qtwi->text(0);
    if (qtwi->data(0, Qt::UserRole + 1).toInt() == 1)
    {
      currentItem = qtwi->data(0, Qt::UserRole).toInt();
      QMenu menu;
      menu.addSeparator();
      if (setGlobal->value("Global/uTorrent") == 1)
      {
        menu.addAction(QIcon(), tr("&Download and install"), this, SLOT(dl_start()));
        menu.addAction(QIcon(), tr("Download &only"), this, SLOT(dl_start_low()));
        menu.addAction(QIcon(), tr("&Stop"), this, SLOT(dl_stop()));
        menu.addAction(QIcon(), tr("&Reinstall"), this, SLOT(dl_reinstall()));
      }
      menu.addAction(QIcon(), tr("&Reconfigure"), this, SLOT(dl_reconfigure()));
      if (setGlobal->value("Global/isAdmin").toInt() == 1)
      {
        menu.addAction(QIcon(), tr("Download and install for ALL"), this, SLOT(dl_start_all()));
        menu.addAction(QIcon(), tr("Download only for ALL"), this, SLOT(dl_start_low_all()));
        menu.addAction(QIcon(), tr("Stop for ALL"), this, SLOT(dl_stop_all()));
      }
      menu.exec(QCursor::pos());
    }
  }
}

