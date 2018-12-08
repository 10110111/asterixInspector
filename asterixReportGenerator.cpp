/*
 * This application is free software and is released under the terms of
 * the BSD license. See LICENSE file for details.
 *
 * Copyright (c) 2016 Volker Poplawski (volker@openbios.org)
 */

#include "asterixReportGenerator.h"

#include <QProgressDialog>

#include "global.h"
#include "asterixModel.h"
#include "uap.h"

AsterixReportGenerator::AsterixReportGenerator( QObject *parent) :
  QObject(parent)
{
}


void AsterixReportGenerator::createReport(QTextStream& out, Type type)
{
  if (g_asterixModel == 0)
    return;

  QScopedPointer<AbstractGenerator> generator;
  switch (type)
  {
    case XML:
      generator.reset(new XmlGenerator(out));
      break;
  }

  QProgressDialog progressDialog("Generating Report...", "Abort", 0, g_asterixModel->numBlocks(), g_mainWindow);
  progressDialog.setWindowModality(Qt::WindowModal);

  for (int i = 0; i < g_asterixModel->numBlocks(); ++i)
  {
    progressDialog.setValue(i);
    if (progressDialog.wasCanceled())
    {
      break;
    }

    const AsterixBlockInfo& blockInfo = g_asterixModel->getBlockInfos()[i];
    const AsterixBlock block(g_file + blockInfo.offset, *g_uap);

    handleBlock(generator.data(), block);
  }

  progressDialog.setValue(g_asterixModel->numBlocks());  // this will close the dialog
}


void AsterixReportGenerator::handleBlock(AbstractGenerator* gen, const AsterixBlock& block)
{
  gen->startBlock(block);
  for (int j = 0; j < block.numRecords(); ++j)
  {
    const AsterixRecord& record = block.record(j);
    gen->startRecord(record);

    gen->startDataItemsSection(record);
    for (int k = 0; k < record.numFields(); ++k)
    {
      const AsterixDataItem &dataItem = record.field(k);
      const UapDataItem &uap = *dataItem.uapDataItem();

      gen->startDataItem(dataItem, uap);

      if (dataItem.numSubfields() == 0)
      {
        foreach (const UapField& uapField, uap.m_fields)
        {
          if (not uap.fieldPresent(dataItem.data(), uapField))
            continue;

          // suppress output of spare fields
          if (uapField.name() == "Spare")
            continue;

          gen->startFinishField(dataItem, uapField);

        }
      }
      else
      {
        for (int si = 0; si < dataItem.numSubfields(); si++)
        {
          const AsterixDataItem& subField = dataItem.subField(si);
          const UapDataItem& subUap = *subField.uapDataItem();
          gen->startDataItem(subField, subUap, 1);

          foreach (const UapField& uapField, subUap.m_fields)
          {
            if (not subUap.fieldPresent(subField.data(), uapField))
              continue;

            // suppress output of spare fields
            if (uapField.name() == "Spare")
              continue;

            gen->startFinishField(subField, uapField, 1);
          }

          gen->finishDataItem(subField, *subField.uapDataItem(), 1);
        }
      }
      gen->finishDataItem(dataItem, uap);
    }
    gen->finishDataItemsSection(record);
    gen->finishRecord(record);
  }
  gen->finishBlock(block);
}


AbstractGenerator::Field AbstractGenerator::decodeField(const AsterixDataItem& dataItem, const UapField& uapField)
{
  const int itemLength = (dataItem.uapDataItem()->format() == UapDataItem::VARIABLE) ? 1 : dataItem.length();
  const QByteArray field = dataItem.bitfield(uapField.octed(), itemLength, uapField.msb()-1, uapField.lsb()-1);
  int value = 0;

  Field ret;
  ret.hasraw = false;

  switch (uapField.format())
  {
    case UapField::ICAO6STR:
      ret.value = AsterixDataItem::decodeIcaoStr((const uchar*)field.constData());
      break;

    case UapField::UINTEGER:
      ret = unitAndScale(uapField, bitfieldToUInt(field));
      break;

    case UapField::INTEGER:
      ret = unitAndScale(uapField, bitfieldToInt(field, uapField.msb()-1 - (((uapField.lsb()-1)/8)*8)));
      break;

    case UapField::OCTAL:
      ret.rawval = bitfieldToUInt(field);
      ret.hasraw = true;
      ret.value = QString("0%1").arg(ret.rawval, 0, 8);
      ret.unit = uapField.unitStr(uapField.unit());
      break;

    case UapField::ASCII:
      ret.value = QString::fromLatin1(field);
      break;

    case UapField::HEX:
    default:
    ret.value = "0x";
    for (int i=0; i < field.size(); i++)
    {
      ret.value += QString("%1").arg((uint)((uchar)field[i]), 2, 16, QChar('0')).toUpper();
    }
    break;
  }

  // place enum description (if ex.) in 'unit' field
  QMap<int, UapEnum>::const_iterator iterator = uapField.enums().constFind(value);
  if (iterator != uapField.enums().end())
  {
    ret.unit = iterator.value().m_desc;
  }

  return ret;
}


AbstractGenerator::Field AbstractGenerator::unitAndScale(const UapField& uapField, qint64 rawvalue)
{
  Field f;
  f.rawval = rawvalue;
  f.hasraw = true;

  if (uapField.unit() != UapField::UNDEFINED)
  {
    f.value = QString::number((double)rawvalue * uapField.scale());
    f.unit = uapField.unitStr(uapField.unit());
  }
  else
  {
    f.value = QString::number(rawvalue);
  }

  return f;
}


void XmlGenerator::startBlock(const AsterixBlock& block)
{
  ind(0) << R"(<block cat=")" << block.category() << R"(" len=")" << block.length() << R"(">)" << endl;
}


void XmlGenerator::finishBlock(const AsterixBlock&)
{
  ind(0) << R"(</block>)"   << endl;
}


void XmlGenerator::startRecord(const AsterixRecord& record)
{
  ind(1) << R"(<record len=")" << record.length() << R"(">)" << endl;
}


void XmlGenerator::finishRecord(const AsterixRecord&)
{
  ind(1) << R"(</record>)"  << endl;
}


void XmlGenerator::startDataItem(const AsterixDataItem& dataItem, const UapDataItem& uap, int i)
{
  ind(2+i) << R"(<dataitem type=")" << uap.id() << R"(" frn=")" << uap.frn() << R"(" len=")" << dataItem.length() << R"(">)" << endl;
}


void XmlGenerator::finishDataItem(const AsterixDataItem&, const UapDataItem&, int i)
{
  ind(2+i) << R"(</dataitem>)" << endl;
}


void XmlGenerator::startFinishField(const AsterixDataItem& dataItem, const UapField& uap, int i)
{
  ind(3+i) << R"(<field name=")" << uap.name() << R"(">)" << endl;
  ind(4+i) << R"(<desc>)" << wrapInCData(uap.desc()) << R"(</desc>)" << endl;   // desc might contain non xml characters
  Field f = decodeField(dataItem, uap);
  ind(4+i) << R"(<value)";
  if (f.hasraw)
  {
   ind(0) << R"( raw=")" << f.rawval << R"(")";
  }
  if (not f.unit.isEmpty())
  {
    ind(0) << R"( unit=")" << f.unit << R"(")";
  }
  ind(0) << R"(>)" << f.value << R"(</value>)" << endl;
  ind(3+i) << R"(</field>)" << endl;
}


QString XmlGenerator::wrapInCData(const QString& s)
{
  if (s.isEmpty())
  {
    return QString();
  }
  else
  {
    return "<![CDATA[" + s + "]]>";
  }
}

