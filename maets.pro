QT       += core gui xml network
TARGET   = ../Cloud
TEMPLATE = app
SOURCES += main.cpp\
           minizip/zip.c \
           minizip/unzip.c \
           minizip/iowin32.c \
           minizip/ioapi.c \
           myunzip.cpp \
           bootloader.cpp \
    cloud-download.cpp \
    cloud-tools.cpp \
    cloud-install.cpp \
    cloud-run.cpp \
    cloud.cpp \
    addserver.cpp

HEADERS  += cloud.h \
            myunzip.h \
            bootloader.h \
    addserver.h

FORMS    += cloud.ui \
            bootloader.ui \
    addserver.ui

OTHER_FILES += config.ini maets.rc
RC_FILE      = maets.rc
INCLUDEPATH += zlib/include
#LIBS        += zlib/lib/zdll.lib
RESOURCES   += res.qrc
