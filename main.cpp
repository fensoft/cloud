#include <QtGui/QApplication>
#include "cloud.h"
#include <QFile>
#include "bootloader.h"
#include <QEventLoop>
#include <QProcess>
#include <qdebug.h>
#include <QSettings>

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  QSettings* settings = new QSettings("config.ini", QSettings::IniFormat);

  qDebug() << "test" << settings->value("Global/Webserverarg").toString().arg(QCoreApplication::applicationDirPath().replace("/", "\\"));

  QProcess* p = new QProcess();
  p->start(QCoreApplication::applicationDirPath() + "/" + settings->value("Global/Webserver").toString(),
           QStringList(settings->value("Global/Webserverarg").toString().arg(QCoreApplication::applicationDirPath().replace("/", "\\"))));

  BootLoader b(settings);
  b.show();
  b.process();
  b.exec();

  Cloud w(settings);
  w.show();

  return a.exec();
}
