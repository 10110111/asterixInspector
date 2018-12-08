/*
 * This application is free software and is released under the terms of
 * the BSD license. See LICENSE file for details.
 *
 * Copyright (c) 2010 Volker Poplawski (volker@openbios.org)
 */

#include "asterixFileMapper.h"


AsterixFileMapper::AsterixFileMapper(QIODevice* file, QObject *parent) :
    QThread(parent),
    m_file(file),
    m_offset(0)
{
  m_reachedEnd = false;
  qRegisterMetaType<QList<AsterixBlockInfo> >();
}


void AsterixFileMapper::run()
{
  qint64 blockCount = 0;
  m_stopRequested = false;
  m_file->seek(m_offset);

  while (m_reachedEnd == false && m_stopRequested == false)
  {
    QList<AsterixBlockInfo> ret;

    QTime time;
    time.start();

    do {
      if (m_file->atEnd())
      {
        m_reachedEnd = true;
        break;
      }

      m_file->seek(m_offset);
      qint64 offset = m_file->pos();

      quint8 category;
      if (not m_file->read((char*)&category, 1))
        break;

      char sbuffer[2];
      if (not m_file->read(sbuffer, 2))
        break;

      quint16 blockSize = qFromBigEndian<quint16>((uchar*)sbuffer);

      if (blockSize >= 3                         // valid blocksize
          &&                                     // and
          offset + blockSize <= m_file->size() ) // block not truncated?
      {
        // yes
        AsterixBlockInfo block = { offset, category, blockSize };
        ret << block;
      }
      else
      {
        m_reachedEnd = true;
        break;
      }

      m_offset += blockSize;
      blockCount++;

    } while (time.elapsed() < 100);

    emit asterixBlock(ret);
    emit bytesScanned(m_offset);
    emit blocksScanned(blockCount);
    msleep(50);
  }
}


