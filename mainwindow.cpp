#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QSettings"
#include "QDebug"
#include "QMenu"
#include <QTimer>
#include <QHttp>
#include <QBuffer>
#include <QList>
#include <QFile>
#include <QtGui>

QBuffer MainWindow::httpBuf;
QBuffer MainWindow::httpBufStatus;

class ItemDelegate: public QItemDelegate
{
public:
  ItemDelegate(QObject* parent = 0): QItemDelegate(parent)
  {
  }

  void paint(QPainter* painter,
             const QStyleOptionViewItem& option,
             const QModelIndex& index) const
  {
    QItemDelegate::paint(painter, option, index);
    if (index.data(Qt::DisplayRole).toString().contains("/"))
    {
      QRegExp re("([^ ]) ([^/]*)/(.*)");
      re.indexIn(index.data(Qt::DisplayRole).toString());
      float b  = re.cap(2).toFloat();
      float e  = re.cap(3).toFloat();
      painter->drawRect(option.rect.x(),
                        option.rect.y(),
                        option.rect.width()-1,
                        option.rect.height()/5);
      painter->fillRect(option.rect.x()+1,
                        option.rect.y()+1,
                        (option.rect.width()-2)*((float)b/(float)e),
                        (option.rect.height()/5)-1, Qt::green);
    }
  }
};

MainWindow::MainWindow(QSettings* set, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
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
      qDebug() << "will read" << game;
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
        //utorrent.start(setGlobal->value("Global/uTorrentExe").toString(), qsl);
      }
      first_time = 1;
      //QTimer::singleShot(5000, this, SLOT(dl_reset()));
    }
    else
    {
      /*if (set->value("Global/uTorrent").toInt() == 1)
        utorrent.start(setGlobal->value("Global/uTorrentExe").toString(), QStringList("/minimized"));*/
    }

    //connect(&ugfd, SIGNAL(timeout()), this, SLOT(ut_get_file_done()));

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
    ui->toolBar->addAction(QIcon(":/res/slow.png"), "Ralentir", this, SLOT(show_utorrent()));
    ui->toolBar->addAction(QIcon(":/res/stop.png"), "", this, SLOT(dl_stop_torrent()));
    ui->toolBar->addAction(QIcon(":/res/eject.png"), "Annuler tout", this, SLOT(dl_reset()));
    ui->toolBar->addSeparator();
    ui->toolBar->addAction(QIcon(":/res/add.png"), "", this, SLOT(show_utorrent()));
    ui->toolBar->addAction(QIcon(":/res/utorrent.png"), "", this, SLOT(show_utorrent()));
    ui->toolBar->addSeparator();
    ui->toolBar->addAction(QIcon(":/res/shutdown.png"), "", this, SLOT(appExit()));
}

void MainWindow::appExit()
{
  QProcess* p = new QProcess();
  p->start(setGlobal->value("Global/CloseApp").toString(),
           QStringList(QString(
               QCoreApplication::applicationDirPath() + "/" + setGlobal->value("Global/uTorrentExe").toString()
               ).replace("/", "\\")));
  p = new QProcess();
  p->start(setGlobal->value("Global/CloseApp").toString(), QStringList(setGlobal->value("Global/uTorrentExe").toString()));
  p = new QProcess();
  p->start(setGlobal->value("Global/CloseApp").toString(), QStringList("P:\\Dev\\Qt\\2010.04\\maets\\TINY.EXE"));
  //p->waitForFinished(300);
  //p->start(setGlobal->value("Global/CloseApp").toString(), QStringList(setGlobal->value("Global/uTorrentExe").toString()));
  //p->waitForFinished(750);
  close();
}

void MainWindow::show_utorrent()
{
  QProcess::startDetached(setGlobal->value("Global/uTorrentExe").toString(), QStringList("/bringtofront"));
}

void MainWindow::dl_reset()
{
  qDebug() << "begin dl_reset";
  for (int i =1; i != 255; i++)
  {
    currentItem = i;//QString("item%1").arg(i);
    dl_set_prio(0);
  }
  qDebug() << "end dl_reset";
}

MainWindow::~MainWindow()
{
  //utorrent.kill();
  //utorrent.terminate();
  delete ui;
}

//#define V(_v_) setTmp->value(item + "/" + _v_).toString()
void MainWindow::readIni(QString ini)
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

void MainWindow::updateContent()
{
  qDebug() << "begin updateContent";
//  ui->list->setContextMenuPolicy(Qt::CustomContextMenu);
//  for (int i = 1; i <= setTmp->value("Global/NbItems").toInt(); i++)
//  {
//    QString item = QString("item%1").arg(i);
//    if (!games.at(item).name.isEmpty())
//    {
//      qDebug() << "will add";
//      QTreeWidgetItem* lwitem = new QTreeWidgetItem();
//      lwitem->setText(0, games.at(item).name);
//      QFont f = lwitem->font(0);
//      f.setBold(true);
//      f.setPixelSize(14);
//      lwitem->setFont(0, f);
//      lwitem->setIcon(0, QIcon(QString("icons/%1").arg(V("icon"))));
//      lwitem->setData(0, Qt::UserRole, item);
//      lwitem->setData(0, Qt::UserRole + 1, 1);
//      ui->list->addTopLevelItem(lwitem);
//      items[item] = lwitem;
//    }
//    else
//    {
//      qDebug() << "name empty";
//      continue;
//    }
//  }
  ui->list->setContextMenuPolicy(Qt::CustomContextMenu);
  for (int i = 0; i < games.size(); i++)
  {
    //QString item = QString("item%1").arg(i);
    //if (!games.at(item).name.isEmpty())
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
    /*else
    {
      qDebug() << "name empty";
      continue;
    }*/
  }
  qDebug() << "end updateContent";
}

void MainWindow::on_list_customContextMenuRequested(QPoint pos)
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
        menu.addAction(QIcon(), "&Démarrer le téléchargement puis installer", this, SLOT(dl_start()));
        menu.addAction(QIcon(), "&Démarrer le téléchargement seulement", this, SLOT(dl_start_low()));
        menu.addAction(QIcon(), "&Arrêter le téléchargement", this, SLOT(dl_stop()));
        menu.addAction(QIcon(), "&Réinstaller", this, SLOT(dl_reinstall()));
      }
      menu.addAction(QIcon(), "&Configurer", this, SLOT(dl_reconfigure()));
      if (setGlobal->value("Global/isAdmin").toInt() == 1)
      {
        menu.addAction(QIcon(), "&Démarrer le téléchargement puis installer POUR TOUS", this, SLOT(dl_start_all()));
        menu.addAction(QIcon(), "&Démarrer le téléchargement seulement POUR TOUS", this, SLOT(dl_start_low_all()));
        menu.addAction(QIcon(), "&Arrêter le téléchargement POUR TOUS", this, SLOT(dl_stop_all()));
      }
      menu.exec(QCursor::pos());
    }
  }
}

void MainWindow::dl_start_low_all()
{
  for (int i = 1; i != 254; i++)
    dl_set_prio(1, QString("%1%2").arg(setGlobal->value("Global/uTorrentIPs").toString()).arg(i));
}

void MainWindow::dl_start_all()
{
  for (int i = 1; i != 254; i++)
    dl_set_prio(2, QString("%1%2").arg(setGlobal->value("Global/uTorrentIPs").toString()).arg(i));
}

void MainWindow::dl_stop_all()
{
  for (int i = 1; i != 254; i++)
    dl_set_prio(0, QString("%1%2").arg(setGlobal->value("Global/uTorrentIPs").toString()).arg(i));
}

void MainWindow::dl_start_low()
{
  dl_set_prio(1);
}

void MainWindow::dl_start()
{
  dl_set_prio(2);
}

void MainWindow::dl_stop()
{
  dl_set_prio(0);
}

void MainWindow::dl_reinstall()
{
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
}

void MainWindow::dl_reconfigure()
{
  int item = currentItem;
  QString fol = games.at(item).folder.isEmpty() ? games.at(item).name : games.at(item).folder;
  QString cmd = QString("%1/%2/%3/%4").arg(QApplication::applicationDirPath(), setGlobal->value("Global/Folder").toString(), fol, "install.cmd");
  QString fol2 = QString("%1/%2/%3").arg(QApplication::applicationDirPath(), setGlobal->value("Global/Folder").toString(), fol);
  qDebug() << "reconfigure" << cmd << "in" << fol2;
  QProcess::startDetached(cmd, QStringList(), fol2);
}

void MainWindow::dl_start_torrent()
{
  dl_startstop_torrent("start");
}

void MainWindow::dl_stop_torrent()
{
  dl_startstop_torrent("stop");
}


void MainWindow::dl_startstop_torrent(QString start)
{
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
  //for (int i = 0; i != 255; i++)
  {
    //QString item = currentItem;
    //if (!V(QString("file%1").arg(i)).isEmpty())
    {
      //qDebug() << V(QString("file%1").arg(i));
      QString test = QString("/gui/?action=%1&hash=%2&").arg(start).arg(sl.at(i));
      //qDebug() << test << dllist[V(QString("file%1").arg(i))].pos;
      qDebug() << /*ip << */test;
      httpget->get(test, httpBufLocal);
    }
  }
}

void MainWindow::dl_set_prio(int prio, QString ip)
{
  qDebug() << "begin dl_set_prio";
  QBuffer* httpBufLocal = new QBuffer(this);
  httpBufLocal->open(QIODevice::ReadWrite);
  QHttp* httpget = new QHttp();
  httpget->setHost(ip, setGlobal->value("Global/uTorrentPort").toInt());
  httpget->setUser(setGlobal->value("Global/uTorrentAdminLogin").toString(), setGlobal->value("Global/uTorrentAdminPassword").toString());

  qDebug() << "dl_start" << currentItem;
  for (int i = 0; i != 255; i++)
  {
    int item = currentItem;
    //if (!V(QString("file%1").arg(i)).isEmpty())
    for (int j = 0; j < games.at(item).files.count(); j++)
    {
      //qDebug() << V(QString("file%1").arg(i));
      QString test = QString("/gui/?action=setprio&hash=%3&p=%2&f=%1").arg(dllist[games.at(item).files.at(j)].pos).arg(prio).arg(games.at(item).hash);
      //qDebug() << test << dllist[V(QString("file%1").arg(i))].pos;
      qDebug() << ip << test;
      httpget->get(test, httpBufLocal);
    }
  }
  qDebug() << "end dl_set_prio";
}

void MainWindow::dl_get()
{
  ut_get_file();
}

void MainWindow::ut_get_file()
{
  //qDebug() << "begin ut_get_file";
  if (games.size() == 0)
    return;
  //qDebug() << "ugf";
  httpBuf.open(QIODevice::ReadWrite | QIODevice::Truncate);
  httpBufStatus.open(QIODevice::ReadWrite | QIODevice::Truncate);
  http->get(QString("/gui/?action=getfiles&hash=%1").arg(games.at(current_torrent).hash), &httpBuf);
  httpStatus->get(QString("/gui/?list=1"), &httpBufStatus);
  current_torrent = (current_torrent + 1) % hashes.size();
  //qDebug() << "end ut_get_file";
}

#define SET_ITEM(__item__, __string__, __state__, __pos__, __color__) \
if (1) \
{ \
  if (__item__) \
  { \
    __item__->setText(0, __string__);\
    if (__state__ != __item__->text(1)) \
    { \
      /*ui->list->resizeColumnToContents(0);*/ \
      /*ui->list->resizeColumnToContents(1);*/ \
      /*ui->list->sortItems(1, Qt::AscendingOrder);*/ \
      changed = setGlobal->value("Global/NonSleepRefreshTime").toInt(); \
      local_changed = 1; \
      /*qDebug() << "changed";*/ \
    } \
    __item__->setText(1, __state__); \
    __item__->setText(2, QVariant(__pos__).toString()); \
    __item__->setForeground(0, QBrush(__color__)); \
    __item__->setForeground(1, QBrush(__color__)); \
  } \
  else \
    qDebug() << "not found"; \
} \
else

void MainWindow::ut_get_file_done_timer()
{
  //qDebug() << "begin ut_get_file_done_timer";
  //qDebug() << "ugfdt";
  changed = ((changed >= setGlobal->value("Global/SleepRefreshTime").toInt()) ? (setGlobal->value("Global/SleepRefreshTime").toInt()) : (changed *2));
  QTimer::singleShot(changed, this, SLOT(ut_get_file_done()));
  //httptimer.setInterval(changed);
  //ut_get_file_done();
  //qDebug() << "end ut_get_file_done_timer";
}

void MainWindow::ut_status_done_timer()
{
  //qDebug() << "begin ut_status_done_timer";
  //changed = ((changed >= set->value("Global/SleepRefreshTime").toInt()) ? (set->value("Global/SleepRefreshTime").toInt()) : (changed *2));
  QTimer::singleShot(changed, this, SLOT(ut_status_done()));
  //httptimer.setInterval(changed);
  //ut_get_file_done();
  //qDebug() << "end ut_status_done_timer";
}

void MainWindow::ut_status_done()
{
  //qDebug() << "begin ut_status_done";
  //int local_changed = 0;
  httpBufStatus.seek(0);
  //int i = 0;
  while (!httpBufStatus.atEnd())
  {
    char buf[1024];
    if (httpBufStatus.readLine(buf, 1024) == 0)
      break;
    QString bufS(buf);
    bufS.replace("[", "");
    bufS.replace("]", "");
    bufS.replace("\"", "");
    //qDebug() << bufS;
    QStringList sl = bufS.split(',');
    //qDebug() << sl.size();
    if (sl.size() == 19)
    {
      long long r = QVariant(sl[8]).toLongLong() / 104857L;
      long long e = QVariant(sl[9]).toLongLong() / 104857L;
      int t = QVariant(sl[10]).toInt() / 60;
      statusBar()->showMessage(QString("E %1,%2 Mo/s\tR %3,%4 Mo/s\tTemps : %5 min").arg(r/10).arg(r%10).arg(e/10).arg(e%10).arg(t));
      //qDebug() << sl[8] << sl[9];
    }
   /* if (sl.size() == 4)
    {
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
      /while (dllist2.remove(sl[0]))
        ;*/
      //dllist.insert(sl[0], c);
      //qDebug() << sl[0] << c.current << c.max << c.state;
    //}
    /*else
      qDebug() << "unrecognized" << sl.size() << sl;*/
  }
   ///qDebug() << "end ut_status_done";
}

void MainWindow::ut_get_file_done()
{
  //qDebug() << "begin ut_get_file_done";
  int local_changed = 0;
  httpBuf.seek(0);
  int i = 0;
  while (!httpBuf.atEnd())
  {
    char buf[1024];
    httpBuf.readLine(buf, 1024);
    QString bufS(buf);
    bufS.replace(QRegExp("^,*"), "");
    //qDebug() << bufS;
    QStringList sl = bufS.split(',');
    if (sl.size() == 4)
    {
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
      /*while (dllist2.remove(sl[0]))
        ;*/
      dllist.insert(sl[0], c);
      //qDebug() << sl[0] << c.current << c.max << c.state;
    }
    /*else
      qDebug() << "unrecognized" << sl.size() << sl;*/
  }
  if (dllist.size() > 5 && first_time == 1)
  {
    first_time = 0;
    dl_reset();
  }
  //httpBuf.setBuffer("");

  //qDebug() << dllist.size();

  //qDebug() << "refresh" << changed << dllist.size();

  //qDebug() << "in ut_get_file_done 1";

  for (int h = 0; h < games.size(); h++)
  {
    //do_item(h);
    int item = h;
    if (item == -1)
      continue;
    if (games.at(item).name.isEmpty())
    {
      //qDebug() << "empty" << h;
      continue;
    }
    sdl total;
    dl_progress(h, &total);

    QString fol = games.at(item).folder.isEmpty() ? games.at(item).name : games.at(item).folder;
    QString desc = QString("%1/%2/" + setGlobal->value("Global/DescFile").toString()).arg(setGlobal->value("Global/Folder").toString(), fol);

    int downloadfinished = total.state && total.current == total.max;
    int mustinstall = total.state >= 2 && total.current == total.max;
    int lowprio = total.state == 1;
    int descfilepresent = QFile::exists(desc);
    int extractfinished = (current_v == -1);
    int shortcutcreated = installed.contains(item) == true;
    int installinprogress = (current_v == item);

    //qDebug() << "in ut_get_file_done 2";

    if (setGlobal->value("Global/uTorrent").toInt() == 1)
    {
      if (total.state == 0)
      {
        SET_ITEM(items[item], QString("%1").arg(games.at(item).name), "pause", 99, Qt::darkGray);
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
      SET_ITEM(items[item], QString("%1").arg(games.at(item).name), "raccourcis", 30, Qt::cyan);
      installed << item;
      add_shortcuts(item);
      continue;
    }

    /*if (downloadfinished)
    {
      if (mustinstall)
      {
        SET_ITEM(items[item], QString("%1").arg(games.at(item).name), "install...", Qt::cyan);
        if (unzip.state() == QProcess::NotRunning)
        {
          current_v = QString("item%1").arg(h);
          current_i = 0;
          refresh_log();
        }
        continue;
      }
      else
      {
        SET_ITEM(items[item], QString("%1").arg(games.at(item).name), "complet", Qt::cyan);
        continue;
      }
    }*/
  }

    if (setGlobal->value("Global/uTorrent") == 1)
    {
      if (downloadfinished)
      {
        if (!extractfinished)
        {
          SET_ITEM(items[item], QString("%1").arg(games.at(item).name), "attente", 40, Qt::cyan);
          continue;
        }
        else
        {
          if (mustinstall)
          {
            SET_ITEM(items[item], QString("%1").arg(games.at(item).name), "install", 30, Qt::cyan);
            current_v = h;
            current_i = 0;
            refresh_log();
            continue;
          }
          else
          {
            SET_ITEM(items[item], QString("%1").arg(games.at(item).name), "terminé", 50, Qt::cyan);
            continue;
          }
        }
      }


      if (total.state)
      {
        if (lowprio)
          SET_ITEM(items[item], QString("%1").arg(games.at(item).name), QString("Reception... %2/%3").arg((unsigned long)total.current).arg((unsigned long)total.max), 55, Qt::gray);
        else
          SET_ITEM(items[item], QString("%1").arg(games.at(item).name), QString("Reception... %2/%3").arg((unsigned long)total.current).arg((unsigned long)total.max), 45, Qt::cyan);
        continue;
      }
    }
    if (setGlobal->value("Global/uTorrent") == 0)
    {
      SET_ITEM(items[item], QString("%1").arg(games.at(item).name), "non installé", 60, Qt::darkGray);
      continue;
    }

    SET_ITEM(items[item], QString("%1").arg(games.at(item).name), "erreur", 100, Qt::red);
    /*if (total.state != 0)
    {
      if (total.current == total.max)
      {

        if (!finished.contains(item))
        {
          if (unzip.state() == QProcess::NotRunning)
          {
            finished << item;
            if (QFile::exists(desc))
            {
              SET_ITEM(items[item], QString("%1").arg(games.at(item).name), Qt::white);
              return;
            }
            items[item]->setForeground(0, QBrush(Qt::cyan));
            items[item]->setText(0, QString("%1 (installation1)").arg(games.at(item).name));
            current_v = QString("item%1").arg(h);
            current_i = 0;
            refresh_log();
            //return;
          }
        }
        if (!installed.contains(item))
        {
          if (QFile::exists(desc) && current_v.isEmpty())
          {
            add_shortcuts(item);
          }
          else
          {
            items[item]->setText(0, QString("%1 (installation2)").arg(games.at(item).name));
            items[item]->setForeground(0, QBrush(Qt::cyan));
          }
        }
        else
        {
          items[item]->setText(0, QString("%1").arg(games.at(item).name));
          items[item]->setForeground(0, QBrush(Qt::white));
        }
      }
      else
      {
        items[item]->setText(0, QString("%1 (%2/%3)").arg(games.at(item).name).arg(total.current / (1024*1024)).arg(total.max / (1024*1024)));
        items[item]->setForeground(0, QBrush(Qt::white));
      }
    }
    else
    {
      if (total.max)
      {
        items[item]->setText(0, QString("%1").arg(games.at(item).name));
        items[item]->setForeground(0, QBrush(Qt::darkGray));
      }
      else
      {
        items[item]->setText(0, QString("%1").arg(games.at(item).name));
        items[item]->setForeground(0, QBrush(Qt::white));
      }
    }*/
  }
  if (local_changed == 1)
  {
    qDebug() << "changed";
    ui->list->sortItems(0, Qt::AscendingOrder);
    ui->list->sortItems(2, Qt::AscendingOrder);
    ui->list->resizeColumnToContents(0);
    ui->list->resizeColumnToContents(1);
  }
  dl_get();
  //qDebug() << "end ut_get_file_done";
}

void MainWindow::add_shortcuts(int item)
{
  qDebug() << "begin add_shortcuts";
  ui->list->sortItems(1, Qt::AscendingOrder);
  QString fol = games.at(item).folder.isEmpty() ? games.at(item).name : games.at(item).folder;
  QString desc = QString("%1/%2/" + setGlobal->value("Global/DescFile").toString()).arg(setGlobal->value("Global/Folder").toString(), fol);
  installed << item;
  //qDebug() << desc;
  QFile file(desc);
  file.open(QFile::ReadOnly);
  while (!file.atEnd())
  {
    QString line = file.readLine();
    if (line.contains("="))
    {
      line = line.left(line.size() - 2);
      QString name = line.split("=")[0];
      QString val = line.mid(line.indexOf("=")+1);//line.split("=")[1];
      QRegExp re("(.*)[.]exe");
      re.setMinimal(true);
      val = val.replace(re, "\\1.exe\"");//\"
      val = val.replace("\\\\", "\\");
      //val = val.replace(QRegExp("(.*)[.]bat"), "\"\\1.bat\"");
      //val = val.replace(QRegExp("(.*)[.]cmd"), "\"\\1.cmd\"");
      QTreeWidgetItem* lwitem = new QTreeWidgetItem();
      items[item]->setForeground(0, QBrush(Qt::white));
      lwitem->setText(0, QString("Lancer %1").arg(name));
      //lwitem->setIcon(0, QIcon(V("icon")));
      lwitem->setData(0, Qt::UserRole, val);
      lwitem->setData(0, Qt::UserRole + 1, 2);
      QString fol = games.at(item).folder.isEmpty() ? games.at(item).name : games.at(item).folder;
      lwitem->setData(0, Qt::UserRole + 2, QString("%1/%2").arg(setGlobal->value("Global/Folder").toString(), fol));
      items[item]->addChild(lwitem);
      ui->list->expandAll();
      //qDebug() << name << val;
    }
  }
  qDebug() << "end add_shortcuts";
}

void MainWindow::dl_progress(int h, sdl* total2)
{
  //qDebug() << "begin dl_progress";
  int item = h;

  dl total;
  total.current = total.max = total.state = 0;
  for (int i = 0; i != games.at(item).files.count(); i++)
  {
    QString filename = games.at(item).files.at(i);//V(QString("file%1").arg(i));
    //if (filename.isEmpty())
    //  continue;
    total.current += dllist[filename].current;
    total.max += dllist[filename].max;
    total2->state = dllist[filename].state;
  }
  total2->current = total.current / 1048576L;
  total2->max = total.max / 1048576L;
  /*if (total->state)
    qDebug() << "state" << total->state;*/
  //qDebug() << "end dl_progress";
}

void MainWindow::do_item(int h)
{

}

void MainWindow::refresh_log()
{
  qDebug() << "begin refresh_log";
  if (!unzip2.isRunning())
  {
    int item = current_v;
    if (item == -1)
      return;
    QStringList files;
    /*while (!V(QString("file%1").arg(++current_i)).isNull())
    {
      QString file = QString("%1/%2").arg(setGlobal->value("Global/Folder").toString(), V(QString("file%1").arg(current_i)));
      qDebug() << "must install via" << file;
      files << file;
      //unzip2.setWorkingDirectory(QApplication::applicationDirPath());
    }*/
    if (games.at(item).files.size())
    {
      unzip2.setZip(games.at(item).files);
      unzip2.setDest(setGlobal->value("Global/Folder").toString());
      unzip2.start(QThread::HighPriority);
      ut_get_file_done();
    }
    else
      refreshUnzipProgressEnd();
    current_i = 0;
  }
  else
  {
    /*finished << current_v;
    current_v = "";*/
  }
  qDebug() << "end refresh_log";
}

void MainWindow::refreshUnzipProgressEnd()
{
  qDebug() << "refreshUnzipProgressEnd";
  finished << current_v;
  current_v = -1;
}

void MainWindow::refreshUnzipProgressMax(unsigned long long int d)
{
  max = d;
}

void MainWindow::refreshUnzipProgress(unsigned long long int d)
{
  qDebug() << "begin refreshUnzipProgress";
  int item = current_v;
  if (current_v == -1)
    return;
  int local_changed;
  SET_ITEM(items[item], QString("%1").arg(games.at(item).name), QString("Installation... %2/%3").arg((unsigned long)(d)).arg((unsigned long)max), 55, Qt::gray);
  qDebug() << d;
  qDebug() << "end refreshUnzipProgress";
}

void MainWindow::on_list_itemDoubleClicked(QTreeWidgetItem* item, int column)
{
  if (item->data(0, Qt::UserRole + 1).toInt() == 1)
  {
    currentItem = item->data(0, Qt::UserRole).toInt();
    dl_set_prio(2);
  }

  if (item->data(0, Qt::UserRole + 1).toInt() == 2)
  {
    qDebug() << "run" << item->data(0, Qt::UserRole + 2).toString() << item->data(0, Qt::UserRole + 1).toString() << item->data(0, Qt::UserRole).toString();
    p.setWorkingDirectory(QString("%1").arg(item->data(0, Qt::UserRole + 2).toString()));
    qDebug() << p.workingDirectory();
    QString cmd = QString("\"%1/%2\"").arg(item->data(0, Qt::UserRole + 2).toString(), item->data(0, Qt::UserRole).toString());
    cmd = QString("%1/%2").arg(item->data(0, Qt::UserRole + 2).toString(), item->data(0, Qt::UserRole).toString());
    cmd = cmd.replace("\\", "/");
    //QString fol = QString("%1/%2").arg(QApplication::applicationDirPath(), QString(cmd).replace(QRegExp("[/][^./]*[.]exe.*"), "").replace("\"", ""));

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
