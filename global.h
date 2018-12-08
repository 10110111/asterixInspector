/*
 * This application is free software and is released under the terms of
 * the BSD license. See LICENSE file for details.
 *
 * Copyright (c) 2010 Volker Poplawski (volker@openbios.org)
 */
#ifndef GLOBAL_H
#define GLOBAL_H

#include <QtCore>

#include "logger.h"

class QMainWindow;
class QLabel;
class QColor;
class AsterixModel;
class Uap;

extern QMainWindow*   g_mainWindow;
extern QString        g_specsDirectory;
extern QString        g_openFileName;
extern const uchar*   g_file;
extern qint64         g_fileSize;
extern QLabel*        g_blockCountLabel;
extern AsterixModel*  g_asterixModel;
extern Uap*           g_uap;
extern QColor         g_highlightColor;
extern Logger*        g_logger;


void logInfo(const QString& msg);
void logWarn(const QString& msg);
void logError(const QString& msg);


#endif // GLOBAL_H
