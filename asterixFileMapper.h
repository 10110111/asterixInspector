/*
 * This application is free software and is released under the terms of
 * the BSD license. See LICENSE file for details.
 *
 * Copyright (c) 2010 Volker Poplawski (volker@openbios.org)
 */

#ifndef ASTERIXFILEMAPPER_H
#define ASTERIXFILEMAPPER_H

#include <QtCore>


class AsterixBlockInfo
{
  public:
    qint64    offset;
    qint16    category;
    quint16   size;
};

Q_DECLARE_METATYPE(AsterixBlockInfo);
Q_DECLARE_METATYPE(QList<AsterixBlockInfo>);

class AsterixFileMapper : public QThread
{
    Q_OBJECT
public:
    explicit AsterixFileMapper(QIODevice* file, QObject *parent = 0);

    bool reachedEnd() const { return m_reachedEnd; }

  signals:
    void asterixBlock(const QList<AsterixBlockInfo> block);
    void bytesScanned(qint64 pos);
    void blocksScanned(qint64 pos);

  public slots:
    void stop() { m_stopRequested = true; }
    void setOffset(qint64 ofs) { m_offset = ofs; }


  protected:
    virtual void run();

    QIODevice* m_file;
    quint64    m_offset;
    bool       m_reachedEnd;
    bool       m_stopRequested;
};

#endif // ASTERIXFILEMAPPER_H
