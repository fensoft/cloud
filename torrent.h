#ifndef TORRENT_H
#define TORRENT_H

#include <QObject>
#include "QSettings"
#include "QDebug"
#include "QMenu"
#include <QTimer>
#include <QHttp>
#include <QBuffer>
#include <QList>
#include <QFile>
#include <QtGui>
#include "cloud.h"

class Torrent : public QObject
{
  Q_OBJECT
public:
  explicit Torrent(QSettings* setGlobal, QList<game>* games, QStringList* hashes, QObject *parent = 0);

  QSettings* setGlobal;
  QList<game>* games;
  QStringList* hashes;

  void startApp();
  void showApp();
  void setState(torrent_state i);
  void setPrio(torrent_prio prio, int gameid, QString ip = "localhost");
  void setPrioAll(torrent_prio prio, int gameid);
  QMap<QString, dl> getInfo(QStringList hashes);
  stats getStats();

signals:

public slots:

};

#endif // TORRENT_H
