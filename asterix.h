/*
 * This application is free software and is released under the terms of
 * the BSD license. See LICENSE file for details.
 *
 * Copyright (c) 2014 Volker Poplawski (volker@openbios.org)
 */
#ifndef ASTERIX_H
#define ASTERIX_H

#include <QtCore>


class AsterixRecord;
class AsterixDataItem;

#include "uap.h"
#include "bit.h"


class AsterixBlock
{
  public:
    AsterixBlock(const uchar* data, const Uap& uap);

    const uchar* data() const { return m_data; }
    quint8 category() const { return *(quint8*)(m_data + 0); }
    quint16 length() const { return qFromBigEndian<quint16>(m_data + 1); }

    int numRecords() const { return m_records.count(); }
    const AsterixRecord& record(int index) const { return m_records[index]; }

    static int countExtends(const uchar* p);
    static QList<int> decodeFspecSection(const uchar* p, int size);

  protected:
    const uchar*         m_data;
    QList<AsterixRecord> m_records;
};


class AsterixRecord
{
  public:
    AsterixRecord(const uchar* data, int blockIndex, const UapCategory* uapCat);

    const uchar* data() const { return m_data; }
    int length() const { return m_length; }
    int numFields() const { return m_fields.count(); }
    const AsterixDataItem& field(int index) const { return m_fields[index]; }
    QTime timeOfDay() const { return m_timeOfDay; }

    QList<int> decodeFspecSection() const;

  protected:

    const uchar*            m_data;
    int                     m_index;   // record-index in block
    int                     m_length;
    QTime                   m_timeOfDay;
    QList<AsterixDataItem>  m_fields;
};


class AsterixDataItem
{
  public:
    AsterixDataItem(const uchar* data, const UapDataItem* uapDataItem);
    const UapDataItem* uapDataItem() const { return m_uapDataItem; }

    const uchar* data() const { return m_data; }
    int length() const { return m_length; }

    int numSubfields() const { return m_subfields.count(); }
    const AsterixDataItem& subField(int index) const { return m_subfields[index]; }

    QByteArray bitfield(int o, int l, int msb, int lsb) const;

    static QChar decodeIcaoSixBitChar(int c);
    static QString decodeIcaoStr(const uchar* p);

  protected:
    int numExtends() const { return AsterixBlock::countExtends(m_data); }

    const uchar*             m_data;
    int                      m_length;
    const UapDataItem*       m_uapDataItem;
    QList<AsterixDataItem>   m_subfields;    // if this is a compound item, otherwise empty
};



#endif // ASTERIX_H

