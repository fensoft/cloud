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

void Cloud::on_list_itemDoubleClicked(QTreeWidgetItem* item, int column)
{
  if (item->data(0, Qt::UserRole + 1).toInt() == 1)
    torrent_setPrio(torrent_prio_normal, item->data(0, Qt::UserRole).toInt());

  if (item->data(0, Qt::UserRole + 1).toInt() == 2)
  {
    qDebug() << "run" << item->data(0, Qt::UserRole + 2).toString() << item->data(0, Qt::UserRole + 1).toString() << item->data(0, Qt::UserRole).toString();
    p.setWorkingDirectory(QString("%1").arg(item->data(0, Qt::UserRole + 2).toString()));
    qDebug() << p.workingDirectory();
    QString cmd = QString("\"%1/%2\"").arg(item->data(0, Qt::UserRole + 2).toString(), item->data(0, Qt::UserRole).toString());
    cmd = QString("%1/%2").arg(item->data(0, Qt::UserRole + 2).toString(), item->data(0, Qt::UserRole).toString());
    cmd = cmd.replace("\\", "/");
    QString fol = QString("%1/%2").arg(QApplication::applicationDirPath(), item->data(0, Qt::UserRole + 2).toString());
    QString cmd2 = QString("\"%1/%2").arg(QApplication::applicationDirPath(), cmd);
    qDebug() << "run cmd" << cmd2 << "in" << fol;
    if (cmd.contains(".cmd") || cmd.contains(".bat"))
    {
      p.startDetached(QString("%1\"").arg(cmd2), QStringList(), fol);
    }
    else
    {
      p.setWorkingDirectory(fol);
      p.start(cmd2);
    }
  }
}
