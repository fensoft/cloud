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

/*QBuffer Cloud::httpBuf;
QBuffer Cloud::httpBufStatus;*/

Cloud::Cloud(QSettings* set, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Cloud)
{
  torrent = new Torrent(set, &games, &hashes, this);

  unzip_max = 0;
  current_torrent = 0;
  ui->setupUi(this);
  ui->list->clear();
  ui->list->hideColumn(2);
  setGlobal = set;

  updateContent();
  TRACE(5, "");
  torrent->startApp();

  ui->list->setIconSize(QSize(32, 32));
  current_v = -1;
  //changed = 10;
  ui->list->setColumnWidth(0, 350);

  timer_ui = new QTimer();
  connect(timer_ui, SIGNAL(timeout()), this, SLOT(ui_refresh()));
  timer_ui->start(1000);

  statusBar()->showMessage(tr("Loading..."));

  ui->list->setItemDelegateForColumn(1, new ItemDelegate(ui->list));

  qRegisterMetaType<unsigned long long int>("unsigned long long int");

  connect(&unzip, SIGNAL(newState(unsigned long long int)), this, SLOT(refreshUnzipProgress(unsigned long long int)));
  connect(&unzip, SIGNAL(max(unsigned long long int)), this, SLOT(refreshUnzipProgressMax(unsigned long long int)));
  connect(&unzip, SIGNAL(end()), this, SLOT(refreshUnzipProgressEnd()));

  ui->toolBar->setFixedHeight(32);
  ui->toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  ui->toolBar->addAction(QIcon(":/res/play.png"), "", this, SLOT(dl_start_torrent()))->setToolTip(tr("Start all"));
  //ui->toolBar->addAction(QIcon(":/res/slow.png"), tr("Slow down"), this, SLOT(show_utorrent()));
  ui->toolBar->addAction(QIcon(":/res/stop.png"), "", this, SLOT(dl_stop_torrent()))->setToolTip(tr("Pause all"));
  ui->toolBar->addAction(QIcon(":/res/eject.png"), "", this, SLOT(dl_reset()))->setToolTip(tr("Cancel all"));
  ui->toolBar->addSeparator();
  //ui->toolBar->addAction(QIcon(":/res/add.png"), "", this, SLOT(show_utorrent()));
  ui->toolBar->addAction(QIcon(":/res/add-server.png"), tr("Add server"), this, SLOT(newIp()));
  //ui->toolBar->addAction(QIcon(":/res/utorrent.png"), "", this, SLOT(show_utorrent()));
  ui->toolBar->addSeparator();
  ui->toolBar->addAction(QIcon(":/res/shutdown.png"), "", this, SLOT(appExit()))->setToolTip(tr("Exit"));
}

Cloud::~Cloud()
{
  delete ui;
}
