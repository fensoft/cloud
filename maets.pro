QT       += core gui xml network webkit
TARGET   = ../Cloud
TEMPLATE = app
SOURCES += main.cpp\
           #minizip/zip.c \
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
    addserver.cpp \
    cloud-torrent.cpp \
    torrent.cpp

HEADERS  += cloud.h \
            myunzip.h \
            bootloader.h \
    addserver.h \
    torrent.h

#HEADERS += qjson/src/json_parser.hh \
#  qjson/src/json_scanner.h \
#  qjson/src/location.hh \
#  qjson/src/parser_p.h  \
#  qjson/src/position.hh \
#  qjson/src/qjson_debug.h  \
#  qjson/src/stack.hh \
#  qjson/src/parser.h \
#  qjson/src/parserrunnable.h \
#  qjson/src/qobjecthelper.h \
#  qjson/src/serializer.h \
#  qjson/src/serializerrunnable.h \
#  qjson/src/qjson_export.h

#SOURCES += \
#  qjson/src/json_parser.cc \
#  qjson/src/json_scanner.cpp \
#  qjson/src/parser.cpp \
#  qjson/src/parserrunnable.cpp \
#  qjson/src/qobjecthelper.cpp \
#  qjson/src/serializer.cpp \
#  qjson/src/serializerrunnable.cpp

#CONFIG += create_prl
#DEFINES += QJSON_MAKEDLL

FORMS    += cloud.ui \
            bootloader.ui \
    addserver.ui

OTHER_FILES += config.ini maets.rc \
    qjson/src/json_parser.yy
RC_FILE      = maets.rc
INCLUDEPATH += zlib/include
#LIBS        += zlib/lib/zdll.lib
RESOURCES   += res.qrc
