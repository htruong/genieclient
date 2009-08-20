/*
 * ---------------------------------------------------
 * Copyright (C) 2009 Huan Truong and Amit Shrestha (hnt7438,as043@truman.edu)
 * ---------------------------------------------------
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */


#ifndef WINDOW_H
#define WINDOW_H

#include <QSystemTrayIcon>
#include <QDialog>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFile>
#include <QTextStream>
#include <QNetworkAccessManager>


#ifdef Q_OS_WIN
#include <windows.h>
#endif


QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
QT_END_NAMESPACE

//! [0]
class Window : public QDialog
{
    Q_OBJECT

public:
    Window();
    ~Window();

private slots:
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void showMessage();
    void messageClicked();
    void reportProblems();
    void reportEvent(QString eventStr, QString computerNameStr);
    void reportStatistics(bool includeProcesses);
    void reportStatistics();
    void networkManagerFinished(QNetworkReply *reply);
    void setupRequest(QNetworkRequest &req);
    void logText(QString txt);

private:

    void createActions();
    void createTrayIcon();
    #ifdef Q_OS_WIN
    QString GetProcessName( DWORD processID );
    #endif
    QString GetCurrentComputerName();


    QAction *aboutAction;
    QAction *stopReportingAction;
    QAction *reportProblemsAction;
    QAction *quitAction;

    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;
    QTimer *reportTimer;

    QNetworkAccessManager *manager;

    int reportInterval;
    bool greetingBalloon;

    QString serverAddress;
    int serverPort;
    QString serverReportURL;

    bool verbose;
    bool vmwareViewMode;
    // Runs in VmWare view or not
    // Can't have a better solution for this.

    QString computerName;
    QString programName;
    QString greetingMessage;
    QString aboutMessage;
    QString stopReportingMessage;

    QFile *logFile;
    QTextStream *logStream;

};
//! [0]

#endif
