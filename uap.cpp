/*
 * This application is free software and is released under the terms of
 * the BSD license. See LICENSE file for details.
 *
 * Copyright (c) 2010 Volker Poplawski (volker@openbios.org)
 */
#include "uap.h"

#include <QtXml>

#include "asterix.h"
#include "scaleExpressionParser.h"
#include "global.h"

const QStringList UapDataItem::m_formatStrs = QStringList() << "fixed" << "variable" << "explicit" << "repetive" << "compound";    // must match enum UapDataItem::Format

const QStringList UapField::m_formatStrs = QStringList() << "int" << "uint" << "hex" << "octal" << "icao6str" << "ascii";          // must match enum UapField::Format

// must match enum UapField::Unit
const QStringList UapField::m_unitStrs =
  QStringList() << "UNDEFINED" << "deg" << "celsius" << "deg/s" << "ft" << "ft/min" << "FL" << "kn" << "ma" <<
                   "m" << "m/s" << "m/s^2" << "mb" << "nmi" << "nmi/s" << "s" << "m^2" << "MHz" << "dBm";


UapField::UapField(QDomElement fieldElm) :
  m_octed( Uap::parseNumber(Uap::mandAtt(fieldElm, "octed")) ),
  m_msb( Uap::parseNumber(fieldElm.attribute("msb")) ),
  m_lsb( Uap::parseNumber(fieldElm.attribute("lsb")) ),
  m_format(HEX),
  m_unit(UNDEFINED),
  m_scale(1.0)
{
  m_name = fieldElm.firstChildElement("name").text();
  m_desc = fieldElm.firstChildElement("desc").text();
  m_format = formatFromStr(fieldElm.firstChildElement("format").text());

  if (not fieldElm.firstChildElement("unit").text().isEmpty())
  {
    m_unit = unitFromStr(fieldElm.firstChildElement("unit").text());
  }

  if (not fieldElm.firstChildElement("scale").text().isEmpty())
  {
    bool error;
    m_scale = ScaleExpressionParser::parse(fieldElm.firstChildElement("scale").text(), error);

    if (error)
    {
      logError(ScaleExpressionParser::lastError + " in <scale> expression '" + fieldElm.firstChildElement("scale").text() + "'");
      m_scale = 1.0;
    }

  }

  if (m_msb < m_lsb)
  {
    logWarn("Invalid msb/lsb on field '" + m_name + "'");
    m_msb = m_lsb;
  }

  QDomNodeList enums = fieldElm.elementsByTagName("enum");
  for (int i = 0; i < enums.count(); i++)
  {
    QDomElement enumElm = enums.at(i).toElement();
    UapEnum uapEnum( Uap::parseNumber(enumElm.attribute("value")), enumElm.text() );
    m_enums.insert(uapEnum.m_value, uapEnum);
  }
}


const QString& UapField::formatStr(Format format)
{
  return m_formatStrs[format];
}


UapField::Format UapField::formatFromStr(const QString &str)
{
  int i = m_formatStrs.indexOf(str.toLower());

  if (i < 0)
    return HEX;   // Format description unknown or empty. Default to HEX.
  else
    return (Format)(i);
}


const QString& UapField::unitStr(Unit unit)
{
  return m_unitStrs[unit];
}


UapField::Unit UapField::unitFromStr(const QString &str)
{
  int i = (Unit)m_unitStrs.indexOf(str);

  if (i < 0)
  {
    logWarn("Unknown <unit> " + str);
    return UNDEFINED;  // Unit description unknown or empty.
  }
  else
  {
    return (Unit)(i);
  }
}


const QString& UapDataItem::formatStr(Format format)
{
  return m_formatStrs[format];
}


UapDataItem::Format UapDataItem::formatFromStr(const QString &str)
{
  return (Format)m_formatStrs.indexOf(str.toLower());
}


UapDataItem::UapDataItem(QDomElement itemElm) :
  m_frn( Uap::mandAtt(itemElm, "frn").toInt() ),
  m_id( itemElm.attribute("id") ),
  m_format( UapDataItem::formatFromStr(Uap::mandAtt(itemElm, "format")) ),
  m_length( Uap::parseNumber( itemElm.attribute("length") ) )
{
  m_name = itemElm.firstChildElement("name").text();
  m_definition = itemElm.firstChildElement("definition").text();
  m_description = extractContents(itemElm.firstChildElement("desc"));

  if (m_format != COMPOUND)
  {
    // iterate over child elements <field>
    QDomNode node = itemElm.firstChild();
    while (not node.isNull())
    {
      if (node.toElement().tagName() == "field")
      {
        UapField uapField( node.toElement() );
        m_fields.append(uapField);
      }
      node = node.nextSiblingElement();
    }
  }
  else
  {
    // iterate over child elements <subfield>
    QDomNode node = itemElm.firstChild();
    while (not node.isNull())
    {
      if (node.toElement().tagName() == "subfield")
      {
        UapDataItem* uapItem = new UapDataItem( node.toElement() );
        uapItem->m_id = m_id + "#" + QString::number(uapItem->frn());
        m_subFields.insert( uapItem->frn(), uapItem);
      }
      node = node.nextSiblingElement();
    }
  }
}


QString UapDataItem::extractContents(QDomElement el)
{
  QString ret;
  QTextStream s(&ret);
  el.save(s, 0);
  return ret;
}


int UapDataItem::calcLength(const uchar *p) const
{
  switch (m_format) {
  case FIXED:
    return m_length;
    break;

  case VARIABLE:
    return m_length + AsterixBlock::countExtends(p+m_length-1);
    break;

  case EXPLICIT:
    return *(const quint8*)p;
    break;

  case REPETIVE:
    return 1 + (*(const quint8*)p * m_length);
    break;

  case COMPOUND:
    logError("Error in specification: Compound length cannot be computed.");
    break;
  }

  return 0;
}


bool UapDataItem::fieldPresent(const uchar *p, const UapField &uapField) const
{
  switch (m_format) {
  case FIXED:
    return true;
    break;

  case VARIABLE:
    return uapField.octed() <= calcLength(p);
    break;

  case EXPLICIT:
    return true;
    break;

  case REPETIVE:
    return true;
    break;

  case COMPOUND:
    logError("Error in specification: Field defintion not allowed below compound.");
    return false;
    break;
  }

  return false;
}


UapCategory::UapCategory(QDomElement catEl)
{
  m_category = Uap::mandAtt(catEl, "cat").toInt();
  m_version = catEl.attribute("version");

  QDomNode node = catEl.firstChild();
  while (not node.isNull())
  {
    if (node.toElement().tagName() == "dataitem")
    {
      UapDataItem* uapItem = new UapDataItem( node.toElement() );
      m_dataItems.insert( uapItem->frn(), uapItem);
    }
    node = node.nextSiblingElement();
  }
}


/** Return FRN of Time-Of-Day/Time-Of-Track field.
 ** or -1 if unknown or not present.
 **/
int UapCategory::timeOfDayFrn() const
{
  switch (m_category)
  {
  case  1: return 9;
  case  2: return 4;
  case  4: return 4;
  case  7: return 4;
  case  8: return 8;
  case 10: return 4;
  case 11: return 4;
  case 19: return 3;
  case 20: return 3;
  case 21: return 5;
  case 23: return 4;
  case 25: return 6;
  case 34: return 3;
  case 48: return 2;
  case 62: return 4;
  case 63: return 3;
  case 65: return 4;
  }

  return -1;
}


void Uap::readXml(QFile* input)
{
  QDomDocument doc;
  doc.setContent(input);

  QDomNodeList catElms = doc.elementsByTagName("category");
  for (int i = 0; i < catElms.count(); i++)
  {
    UapCategory* uapCategory = new UapCategory(catElms.at(i).toElement());
    uapCategory->m_sourceFile = input->fileName();
    m_categories.insert(uapCategory->category(), uapCategory);

    logInfo("Read cat " + QString::number(uapCategory->m_category) + " version " + uapCategory->m_version + " from file '" + uapCategory->m_sourceFile + "'");
  }
}


void Uap::readXml(QString dirpath)
{
  QDir dir(dirpath);

  logInfo("Reading *.xml files from directory " + dir.absolutePath());

  QFileInfoList fileInfos = dir.entryInfoList(QStringList() << "*.xml", QDir::Files | QDir::Readable);

  if (fileInfos.isEmpty())
  {
    logWarn("No *.xml files found in " + dir.absolutePath());
  }

  foreach (QFileInfo fileInfo, fileInfos)
  {
    QFile file(fileInfo.filePath());

    if (not file.open(QIODevice::ReadOnly))
    {
      logWarn("Unable to open file " + fileInfo.filePath());
      continue;
    }

    readXml(&file);
  }
}


const UapCategory *Uap::selectedUapCategory(int cat) const
{
  if (m_selectedCategory.contains(cat))
  {
    return m_selectedCategory.value(cat);
  }
  else if (m_categories.contains(cat))
  {
    return m_categories.value(cat);   // return first found
  }
  else
  {
    return nullptr;
  }
}


/** Select UapCategory to be used for certain Asterix category.
 ** Asterix category number is extracted from uapCategory.
 **/
void Uap::selectUapCategory(const UapCategory *uapCategory)
{
  Q_ASSERT(uapCategory);
  m_selectedCategory.insert(uapCategory->category(), uapCategory);
}


int Uap::parseNumber(const QString &number)
{
  if (number.left(2) == "0x")
    return number.mid( 1 ).toInt(0, 16);
  else if (number.right(1) == "b")
    return number.left( number.size()-1 ).toInt( 0, 2 );
  else
    return number.toInt();
}


QString Uap::mandAtt(QDomElement elm, const QString &att)
{
  if (elm.attribute(att).isEmpty())
    qFatal(qPrintable("Mandadory attribute '" + att + "'' is missing or empty on '" + elm.tagName() + "'."));

  return elm.attribute(att);
}
