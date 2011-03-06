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

QBuffer Cloud::httpBuf;
QBuffer Cloud::httpBufStatus;

Cloud::Cloud(QSettings* set, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Cloud)
{
  max = 0;
  current_torrent = 0;
  ui->setupUi(this);
  ui->list->clear();
  ui->list->hideColumn(2);
  setGlobal = set;
  QDir dir(setGlobal->value("Global/Database").toString());
  QStringList listgames = dir.entryList(
      QStringList("*." + setGlobal->value("Global/TorrentDescriptionExt").toString()));
  foreach(QString game,listgames)
  {
    TRACE(5, "will read" << game);
    readIni(game);
  }

  updateContent();
  first_time = 0;

  http = new QHttp();
  http->setHost("localhost", setGlobal->value("Global/uTorrentPort").toInt());
  http->setUser(setGlobal->value("Global/uTorrentAdminLogin").toString(), setGlobal->value("Global/uTorrentAdminPassword").toString());
  connect(http, SIGNAL(done(bool)), this, SLOT(ut_get_file_done_timer()));

  httpStatus = new QHttp();
  httpStatus->setHost("localhost", setGlobal->value("Global/uTorrentPort").toInt());
  httpStatus->setUser(setGlobal->value("Global/uTorrentAdminLogin").toString(), setGlobal->value("Global/uTorrentAdminPassword").toString());
  connect(httpStatus, SIGNAL(done(bool)), this, SLOT(ut_status_done_timer()));

  connect(&unzip2, SIGNAL(finished()), this, SLOT(refresh_log()));
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

  ui->list->setIconSize(QSize(32, 32));
  current_v = -1;
  changed = 10;
  ui->list->setColumnWidth(0, 350);
  dl_get();
  statusBar()->showMessage("...");
  ui->list->setItemDelegateForColumn(1, new ItemDelegate(ui->list));
  QProcess::startDetached(QCoreApplication::applicationDirPath() + "/" + setGlobal->value("Global/uTorrentExe").toString(), QStringList("/minimized"));
  qRegisterMetaType<unsigned long long int>("unsigned long long int");
  connect(&unzip2, SIGNAL(newState(unsigned long long int)), this, SLOT(refreshUnzipProgress(unsigned long long int)));
  connect(&unzip2, SIGNAL(max(unsigned long long int)), this, SLOT(refreshUnzipProgressMax(unsigned long long int)));
  connect(&unzip2, SIGNAL(end()), this, SLOT(refreshUnzipProgressEnd()));
  ui->toolBar->setFixedHeight(32);
  ui->toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  ui->toolBar->addAction(QIcon(":/res/play.png"), "", this, SLOT(dl_start_torrent()));
  ui->toolBar->addAction(QIcon(":/res/slow.png"), tr("Slow down"), this, SLOT(show_utorrent()));
  ui->toolBar->addAction(QIcon(":/res/stop.png"), "", this, SLOT(dl_stop_torrent()));
  ui->toolBar->addAction(QIcon(":/res/eject.png"), tr("Cancel all"), this, SLOT(dl_reset()));
  ui->toolBar->addSeparator();
  ui->toolBar->addAction(QIcon(":/res/add.png"), "", this, SLOT(show_utorrent()));
  ui->toolBar->addAction(QIcon(":/res/utorrent.png"), "", this, SLOT(show_utorrent()));
  ui->toolBar->addSeparator();
  ui->toolBar->addAction(QIcon(":/res/shutdown.png"), "", this, SLOT(appExit()));
}

Cloud::~Cloud()
{
  delete ui;
}
