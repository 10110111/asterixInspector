/*
 * This application is free software and is released under the terms of
 * the BSD license. See LICENSE file for details.
 *
 * Copyright (c) 2013 Volker Poplawski (volker@openbios.org)
 */
#include "logger.h"

const QString typeStrs[]{"INFO", "WARN", "ERROR"};


class MyTextEdit : public QTextEdit
{
  protected:
    virtual void contextMenuEvent(QContextMenuEvent *e) override
    {
      QMenu *menu = createStandardContextMenu();
      menu->addAction("Clear", this, SLOT(clear()));

      menu->exec(e->globalPos());
      delete menu;
    }
};


Logger::Logger(QObject *parent) :
  QObject(parent)
{
  m_textEdit = new MyTextEdit;
  m_textEdit->setReadOnly(true);
  // m_textEdit->setFontFamily("monospace");
}


void Logger::log(Logger::Type type, const QString& msg)
{
  m_textEdit->append(typeStrs[(int)type] + ": " + msg);
}
