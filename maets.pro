QT       += core gui xml network
TARGET   = ../Cloud
TEMPLATE = app
SOURCES += main.cpp\
           mainwindow.cpp \
           minizip/zip.c \
           minizip/unzip.c \
           #minizip/miniunz.c \
           minizip/iowin32.c \
           minizip/ioapi.c \
           myunzip.cpp \
           bootloader.cpp
HEADERS  += mainwindow.h \
            myunzip.h \
            bootloader.h

FORMS    += mainwindow.ui \
            bootloader.ui

OTHER_FILES += config.ini maets.rc
RC_FILE      = maets.rc
INCLUDEPATH += zlib/include
#LIBS        += zlib/lib/zdll.lib
RESOURCES   += res.qrc
