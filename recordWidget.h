/*
 * This application is free software and is released under the terms of
 * the BSD license. See LICENSE file for details.
 *
 * Copyright (c) 2010 Volker Poplawski (volker@openbios.org)
 */
#ifndef RECORDWIDGET_H
#define RECORDWIDGET_H

#include <QtWidgets>

#include "asterix.h"

class RecordWidget : public QWidget
{
  Q_OBJECT
  public:
    explicit RecordWidget(QWidget *parent = 0);

  void setRecord(const AsterixRecord* rec);

  signals:

  public slots:

  protected:
    QTextEdit*   m_textEdit;

};

#endif // RECORDWIDGET_H
