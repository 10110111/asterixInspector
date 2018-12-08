/*
 * This application is free software and is released under the terms of
 * the BSD license. See LICENSE file for details.
 *
 * Copyright (c) 2013 Volker Poplawski (volker@openbios.org)
 */
#ifndef LOGGER_H
#define LOGGER_H

#include <QtWidgets>

#ifdef __MINGW32__
#ifdef ERROR
#undef ERROR
#endif
#endif

class Logger : public QObject
{
    Q_OBJECT
  public:
    explicit Logger(QObject *parent = 0);

    QTextEdit* textWidget() const { return m_textEdit; }

    enum class Type {
      INFO,
      WARN,
      ERROR
    };

  signals:

  public slots:
    void log(Type type, const QString& msg);

  private:
    QTextEdit*      m_textEdit;
};



#endif // LOGGER_H
