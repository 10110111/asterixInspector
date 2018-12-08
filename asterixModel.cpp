/*
 * This application is free software and is released under the terms of
 * the BSD license. See LICENSE file for details.
 *
 * Copyright (c) 2010 Volker Poplawski (volker@openbios.org)
 */

#include "asterixModel.h"

#include "global.h"
#include "asterixFileMapper.h"


AsterixModel::AsterixModel(QObject *parent) :
    QAbstractItemModel(parent)
{
  m_blocksExposed = 0;
  m_blockInsertPos = 0;
}


void AsterixModel::asterixBlock(const QList<AsterixBlockInfo> blocks)
{
  if (blocks.isEmpty())
    return;

  beginInsertRows( QModelIndex(), m_blockInsertPos, m_blockInsertPos + blocks.count() - 1);
  foreach (const AsterixBlockInfo& block, blocks)
  {
    if (m_blockInsertPos >= m_blocks.size())
    {
      m_blocks.resize(m_blocks.size() + m_blocks.size()/2);
    }
    m_blocks[m_blockInsertPos++] = block;
  }
  endInsertRows();
}


//bool AsterixModel::canFetchMore(const QModelIndex &parent) const
//{
//  if (parent.isValid())
//  {
//    return false;
//  }
//  else
//  {
//    return m_blocksExposed < m_blocks.size();
//  }
//}


//void AsterixModel::fetchMore(const QModelIndex &parent)
//{
//  if (not parent.isValid())
//  {
//    int numFetch = qMin(m_blocks.count() - m_blocksExposed, fetchIncrement);
//    if (numFetch > 0)
//    {
//      beginInsertRows( QModelIndex(), m_blocksExposed, m_blocksExposed + numFetch - 1);
//      m_blocksExposed += numFetch;
//      endInsertRows();
//    }
//  }
//}


void AsterixModel::blocksScanned(qint64 blockNum)
{
  m_numBlocks = blockNum;
  g_blockCountLabel->setText( QString("Blocks: %1").arg(blockNum) );
}


QVariant AsterixModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  static QString headers[] = { "Block Offset", "Length", "Category" };

  if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    return headers[section];
  else if (orientation == Qt::Horizontal && role == Qt::ToolTipRole)
  {
    switch (section)
    {
      case 0: return "Position in file";
      case 1: return "Stated length";
      case 2: return "Asterix Category";
    }

    return QVariant();
  }
  else
    return QAbstractItemModel::headerData(section, orientation, role);
}


QModelIndex AsterixModel::index(int row, int column, const QModelIndex &parent) const
{
  return createIndex(row, column, row);

}


QVariant AsterixModel::data(const QModelIndex &index, int role) const
{
  switch (role)
  {
  case Qt::DisplayRole:
    {
      switch (index.column())
      {
      case 0:
        return QString("0x%1").arg(m_blocks[index.row()].offset, 0, 16);
        break;

      case 1:
        return m_blocks[index.row()].size;
        break;

      case 2:
        return m_blocks[index.row()].category;
        break;
      }
    }

  default:
    return QVariant();
  }

}


int AsterixModel::rowCount(const QModelIndex &parent) const
{
  if (parent.isValid())
  {
    return 0;
  }
  else
  {
    return m_blockInsertPos;
  }
}


const AsterixBlockInfo& AsterixModel::blockInfo(const QModelIndex &index) const
{
  Q_ASSERT(index.isValid());
  Q_ASSERT(index.row() < m_blocks.count());

  return m_blocks[index.row()];
}


AsterixBlockModel::AsterixBlockModel(const uchar* base, const AsterixBlock& block, QObject *parent) :
    QAbstractItemModel(parent),
    m_base(base),
    m_asterixBlock(block)
{
}


QVariant AsterixBlockModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  static QString headers[] = { "Record Offset", "Time", "Length", "#Items" };

  if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    return headers[section];
  else if (orientation == Qt::Horizontal && role == Qt::ToolTipRole)
  {
    switch (section)
    {
      case 0: return "Position in file";
      case 1: return "Value of Time-Of-Day item (if applicable)";
      case 2: return "Calculated length";
      case 3: return "Number of data items";
    }

    return QVariant();
  }
  else
    return QAbstractItemModel::headerData(section, orientation, role);
}


QModelIndex AsterixBlockModel::index(int row, int column, const QModelIndex &parent) const
{
  return createIndex(row, column, row);

}


int AsterixBlockModel::rowCount(const QModelIndex &parent) const
{
  if (parent.isValid())
  {
    return 0;
  }
  else
  {
    return m_asterixBlock.numRecords();
  }
}


QVariant AsterixBlockModel::data(const QModelIndex &index, int role) const
{
  if (not index.isValid() || index.row() >= m_asterixBlock.numRecords())
    return QVariant();

  switch (role)
  {
    case Qt::DisplayRole:
    {
      switch (index.column())
      {
      case 0:
        return QString("0x%1").arg(m_asterixBlock.record(index.row()).data() - m_base, 0, 16);
        break;

      case 1:
        return m_asterixBlock.record(index.row()).timeOfDay().toString("HH:mm:ss.zzz");
        break;

      case 2:
        return m_asterixBlock.record(index.row()).length();
        break;

      case 3:
        return m_asterixBlock.record(index.row()).numFields();
        break;
      }
    }

    default:
      return QVariant();
  }
}


AsterixRecordModel::AsterixRecordModel(const uchar* base, const AsterixRecord& record, QObject *parent) :
    QAbstractItemModel(parent),
    m_base(base),
    m_asterixRecord(record)
{
  buildRowHeaders();
}


QVariant AsterixRecordModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  static const QString headers[] = { "Item Offset", "Length", "FRN", "Field Type", "Description" };

  if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    return headers[section];
  else if (orientation == Qt::Vertical && role == Qt::DisplayRole)
    return m_rowHeaders[section];
  else if (orientation == Qt::Horizontal && role == Qt::ToolTipRole)
  {
    switch (section)
    {
      case 0: return "Position in file";
      case 1: return "Specified length";
      case 2: return "Specified Field-Reference-Number";
      case 3: return "Specified field type";
    }

    return QVariant();
  }
  else
    return QAbstractItemModel::headerData(section, orientation, role);
}


QModelIndex AsterixRecordModel::index(int row, int column, const QModelIndex &parent) const
{
  return createIndex(row, column, row);
}


int AsterixRecordModel::rowCount(const QModelIndex &parent) const
{
  if (parent.isValid())
  {
    return 0;
  }
  else
  {
    return m_rowHeaders.count();
  }
}


const AsterixDataItem& AsterixRecordModel::dataItemAtRow( int r ) const
{
  const int dataItemI = m_rowHeaders[r].left( m_rowHeaders[r].indexOf('#') ).toInt() - 1;

  if (m_rowHeaders[r].contains('#'))
  {
    int subfieldIndex = m_rowHeaders[r].mid( m_rowHeaders[r].indexOf('#') + 1 ).toInt() - 1;
    return m_asterixRecord.field(dataItemI).subField(subfieldIndex);
  }
  else
  {
    return m_asterixRecord.field(dataItemI);
  }
}


QVariant AsterixRecordModel::data(const QModelIndex &index, int role) const
{
  if (not index.isValid() || index.row() >= m_rowHeaders.count())
    return QVariant();

  const int r = m_rowHeaders[index.row()].left( m_rowHeaders[index.row()].indexOf('#') ).toInt() - 1;

  switch (role)
  {
  case Qt::DisplayRole:
    {
      switch (index.column())
      {
      case 0:
      {
        const AsterixDataItem& dataItem = dataItemAtRow( index.row() );
        return QString("0x%1").arg(dataItem.data() - m_base, 0, 16);
        break;
      }

      case 1:
      {
        const AsterixDataItem& dataItem = dataItemAtRow( index.row() );
        return dataItem.length();
        break;
      }

      case 2:
      {
        const AsterixDataItem& dataItem = dataItemAtRow( index.row() );
        const UapDataItem& uapItem = *dataItem.uapDataItem();
        return uapItem.frn();
      }
        break;

      case 3:
      {
        const AsterixDataItem& dataItem = dataItemAtRow( index.row() );
        const UapDataItem& uapItem = *dataItem.uapDataItem();
        return uapItem.id();
      }
        break;

      case 4:
      {
        const AsterixDataItem& dataItem = dataItemAtRow( index.row() );
        const UapDataItem& uapItem = *dataItem.uapDataItem();
        return uapItem.name();
        break;
      }
    }
    }

  default:
    return QVariant();
  }
}


void AsterixRecordModel::buildRowHeaders()
{
  for (int i = 0; i < m_asterixRecord.numFields(); i++)
  {
    m_rowHeaders << QString::number(i+1);
    for (int si = 0; si < m_asterixRecord.field(i).numSubfields(); si++)
    {
      m_rowHeaders << QString::number(i+1) + "#" + QString::number(si+1);
    }
  }
}

