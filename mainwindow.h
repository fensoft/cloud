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

namespace Ui {
    class MainWindow;
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
  unsigned long current;
  unsigned long max;
  int state;
  int pos;
} sdl;

typedef struct
{
  QString name;
  QString folder;
  QString icon;
  QString hash;
  QStringList files;
} game;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QSettings* set, QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    void updateContent();
    int currentItem;
    QSettings* setGlobal;
    //QSettings* setTmp;
    void ut_get_file();
    static QBuffer httpBuf;
    static QBuffer httpBufStatus;
    QStringList hashes;
    QMap<int, QTreeWidgetItem*> items;
    QMap<QString, dl> dllist;
    void dl_set_prio(int prio, QString = "localhost");
    QList<int> finished;
    QList<int> installed;
    QHttp* http;
    QHttp* httpStatus;
    QProcess utorrent;
    //QProcess unzip;
    MyUnzip unzip2;
    int current_v;
    int current_i;
    int current_state;
    int current_total;
    int current_num;
    QProcess p;
    void do_item(int h);
    void add_shortcuts(int item);
    void dl_progress(int h, sdl* total);
    //QTimer ugfd;
    int changed;
    QTimer httptimer;
    int first_time;
    unsigned long long int max;
    void dl_startstop_torrent(QString start);
    void readIni(QString ini);
    QList<game> games;
    int current_torrent;

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
    void dl_get();
    void dl_reset();
    void ut_status_done_timer();
    void ut_get_file_done_timer();
    void ut_get_file_done();
    void ut_status_done();
    void refresh_log();
    void refreshUnzipProgress(unsigned long long int d);
    void refreshUnzipProgressMax(unsigned long long int d);
    void refreshUnzipProgressEnd();
    void dl_start_torrent();
    void dl_stop_torrent();
    void show_utorrent();
    void appExit();
};

#endif // MAINWINDOW_H
