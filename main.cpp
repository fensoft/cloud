#include <QtGui/QApplication>
#include "cloud.h"
#include <QFile>
#include "bootloader.h"
#include <QEventLoop>
#include <QProcess>
#include <qdebug.h>
#include <QSettings>
#include <QDir>
#include "torrent.h"

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  QSettings* settings = new QSettings("config.ini", QSettings::IniFormat);
  QDir(".").mkdir(settings->value("Global/Folder").toString());
  QDir(".").mkdir(settings->value("Global/Database").toString());
  QDir(".").mkdir(settings->value("Global/WebserverRoot").toString());


  QProcess* p = new QProcess();
  QStringList args;
  args << settings->value("Global/Webserverarg").toString().arg(QCoreApplication::applicationDirPath().replace("/", "\\"));
  args << settings->value("Global/Webserverport").toString();
  p->start(QCoreApplication::applicationDirPath() + "/" + settings->value("Global/Webserver").toString(),
           args);

  BootLoader b(settings);
  b.show();
  b.process();
  b.exec();

  Cloud w(settings);
  w.show();

//  Torrent t(settings, 0, 0);
//  t.getInfo(QStringList("3023DD766169B49F402F248F66B783BD90E6AECB"));

  return a.exec();
}
