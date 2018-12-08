/*
 * This application is free software and is released under the terms of
 * the BSD license. See LICENSE file for details.
 *
 * Copyright (c) 2010 Volker Poplawski (volker@openbios.org)
 */
#include "global.h"

#include <QColor>

QMainWindow*   g_mainWindow = 0;
QString        g_specsDirectory;
QString        g_openFileName;
const uchar*   g_file = 0;
qint64         g_fileSize = 0;
QLabel*        g_blockCountLabel = 0;
AsterixModel*  g_asterixModel = 0;
Uap*           g_uap = 0;
QColor         g_highlightColor = QColor( 225, 235, 250 );
Logger*        g_logger;


void logInfo(const QString &msg)
{
  g_logger->log(Logger::Type::INFO, msg);
  qDebug() << msg;
}


void logWarn(const QString &msg)
{
  g_logger->log(Logger::Type::WARN, msg);
  qWarning(qPrintable(msg));
}


void logError(const QString &msg)
{
  g_logger->log(Logger::Type::ERROR, msg);
  qWarning(qPrintable(msg));
}

