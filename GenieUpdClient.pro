# -------------------------------------------------
# Project created by QtCreator 2009-06-11T08:55:29
# -------------------------------------------------
QT += network #\
    #xml
# DEFINES         += WINVER=0x0501 _WIN32_WINNT=0x0501
TARGET = GenieUpdClient
TEMPLATE = app
SOURCES += main.cpp \
    mainwindow.cpp
ICON = genie.icns
QMAKE_INFO_PLIST = Genie.plist
HEADERS += mainwindow.h
FORMS += 
RESOURCES += GenieUpdClient.qrc
RC_FILE = GenieUpdClient.rc
win32:LIBS += "c:\Program Files\Microsoft SDKs\Windows\v7.0\Lib\Psapi.Lib" #"C:\Program Files\Microsoft SDKs\Windows\v6.0A\Lib\User32.Lib"
win32:LIBS += "c:\Program Files\Microsoft SDKs\Windows\v7.0\Lib\Advapi32.Lib"
# CONFIG += console
