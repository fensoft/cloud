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

void Cloud::dl_reinstall()
{
  TRACE(5, "");
  qDebug() << "reinstall" << currentItem;
  int item = currentItem;
  if (item == -1)
    return;
  QString fol = games.at(item).folder.isEmpty() ? games.at(item).name : games.at(item).folder;
  QString fil = QString("%1/%2/" + setGlobal->value("Global/DescFile").toString()).arg(setGlobal->value("Global/Folder").toString()).arg(fol);
  qDebug() << fil << QFile::remove(fil);
  finished.removeAll(item);
  installed.removeAll(item);
  while (items[item]->childCount())
    items[item]->removeChild(items[item]->child(0));
  TRACE(5, "");
}

void Cloud::dl_reconfigure()
{
  TRACE(5, "");
  int item = currentItem;
  QString fol = games.at(item).folder.isEmpty() ? games.at(item).name : games.at(item).folder;
  QString cmd = QString("%1/%2/%3/%4").arg(QApplication::applicationDirPath(), setGlobal->value("Global/Folder").toString(), fol, "install.cmd");
  QString fol2 = QString("%1/%2/%3").arg(QApplication::applicationDirPath(), setGlobal->value("Global/Folder").toString(), fol);
  qDebug() << "reconfigure" << cmd << "in" << fol2;
  QProcess::startDetached(cmd, QStringList(), fol2);
  TRACE(5, "");
}

void Cloud::add_shortcuts(int item)
{
  TRACE(5, "");
  ui->list->sortItems(1, Qt::AscendingOrder);
  QString fol = games.at(item).folder.isEmpty() ? games.at(item).name : games.at(item).folder;
  QString desc = QString("%1/%2/" + setGlobal->value("Global/DescFile").toString()).arg(setGlobal->value("Global/Folder").toString(), fol);
  installed << item;
  QFile file(desc);
  file.open(QFile::ReadOnly);
  while (!file.atEnd())
  {
    QString line = file.readLine();
    if (line.contains("="))
    {
      line = line.left(line.size() - 2);
      QString name = line.split("=")[0];
      QString val = line.mid(line.indexOf("=")+1);
      QRegExp re("(.*)[.]exe");
      re.setMinimal(true);
      val = val.replace(re, "\\1.exe\"");//\"
      val = val.replace("\\\\", "\\");
      QTreeWidgetItem* lwitem = new QTreeWidgetItem();
      items[item]->setForeground(0, QBrush(Qt::white));
      lwitem->setText(0, tr("Run %1").arg(name));
      lwitem->setData(0, Qt::UserRole, val);
      lwitem->setData(0, Qt::UserRole + 1, 2);
      QString fol = games.at(item).folder.isEmpty() ? games.at(item).name : games.at(item).folder;
      lwitem->setData(0, Qt::UserRole + 2, QString("%1/%2").arg(setGlobal->value("Global/Folder").toString(), fol));
      items[item]->addChild(lwitem);
      ui->list->expandAll();
    }
  }
  TRACE(5, "");
}

void Cloud::refresh_log()
{
  TRACE(5, "");
  if (!unzip2.isRunning())
  {
    int item = current_v;
    if (item == -1)
      return;
    if (games.at(item).files.size())
    {
      unzip2.setZip(games.at(item).files);
      unzip2.setDest(setGlobal->value("Global/Folder").toString());
      unzip2.setSizeBuf(setGlobal->value("Global/UnzipBufSize").toInt());
      unzip2.start(QThread::HighPriority);
      ut_get_file_done();
    }
    else
      refreshUnzipProgressEnd();
    current_i = 0;
  }
  TRACE(5, "");
}

void Cloud::refreshUnzipProgressEnd()
{
  TRACE(5, "");
  finished << current_v;
  current_v = -1;
  TRACE(5, "");
}

void Cloud::refreshUnzipProgressMax(unsigned long long int d)
{
  TRACE(5, "");
  max = d;
  TRACE(5, "");
}

void Cloud::refreshUnzipProgress(unsigned long long int d)
{
  TRACE(5, "");
  int item = current_v;
  if (current_v == -1)
    return;
  int local_changed;
  SET_ITEM(items[item], QString("%1").arg(games.at(item).name), tr("Installing... %2/%3").arg((unsigned long)(d)).arg((unsigned long)max), 55, Qt::gray);
  qDebug() << d;
  TRACE(5, "");
}
