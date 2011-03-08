#ifndef BOOTLOADER_H
#define BOOTLOADER_H

#include <QDialog>
#include <QHttp>
#include <QBuffer>
#include <QSettings>

namespace Ui {
    class BootLoader;
}

class BootLoader : public QDialog
{
    Q_OBJECT

public:
    explicit BootLoader(QSettings* set, QWidget *parent = 0);
    ~BootLoader();
    QMap<QString, QBuffer*> httpBuf;
    int nbfil;
    int nbmac;
    QSettings* settings;
    QStringList ips;
    QStringList processed;
private:
    Ui::BootLoader *ui;
signals:
    void finished();
public slots:
    void process();
    void processExisting();
};

#endif // BOOTLOADER_H
