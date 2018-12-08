/*
 * This application is free software and is released under the terms of
 * the BSD license. See LICENSE file for details.
 *
 * Copyright (c) 2016 Volker Poplawski (volker@openbios.org)
 */

#ifndef ASTERIXREPORTGENERATOR_H
#define ASTERIXREPORTGENERATOR_H

#include <QtCore>
#include <QTextStream>

#include "asterix.h"

class AbstractGenerator;

class AsterixReportGenerator : public QObject
{
    Q_OBJECT
  public:
    explicit AsterixReportGenerator( QObject *parent = 0 );
    enum Type {
      XML,
    };

    void createReport(QTextStream& output, Type type);

    void handleBlock(AbstractGenerator* gen, const AsterixBlock& block);

  protected:
    void createBlockReport( const AsterixBlock& block, QTextStream& out );
    void createRecordReport( const AsterixRecord& record, QTextStream& out );
    void createDataItemReport(const AsterixDataItem &dataItem, QTextStream &out);
    void applyUnitAndScale(const UapField &uapField, int value, QTextStream &out);
};


class AbstractGenerator
{
  public:
    struct Field {
      QString  value;
      QString  unit;
      qint64   rawval;
      bool     hasraw;
    };

    AbstractGenerator(QTextStream& out) : m_stream(out), m_indention(4) {}
    virtual ~AbstractGenerator() {}
    virtual void startBlock(const AsterixBlock& block) = 0;
    virtual void finishBlock(const AsterixBlock& block) = 0;
    virtual void startRecordsSection(const AsterixBlock& /*block*/) {}
    virtual void finishRecordsSection(const AsterixBlock& /*block*/) {}
    virtual void startRecord(const AsterixRecord& record) = 0;
    virtual void finishRecord(const AsterixRecord& record) = 0;
    virtual void startDataItemsSection(const AsterixRecord& /*record*/, int /*indent*/ = 0) {}
    virtual void startDataItem(const AsterixDataItem& dataItem, const UapDataItem& uap, int indent = 0) = 0;
    virtual void finishDataItem(const AsterixDataItem& dataItem, const UapDataItem& uap, int indent = 0) = 0;
    virtual void finishDataItemsSection(const AsterixRecord& /*record*/, int /*indent*/ = 0) {}
    virtual void startFinishField(const AsterixDataItem& dataItem, const UapField& uap, int indent = 0) = 0;

    static Field decodeField(const AsterixDataItem& dataItem, const UapField& uapField);
    static Field unitAndScale(const UapField &uapField, qint64 rawvalue);

  protected:
    QTextStream& ind(int level) const { m_stream << QString(level * m_indention, ' '); return m_stream; }   // indent line

    QTextStream&   m_stream;
    int            m_indention;
};


class XmlGenerator : public AbstractGenerator
{
  public:
    XmlGenerator(QTextStream& out) : AbstractGenerator(out) { m_stream << "<report>" << endl; }
    ~XmlGenerator() { m_stream << "</report>" << endl; }
    void startBlock(const AsterixBlock& block) override;
    void finishBlock(const AsterixBlock& block) override;
    void startRecord(const AsterixRecord& record) override;
    void finishRecord(const AsterixRecord& record) override;
    void startDataItem(const AsterixDataItem& dataItem, const UapDataItem& uap, int indent = 0) override;
    void finishDataItem(const AsterixDataItem& dataItem, const UapDataItem& uap, int indent = 0) override;
    void startFinishField(const AsterixDataItem& dataItem, const UapField& uap, int indent = 0) override;

    static QString wrapInCData(const QString& s);
};


#endif // ASTERIXREPORTGENERATOR_H
