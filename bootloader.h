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
    QBuffer* httpBuf[256];
    int nbfil;
    int nbmac;
    QSettings* settings;
private:
    Ui::BootLoader *ui;
signals:
    void finished();
public slots:
    void process();
    void processExisting();
};

#endif // BOOTLOADER_H
