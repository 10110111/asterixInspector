/*
 * This application is free software and is released under the terms of
 * the BSD license. See LICENSE file for details.
 *
 * Copyright (c) 2010 Volker Poplawski (volker@openbios.org)
 */
#ifndef UAP_H
#define UAP_H

#include <QtCore>
#include <QtXml>

class UapEnum
{
  public:
    UapEnum(int value, const QString& desc) : m_value(value), m_desc(desc) { }

    int       m_value;
    QString   m_desc;
};


class UapField
{
  public:
    enum Format {
      INTEGER,
      UINTEGER,
      HEX,
      OCTAL,
      ICAO6STR,       // str of 8 6-bit characters
      ASCII,
    };

    enum Unit {
      UNDEFINED,
      DEGREE_CELSIUS,
      DEGREE,
      DEGREE_PER_SECOND,
      FOOT,
      FOOT_PER_MINUTE,
      FLIGHTLEVEL,
      KNOT,
      MACH,
      METER,
      METER_PER_SECOND,
      METER_PER_SQSECOND,
      MILLIBAR,
      NAUTICALMILE,
      NAUTICALMILE_PER_SECOND,
      SECOND,
      SQMETER,
      MEGAHERTZ,
      DECIBELMILLIWATT,
    };

    UapField(QDomElement fieldElm);

    QString name() const { return m_name; }
    int octed() const { return m_octed; }

    int msb() const { return m_msb; }
    int lsb() const { return m_lsb; }
    QString desc() const { return m_desc; }
    Format format() const { return m_format; }
    Unit unit() const { return m_unit; }
    double scale() const { return m_scale; }
    const QMap<int, UapEnum>& enums() const { return m_enums; }

    static const QString& formatStr(Format format);
    static Format formatFromStr(const QString& str);

    static const QString& unitStr(Unit format);
    static Unit unitFromStr(const QString& str);

  protected:
    void initStaticHashes();

    QString     m_name;
    QString     m_desc;
    int         m_octed;
    int         m_msb;
    int         m_lsb;
    Format      m_format;
    Unit        m_unit;
    double      m_scale;

    QMap<int, UapEnum>   m_enums;

    static const QStringList m_formatStrs;
    static const QStringList m_unitStrs;
};


class UapDataItem
{
  public:
    UapDataItem() { }
    UapDataItem(QDomElement dataItemElm);

    enum Format {
      FIXED,
      VARIABLE,
      EXPLICIT,
      REPETIVE,
      COMPOUND
    };

    int frn() const { return m_frn; }
    const QString id() const { return m_id; }
    const QString name() const { return m_name; }
    Format format() const { return m_format; }
    int length() const { return m_length; }
    const QString& definition() const { return m_definition; }
    const QString& description() const { return m_description; }

    int calcLength(const uchar* p) const;
    bool fieldPresent(const uchar* p, const UapField& uapField) const;

    static const QString& formatStr(Format format);
    static Format formatFromStr(const QString& str);

    static QString extractContents(QDomElement el);

    QList<UapField>               m_fields;
    QMap<int, const UapDataItem*> m_subFields;  // maps from sub-FRN to DataItem, if m_format==COMPOUND

  protected:
    int                      m_frn;
    QString                  m_id;
    QString                  m_name;
    Format                   m_format;
    int                      m_length;
    QString                  m_definition;
    QString                  m_description;

    static const QStringList m_formatStrs;
};


class UapCategory
{
  public:
    UapCategory() : m_category(-1) { }
    ~UapCategory() { qDeleteAll(m_dataItems); }
    UapCategory(QDomElement catEl);
    int category() const { return m_category; }

    const UapDataItem* dataItem(int frn) const;

    int timeOfDayFrn() const;

    int                              m_category;
    QString                          m_version;
    QString                          m_sourceFile;

    QMap<int, const UapDataItem*>    m_dataItems;   // maps from FRN to DataItem
};


class Uap
{
  public:
    ~Uap() { qDeleteAll(m_categories); }
    void readXml(QFile *input);
    void readXml(QString dirpath);

    const UapCategory* selectedUapCategory(int cat) const;
    void selectUapCategory(const UapCategory* uapCategory);

    static int parseNumber(const QString& number);
    static QString mandAtt(QDomElement elm, const QString& att);

    QMultiMap<int, const UapCategory*> m_categories;

    QMap<int, const UapCategory*> m_selectedCategory;  // selected UAP for category
};



#endif // UAP_H
