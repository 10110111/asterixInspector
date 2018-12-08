/*
 * This application is free software and is released under the terms of
 * the BSD license. See LICENSE file for details.
 *
 * Copyright (c) 2014 Volker Poplawski (volker@openbios.org)
 */

#include "asterix.h"

#include "global.h"

AsterixBlock::AsterixBlock(const uchar* data, const Uap& uap) :
    m_data(data)
{
  if (not uap.m_categories.contains( category() ))
  {
    logError("Cat " + QString::number(category()) + " is undefined");
    return;  // undefined cat
  }

  const UapCategory* uapCat = uap.selectedUapCategory(category());

  const uchar* p = m_data + 3;

  int recordIndex = 0;
  while (p < m_data + length())
  {
    AsterixRecord record(p, recordIndex++, uapCat);

    if (record.length() < 1)
    {
      logError("invalid record");
      return;
    }

    m_records.append(record);
    p += record.length();
  }
}


/** count number of extended octeds at p
 **/
int AsterixBlock::countExtends(const uchar *p)
{
  const uchar* ep = p;
  while (*ep & 0x1)
    ep++;

  return ep - p;
}


// extracts field-reference-numbers (FRNs) from FSPEC (multi)byte section
QList<int> AsterixBlock::decodeFspecSection(const uchar *p, int size)
{
  QList<int> ret;

  for (int fspecIdx = 0; fspecIdx < size; fspecIdx++)  // iterate over fspec bytes
  {
    for (int b = 7; b > 0; b--)  // iterate over bits from left/msb to right/lsb of fspec byte
    {
      if (BITTEST(p[fspecIdx], b))
      {
        // bit at position b is set. this yields a FRN
        int frn = (fspecIdx * 7) + (8-b);
        ret.append(frn);
      }
    }
  }

 return ret;
}


AsterixRecord::AsterixRecord(const uchar *data, int blockIndex, const UapCategory* uapCat) :
  m_data(data),
  m_index(blockIndex)
{
  int numFspec = 1 + AsterixBlock::countExtends(m_data);   // get number of fspec bytes
  QList<int> frns = AsterixBlock::decodeFspecSection(m_data, numFspec);

  const uchar* dataItemPtr = data + numFspec;

  foreach (int frn, frns)
  {
    if (not uapCat->m_dataItems.contains(frn))
    {
      logError("FRN " + QString::number(frn) +  " is undefined");
      m_length = -1;       // mark this record as invalid
      return; // undefined frn
    }

    const UapDataItem* uapItem = uapCat->m_dataItems[frn];

    AsterixDataItem dataItem(dataItemPtr, uapItem);
    m_fields.append(dataItem);

    if (frn == uapCat->timeOfDayFrn() && !uapItem->m_fields.empty())  // Time-of-Day field is comming along
    {
      const UapField& uapField = uapItem->m_fields.first();
      // some checks if it actualy contains a time entry
      if (dataItem.length() >= 2 && uapField.format() == UapField::UINTEGER)
      {
        const int value = bitfieldToUInt(dataItem.bitfield(uapField.octed(), dataItem.length(), uapField.msb()-1, uapField.lsb()-1));
        m_timeOfDay = QTime::fromMSecsSinceStartOfDay((1000.0*value)/128.0);
      }
    }

    dataItemPtr += dataItem.length();  // advance to start of next dataitem
  }
  m_length = dataItemPtr - m_data;
}


AsterixDataItem::AsterixDataItem(const uchar *data, const UapDataItem *uapDataItem) :
    m_data(data),
    m_uapDataItem(uapDataItem)
{
  if (m_uapDataItem->format() == UapDataItem::COMPOUND)
  {
    int numFspec = 1 + AsterixBlock::countExtends(m_data);   // get number of fspec bytes
    QList<int> frns = AsterixBlock::decodeFspecSection(m_data, numFspec);

    const uchar* dataItemPtr = m_data + numFspec;

    foreach (int frn, frns)
    {
      if (not uapDataItem->m_subFields.contains(frn))
      {
        logError("Subfield FRN " + QString::number(frn) + " is undefined");
        m_length = -1;       // mark this record as invalid
        return; // undefined frn
      }

      const UapDataItem* uapItem = uapDataItem->m_subFields[frn];

      AsterixDataItem dataItem(dataItemPtr, uapItem);
      m_subfields.append(dataItem);

      dataItemPtr += dataItem.length();  // advance to start of next dataitem
    }
    m_length = dataItemPtr - m_data;
  }
  else
  {
    m_length = uapDataItem->calcLength(m_data);
  }
}


/** decode a character from ICAO 6bit presentation to ascii/QChar
 **/
QChar AsterixDataItem::decodeIcaoSixBitChar(int c)
{
  c &= BITMASK(6);
  if (c >= 1 && c <= 26)       // 1..26 => 'A'..'Z'
  {
    return 'A' + c-1;
  }
  else if (c == 32)            // 32 => space
  {
    return ' ';
  }
  else if (c >= 48 && c <= 57) // 48..57 => '0'..'9'
  {
    return '0' + c-48;
  }
  else
  {
    return QChar::ReplacementCharacter;
  }
}


/** decode ICAO 6bit 8 character string located in 6 bytes at 'p'
 **/
QString AsterixDataItem::decodeIcaoStr(const uchar* p)
{
  QByteArray buffer(8, 0);  // a buffer of 8 bytes, set to 0
  buffer.replace(2, 6, (const char*)p);  // align *p to end of buffer

  quint64 s = qFromBigEndian<quint64>((const uchar*)buffer.constData()); // and convert buffer into 64bit machine word

  QString ret;
  for (int b = 42; b >= 0; b -= 6)     // first char at bit pos 42, second at 36 ... last at 0
  {
    ret += decodeIcaoSixBitChar(s >> b);
  }

  return ret;
}


/** Extract arbitrary bitfield (i.e. memory section on bit precision).
 ** Extract from multiple octeds starting at 'o' (counting from 1) and length l
 ** with bit index 0 at o+l.
 ** Resulting Bytearray is right-justified and 0-padded on left side.
 **/
QByteArray AsterixDataItem::bitfield(int o, int l, int msb, int lsb) const
{
  return ::bitfield(this->m_data + o - 1, l, msb, lsb);
}

