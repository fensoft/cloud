#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "QListWidgetItem"
#include "QSettings"
#include <QBuffer>
#include "QHttp"
#include <QProcess>
#include "QTreeWidgetItem"
#include <QTimer>
#include "myunzip.h"
#include <QItemDelegate>
#include <QPainter>

#define TRACE(_lev_, _what_) do { if (_lev_ <= 10) qDebug() << QString("%1:%2 %3()").arg(__FILE__).arg(__LINE__).arg(__FUNCTION__) << _what_; } while (0)

namespace Ui {
    class Cloud;
}

typedef struct
{
  unsigned long long current;
  unsigned long long max;
  int state;
  int pos;
} dl;

typedef struct
{
  QString name;
  QString folder;
  QString icon;
  QString hash;
  QString location;
  QStringList files;
} game;

typedef struct
{
  long long down;
  long long up;
  int estimated;
} stats;

typedef enum
{
  torrent_start = 1,
  torrent_stop = 2
} torrent_state;

typedef enum
{
  torrent_prio_no = 0,
  torrent_prio_low = 1,
  torrent_prio_normal = 2,
  torrent_prio_high = 3
} torrent_prio;

class Cloud : public QMainWindow
{
    Q_OBJECT

public:
    explicit Cloud(QSettings* set, QWidget *parent = 0);
    ~Cloud();

private:
    Ui::Cloud *ui;
    void updateContent();
    int currentItem;
    QSettings* setGlobal;
    //QSettings* setTmp;
    void ut_get_file();
    //static QBuffer httpBuf;
    //static QBuffer httpBufStatus;
    QStringList hashes;
    QMap<int, QTreeWidgetItem*> items;
    //QMap<QString, dl> dllist;
    QList<int> finished;
    QList<int> installed;
    QHttp* http;
    QHttp* httpStatus;
    QProcess utorrent;
    //QProcess unzip;
    MyUnzip unzip;

    int current_v;
    int current_i;
    int current_state;
    int current_total;
    int current_num;

    QProcess p;
    void add_shortcuts(int item);
    unsigned long long int unzip_max;
    void readIni(QString ini);
    QList<game> games;
    int current_torrent;
    QTimer* timer_ui;
    void torrent_startApp();
    void torrent_showApp();
    void torrent_setState(torrent_state i);
    void torrent_setPrio(torrent_prio i, int gameid, QString ip = "localhost");
    void torrent_setPrioAll(torrent_prio i, int gameid);
    QMap<QString, dl> torrent_getInfo(QStringList hashes);
    stats torrent_getStats();

private slots:
    void on_list_itemDoubleClicked(QTreeWidgetItem* item, int column);
    void on_list_customContextMenuRequested(QPoint pos);
    void dl_start();
    void dl_start_low();
    void dl_start_all();
    void dl_stop_all();
    void dl_start_low_all();
    void dl_stop();
    void dl_reconfigure();
    void dl_reinstall();
    void ui_refresh();
    void dl_reset();
    void ui_list_update(QMap<QString, dl> dllist);
    void refreshUnzipProgress(unsigned long long int d);
    void refreshUnzipProgressMax(unsigned long long int d);
    void refreshUnzipProgressEnd();
    void dl_start_torrent();
    void dl_stop_torrent();
    void show_utorrent();
    void appExit();
    void newIp();
};

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

#define SET_ITEM(__item__, __string__, __state__, __pos__, __color__) \
if (1) \
{ \
  if (__item__) \
  { \
    __item__->setText(0, __string__);\
    if (__state__ != __item__->text(1)) \
    { \
      local_changed = 1; \
      /*ui->list->resizeColumnToContents(0);*/ \
      /*ui->list->resizeColumnToContents(1);*/ \
      /*ui->list->sortItems(1, Qt::AscendingOrder);*/ \
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


#endif // MAINWINDOW_H
