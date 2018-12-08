/*
 * This application is free software and is released under the terms of
 * the BSD license. See LICENSE file for details.
 *
 * Copyright (c) 2010 Volker Poplawski (volker@openbios.org)
 */

#ifndef ASTERIXMODEL_H
#define ASTERIXMODEL_H

#include <QtGui>

#include "asterixFileMapper.h"
#include "asterix.h"

class AsterixModel : public QAbstractItemModel
{
    Q_OBJECT
  public:
    explicit AsterixModel(QObject *parent = 0);

    int columnCount(const QModelIndex&) const { return 3; }
    QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    QModelIndex index( int row, int column = 0, const QModelIndex& parent = QModelIndex() ) const;
    int rowCount(const QModelIndex& parent) const;
    QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;
    QModelIndex parent( const QModelIndex&) const { return QModelIndex(); }

    // bool canFetchMore(const QModelIndex &parent) const;
    // void fetchMore(const QModelIndex &parent);

    const AsterixBlockInfo& blockInfo(const QModelIndex& index) const;
    int numBlocks() const { return m_numBlocks; }
    const QVector<AsterixBlockInfo> getBlockInfos() const { return m_blocks; }

  signals:

  public slots:
    void reserve(qint64 blocks) { m_blocks.resize(blocks); }
    void squeeze() { m_blocks.resize(m_numBlocks); m_blocks.squeeze(); }
    void asterixBlock(const QList<AsterixBlockInfo> block);
    void blocksScanned(qint64 blockNum);

  protected:
    QVector<AsterixBlockInfo>  m_blocks;
    int                        m_numBlocks;
    int                        m_blockInsertPos;
    int                        m_blocksExposed;

    QFile                    m_file;
};


class AsterixBlockModel : public QAbstractItemModel
{
  Q_OBJECT
  public:
    explicit AsterixBlockModel(const uchar* base, const AsterixBlock& block, QObject *parent = 0);

    int columnCount(const QModelIndex&) const { return 4; }
    QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    QModelIndex index( int row, int column = 0, const QModelIndex& parent = QModelIndex() ) const;
    int rowCount(const QModelIndex& parent) const;
    QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;
    QModelIndex parent( const QModelIndex&) const { return QModelIndex(); }

  protected:
    const uchar* m_base;
    const AsterixBlock& m_asterixBlock;
};


class AsterixRecordModel : public QAbstractItemModel
{
  Q_OBJECT
  public:
    explicit AsterixRecordModel(const uchar* base, const AsterixRecord& record, QObject *parent = 0);

    int columnCount(const QModelIndex&) const { return 5; }
    QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    QModelIndex index( int row, int column = 0, const QModelIndex& parent = QModelIndex() ) const;
    int rowCount(const QModelIndex& parent) const;
    QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;
    QModelIndex parent( const QModelIndex&) const { return QModelIndex(); }

    const AsterixDataItem& dataItemAtRow( int r ) const;

  protected:
    void buildRowHeaders();

    const uchar* m_base;
    const AsterixRecord& m_asterixRecord;

    QStringList m_rowHeaders;
};


#endif // ASTERIXMODEL_H
