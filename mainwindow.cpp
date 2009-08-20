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


#include <QtGui>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QTimer>

#ifdef Q_OS_WIN
#include <windows.h>
#include <tchar.h>
#include <psapi.h>
#include <winreg.h>
#endif

#ifdef Q_OS_UNIX
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#endif

#include "mainwindow.h"



void Window::logText(QString txt)
{
    *logStream << QString("[%1] %2\n").arg(QDateTime::currentDateTime().toString("hh:mm:ss.zzz")).arg(txt);
    logStream->flush();
}

Window::Window()
{
    bool ok;

    QSettings settings(QSettings::IniFormat, QSettings::SystemScope, "Genie","settings", this);
    settings.beginGroup("General");

    // initialize settings
    verbose = settings.value("verboseMode", false).toBool();

    //settings.setValue("init", 1);

    reportInterval = settings.value("reportInterval", 5 * 60 * 1000).toInt(&ok);
    serverAddress = settings.value("reportServer", "trulabs.truman.edu").toString();
    serverPort = settings.value("reportPort", 80).toInt(&ok);
    serverReportURL = settings.value("reportPath", "/geniemon/client-ping.php").toString();

    settings.endGroup();
    settings.beginGroup(tr("Branding"));

    programName =  settings.value("programName", "Trulabs Client").toString();
    greetingMessage = settings.value(tr("greetingMessage"), "Some anonymous, non-sensitive data will be sent to ITS.\nClick here to find out more.\r").toString();
    aboutMessage = settings.value("aboutMessage", "To better serve all Truman students by understanding application usage,\ncomputer availability and program names will be reported to ITS every several minutes.\nThe data is for collected for statistical purposes only.\nNo identifiable information, personal data or web usage information will be collected and reported by this program.\n\nFor more information, please visit http://trulabs.truman.edu/").toString();
    stopReportingMessage = settings.value("stopReportingMessage", "Your usage information will no longer be reported").toString();
    greetingBalloon = settings.value("greetingBalloon", false).toBool();
    vmwareViewMode = settings.value("vmwareViewMode", true).toBool();


    if (verbose)
    {
        // Inititalize the text stream to write log
        QString logFilename = settings.value("logFilename", "Y:\\My Documents\\genieDebug.log").toString();
        logFile = new QFile(logFilename);
        if (!logFile->open(QIODevice::Append | QIODevice::Text))
            QMessageBox::information(this, programName, "Genie can't open log file to write debug info. You can proceed, but I can't be sure I will run right. Please contact your sysadmin.");

        logStream = new QTextStream(logFile);

        logText(QString("Hi. This is Genie 1.2."));
        logText(QString("Reporting to hostname %1:%2%3").arg(serverAddress).arg(serverPort).arg(serverReportURL));
    }

    settings.endGroup();

    createActions();
    createTrayIcon();

    //connect(trayIcon, SIGNAL(messageClicked()), this, SLOT(messageClicked()));
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));

    trayIcon->show();
    showMessage();

    manager = new QNetworkAccessManager(this);

    connect(manager, SIGNAL(finished(QNetworkReply*)),
         this, SLOT(networkManagerFinished(QNetworkReply*)));

    computerName = GetCurrentComputerName();
    reportEvent(QString("logon"), computerName);

    // Create a report Timer
    reportTimer = new QTimer(this);
    connect(reportTimer, SIGNAL(timeout()), this, SLOT(reportStatistics()));
    reportTimer->start(reportInterval);
}


Window::~Window()
{
    reportEvent(QString("logoff"), computerName);
    if (verbose) logText(QString("QUIT: Genie is finished.\n--\n"));
    logFile->close();
    delete manager;
    delete logStream;
    delete logFile;
}

void Window::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::Trigger:
        // messageClicked();
        break;
    case QSystemTrayIcon::DoubleClick:
        break;
    case QSystemTrayIcon::MiddleClick:
        //showMessage();
        if (verbose) reportStatistics(true);
        break;
    default:
        ;
    }
}


void Window::showMessage()
{
    if (greetingBalloon) {
        QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::MessageIcon(1);
        trayIcon->showMessage(programName,
                              greetingMessage, icon, 5 * 1000);
    }
}


void Window::messageClicked()
{
    QMessageBox::information(this, programName, aboutMessage);
}

void Window::reportProblems()
{
}


void Window::createActions()
{
    quitAction = new QAction(tr("&Quit"), this);
    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));

    aboutAction = new QAction(tr("&About..."), this);
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(messageClicked()));

    reportProblemsAction = new QAction(tr("&Report Problems..."), this);
    connect(reportProblemsAction, SIGNAL(triggered()), this, SLOT(reportProblems()));
}


void Window::createTrayIcon()
{
    QIcon *icon = new QIcon(":/genie.png");

    trayIconMenu = new QMenu(this);

    //trayIconMenu->addAction(reportProblemsAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(aboutAction);

    if (verbose) trayIconMenu->addAction(quitAction);

    trayIcon = new QSystemTrayIcon(this);

    trayIcon->setIcon(*icon);
    trayIcon->setContextMenu(trayIconMenu);
}

void Window::reportStatistics()
{
    reportStatistics(true);
}

void Window::reportEvent(QString eventStr, QString computerNameStr)
{
    QString postList = QString("computerName=").append(computerNameStr);

    postList.append("&type=event&data=").append(eventStr);

    QByteArray postData = QUrl::toPercentEncoding(postList, "&=");

    QNetworkRequest request;
    request.setUrl(QUrl(QString("http://%1:%2%3").arg(serverAddress).arg(serverPort).arg(serverReportURL)));
    setupRequest(request);
    if (verbose) logText(QString("EVENT [%1 %2]: %3").arg(computerNameStr).arg(eventStr).arg(postList));
    manager->post (request, postData);
}

void Window::setupRequest(QNetworkRequest& req)
{
    req.setRawHeader("User-Agent", "GenieUpdateClient/1.1");
    req.setRawHeader("GenieAPILevel", "2");
    // req.setRawHeader("ClientHostName", GetCurrentComputerName().toUtf8());

}

void Window::reportStatistics(bool includeProcesses)
{
    QString currentComputerName = GetCurrentComputerName();

    // Try to detect station switching.

    if ((currentComputerName.compare(QString("")) == 0) && (computerName.compare(QString("")) == 0)) {
        // sit there do nothing
        // because the current virtual station is not connected to any real station.
    }
    else if (currentComputerName.compare(computerName) == 0) // If the computer name has not been changed.
    {
        QString postList = QString("computerName=").append(currentComputerName);

        if (includeProcesses) {

            #ifdef Q_OS_WIN
            postList.append("&type=processList&data=");

            DWORD aProcesses[1024], cbNeeded, cProcesses;
            unsigned int i;

            if ( !EnumProcesses( aProcesses, sizeof(aProcesses), &cbNeeded ) )
                return;
            // Calculate how many process identifiers were returned.

            cProcesses = cbNeeded / sizeof(DWORD);

            bool first = true;
            for ( i = 0; i < cProcesses; i++ ) {
                if( aProcesses[i] != 0 ) {
                    QString processName = GetProcessName( aProcesses[i] );
                    if (processName.compare("<unknown>")) {
                        if (!first) {
                            postList.append("|");
                        } else {
                            first = false;
                        }

                        postList.append(processName);
                    }
                }
            }
            #endif

        } else {
           // else we can just report availbility
        }

        QByteArray postData = QUrl::toPercentEncoding(postList, "&=");

        QNetworkRequest request;
        request.setUrl(QUrl(QString("http://%1:%2%3").arg(serverAddress).arg(serverPort).arg(serverReportURL)));
        setupRequest(request);
        if (verbose) logText(QString("PROGSTAT: %1").arg(postList));
        manager->post (request, postData);
    }
    else if ((currentComputerName.compare(QString("")) == 0) && (computerName.compare(QString("")) != 0))
    {
        // It was connected to some station, but is disconnected now
        reportEvent(QString("logoff"), computerName);
    }
    else if ((computerName.compare(QString("")) == 0) && (currentComputerName.compare(QString("")) != 0))
    {
        // It was not connected to anything, but is connected now
        reportEvent(QString("logon"), currentComputerName);
    }
    else if ((computerName.compare(QString("")) != 0) && (currentComputerName.compare(QString("")) != 0) && (currentComputerName.compare(computerName) != 0))
    {
        // Logged off from some station, logged on on another
        reportEvent(QString("logoff"), computerName);
        reportEvent(QString("logon"), currentComputerName);
    }

    computerName = currentComputerName;
}

void Window::networkManagerFinished(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError)
    {
        if (verbose) logText(QString("SVR_ERROR: %1").arg(reply->errorString()));
    }

    if (!verbose) reportTimer->start(reportInterval);
}


QString Window::GetCurrentComputerName()
{
  QString retVal;

  #ifdef Q_OS_WIN
    if (vmwareViewMode)
    {
        HKEY hKey;
        DWORD dwSize = 256;
        TCHAR keyValue[256];
        if (RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Volatile Environment"), 0, KEY_READ,&hKey)!= ERROR_SUCCESS)
        {
            // Can't find it
            retVal = QString("");
        }

        long err_ret = RegQueryValueEx(hKey, TEXT("ViewClient_Machine_Name"), NULL, NULL, (LPBYTE)keyValue, &dwSize);

        RegCloseKey(hKey);

        if (err_ret) retVal = QString("");

        #ifdef UNICODE
            retVal = QString::fromUtf16((ushort*)keyValue);
        #else
            retVal = QString::fromLocal8Bit(keyValue);
        #endif
    }
    else // runs on a real station, needs to query the computer name from Win32 API
    {
      TCHAR chrComputerName[MAX_COMPUTERNAME_LENGTH + 1];
      DWORD dwBufferSize = MAX_COMPUTERNAME_LENGTH + 1;

      if(GetComputerName(chrComputerName,&dwBufferSize)) {
        // We got the name, set the return value.
          #ifdef UNICODE
            retVal = QString::fromUtf16((ushort*)chrComputerName);
          #else
            retVal = QString::fromLocal8Bit(chrComputerName);
          #endif
      }
    }
  #endif

  #ifdef Q_OS_UNIX
  char * hostname;

  int hresult = gethostname(hostname, 100);

  if (hresult == 0)
  {
    retVal = QString(hostname);
  }
  #endif

  return retVal;
}


QString Window::GetProcessName( DWORD processID )
{
    #ifdef Q_OS_WIN
    QString qstrMessage;

    TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");

    // Get a handle to the process.

    HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION |
                                   PROCESS_VM_READ,
                                   FALSE, processID );

    // Get the process name.

    if (NULL != hProcess )
    {
        HMODULE hMod;
        DWORD cbNeeded;

        if ( EnumProcessModules( hProcess, &hMod, sizeof(hMod),
             &cbNeeded) )
        {
            GetModuleBaseName( hProcess, hMod, szProcessName,
                               sizeof(szProcessName)/sizeof(TCHAR) );
        }
    }

        // Print the process name and identifier.

      #ifdef UNICODE
      qstrMessage = QString::fromUtf16((ushort*)szProcessName);
      #else
      qstrMessage = QString::fromLocal8Bit(szProcessName);
      #endif

      //if (qstrMessage.compare("<unknown>"))
      //{
      //        QMessageBox::information(this, tr("ITS Experience Improvement Program"),
      //                          qstrMessage );
      //}

    CloseHandle( hProcess );

    return qstrMessage;
    #endif

}


