#ifndef ADDSERVER_H
#define ADDSERVER_H

#include <QDialog>
#include <QSettings>

namespace Ui {
    class AddServer;
}

class AddServer : public QDialog
{
    Q_OBJECT

public:
    explicit AddServer(QSettings* settings, QWidget *parent = 0);
    ~AddServer();
  QString getIp();

private:
    Ui::AddServer *ui;
};

#endif // ADDSERVER_H
