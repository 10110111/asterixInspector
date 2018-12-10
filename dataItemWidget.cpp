/*
 * This application is free software and is released under the terms of
 * the BSD license. See LICENSE file for details.
 *
 * Copyright (c) 2010 Volker Poplawski (volker@openbios.org)
 */
#include "dataItemWidget.h"

#include "uap.h"
#include "global.h"


extern Uap* g_uap;

DataItemWidget::DataItemWidget(QWidget *parent) :
    QWidget(parent)
{
  QVBoxLayout* vbox = new QVBoxLayout;
  setLayout(vbox);

  QFile cssFile(":/dataItem.css");
  if (not cssFile.open(QIODevice::ReadOnly))
    qFatal("CSS not found.");

  m_css = cssFile.readAll();

  m_webView = new QWebEngineView(0);
  m_webView->settings()->setFontFamily(QWebEngineSettings::StandardFont, QFont().family());
  m_webView->settings()->setFontSize(QWebEngineSettings::DefaultFontSize, QFont().pointSize() * 1.4);
  m_webView->setMinimumSize(100, 88);
  m_webView->resize(100, 88);
  layout()->addWidget(m_webView);
}


void DataItemWidget::setDataItem(const AsterixBlock* block, const AsterixRecord* record, const AsterixDataItem *dataItem)
{
  if (block == 0)
    return;

  if (g_uap->selectedUapCategory(block->category()))
  {
    const UapDataItem& uapDataItem = *dataItem->uapDataItem();
    const int ofs = uapDataItem.format() == UapDataItem::REPETIVE ? 1 : 0;

    QString html;

    html += "<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en_US\" lang=\"en_US\">";

    html += "<head>";
    html += "<style type=\"text/css\"><!--";
    html += m_css;
    html += "--></style></head>";


    html += "<body>";
    html += "<h3>" + uapDataItem.id() + "&nbsp;&nbsp;" + uapDataItem.name() + "</h3>";
    if (not uapDataItem.definition().isEmpty())
      html += "<p>Definition: " + uapDataItem.definition() + "</p>";
    html += "<p><table border=\"0\" style=\"margin-left:26px\">";

    foreach (const UapField& uapField, uapDataItem.m_fields)
    {
      if (not uapDataItem.fieldPresent(dataItem->data(), uapField))
        continue;

      html += "<tr><td style=\"border-top:10px solid transparent;\" colspan=\"3\">" + renderBitTable(*dataItem, uapField) + "</td></tr>";

      html += "<tr>";

      html += "<td style=\"vertical-align:top;\">" + (uapField.name().isEmpty() ? "<i>nn</i>" : uapField.name()) + "</td>";
      html += "<td style=\"vertical-align:top; padding-left:6px; padding-right:6px\"><u>";

      const int correctedLength = uapDataItem.format() == UapDataItem::VARIABLE ? 1 : uapDataItem.length();
      QByteArray field = dataItem->bitfield(uapField.octed() + ofs, correctedLength, uapField.msb()-1, uapField.lsb()-1);
      int value = 0;

      switch (uapField.format())
      {
        case UapField::ICAO6STR:
        {
          QString s = AsterixDataItem::decodeIcaoStr((const uchar*)field.constData());
          html += "\"" + s.replace(' ', "&nbsp;") + "\"";
        }
        break;

        case UapField::UINTEGER:
        {
          value = bitfieldToUInt(field);
          html += QString("%1").arg(value) + "</u>";
          html += "&nbsp;&nbsp;(<u>" + applyUnitAndScale(uapField, value) + "</u>)<u>";
        }
        break;

        case UapField::INTEGER:
        {
          value = bitfieldToInt(field, uapField.msb()-1 - (((uapField.lsb()-1)/8)*8) );
          html += QString("%1").arg(value) + "</u>";
          html += "&nbsp;&nbsp;(<u>" + applyUnitAndScale(uapField, value) + "</u>)<u>";
        }
        break;

        case UapField::OCTAL:
        {
          value = bitfieldToUInt(field);
          html += QString("0%1").arg(value, 0, 8);
        }
        break;

        case UapField::ASCII:
        {
          QString s = QString::fromLatin1(field);
          s.replace(" ", "&nbsp;");
          html += "\"" + s + "\"";
        }
        break;

        case UapField::HEX:
        default:
        {
          value = bitfieldToUInt(field);
          html += "0x";
          for (int i=0; i < field.size(); i++)
            html += QString("%1").arg((uint)((uchar)field[i]), 2, 16, QChar('0'));
        }
        break;
      }

      // line += dataItem->decode(uapField.m_octed, uapField.msb(), uapField.lsb());
      html += "</u></td>";

      html += "<td style=\"vertical-align:top; padding:0;\">";

      if (not uapField.desc().isEmpty())
        html += uapField.desc();

      html += printEnumeration(uapField, value);

      html += "</td></tr>";


    }
    html += "</table></p>";

    if (not uapDataItem.description().isEmpty())
      html += "<p>" + uapDataItem.description() + "</p>";

    html += "</body></html>";

    m_webView->setHtml(html);
  }
}


QString DataItemWidget::printEnumeration(const UapField& uapField, int value)
{
  QString html = "<table border=\"0\">";
  for (QMap<int, UapEnum>::const_iterator it = uapField.enums().begin(); it != uapField.enums().end(); ++it)
  {
    html += "<tr><td>=&nbsp;" + QString::number(it.key()) + "</td><td>";

    if (it.key() == value)
      html+= "<u>";

    html += it.value().m_desc;

    if (it.key() == value)
      html+= "</u>";

    html += "</td></tr>";
  }
  html += "</table>";
  return html;
}


QString DataItemWidget::applyUnitAndScale(const UapField& uapField, int value)
{
  QString html;

  if (uapField.unit() != UapField::UNDEFINED)
  {
    html += QString::number((double)value * uapField.scale()) + "&nbsp;" + uapField.unitStr(uapField.unit());
  }

  return html;
}


QString DataItemWidget::renderBitTable(const AsterixDataItem &dataItem, const UapField& uapField) const
{
  const UapDataItem uapDataItem = *dataItem.uapDataItem();
  const int correctedLength = uapDataItem.format() == UapDataItem::VARIABLE ? 1 : uapDataItem.length();
  const int ofs = uapDataItem.format() == UapDataItem::REPETIVE ? 1 : 0;

  const int lsb = uapField.lsb()-1;
  const int msb = uapField.msb()-1;
  const int lByte = uapField.octed()-1 + ofs + correctedLength-1 - lsb/8;   // offset of least sig byte
  const int mByte = uapField.octed()-1 + ofs + correctedLength-1 - msb/8;   // offset of most sig byte

  const QString title = QString("Bits-%1/%2").arg(msb+1).arg(lsb+1);
  QString s = "<table border=\"0\" cellspacing=\"0\" class=\"bittable\" title=\"" + title + "\">";

  s += "<tr>";
  for (int byte = mByte; byte <= lByte; byte++)
  {
    QString byteHex = QString("%1").arg((uint)dataItem.data()[byte], 2, 16, QChar('0')).toUpper();
    s += "<td colspan=\"4\" align=\"center\" bgcolor=\"" + g_highlightColor.name() + "\">" + byteHex[0] + "</td>";
    s += "<td colspan=\"4\" align=\"center\" bgcolor=\"" + g_highlightColor.name() + "\">" + byteHex[1] + "</td>";
    if (byte != lByte)
    {
      s += "<td style=\"padding-left:4px;\"></td>";
    }
  }
  s += "</tr>";

  s += "<tr>";
  for (int byte = mByte; byte <= lByte; byte++)
  {
    QString byteBin = QString("%1").arg((uint)dataItem.data()[byte], 8, 2, QChar('0'));
    bool inside = true;
    for (int b = 0; b < 8; b++)
    {
      const int bitpos = (correctedLength-byte+ofs+(uapField.octed()-1))*8-b-1;
      if (bitpos == msb && bitpos == lsb)
        s += "<td class=\"both\">" + byteBin[b] + "</td>";
      else if (bitpos == msb && bitpos > lsb)
        s += "<td class=\"left\">" + byteBin[b] + "</td>";
      else if (bitpos < msb && bitpos > lsb)
        s += "<td class=\"mid\">" + byteBin[b] + "</td>";
      else if (bitpos < msb && bitpos == lsb)
      {
        s += "<td class=\"right\">" + byteBin[b] + "</td>";
        inside = false;
      }
      else
        s += "<td class=\"node\">" + byteBin[b] + "</td>";
    }
    if (byte != lByte)
    {
      if (inside)
        s += "<td class=\"mid\"></td>";
      else
        s += "<td style=\"padding-bottom:0;\" ></td>";
    }
  }
  s += "</tr>";
  s += "</table>";

  return s;
}

