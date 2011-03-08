#ifndef MYUNZIP_H
#define MYUNZIP_H
#include <QThread>
#include <QObject>
#include <QStringList>

class MyUnzip : public QThread
{
  Q_OBJECT
public:
  void run();
  int state();
  void setZip(QString l, QStringList s);
  void setDest(QString s);
  void setSizeBuf(unsigned int size_buf);
signals:
  void end();
  void newState(unsigned long long int);
  void max(unsigned long long int);
private:
  QStringList zip;
  QString dest;
  QString location;
  unsigned int size_buf;
};

#endif // MYUNZIP_H
