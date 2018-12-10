/*
 * This application is free software and is released under the terms of
 * the BSD license. See LICENSE file for details.
 *
 * Copyright (c) 2010 Volker Poplawski (volker@openbios.org)
 */
#ifndef DATAITEMWIDGET_H
#define DATAITEMWIDGET_H

#include <QtGui>
#include <QtWebEngineWidgets>

#include "asterix.h"

class DataItemWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DataItemWidget(QWidget *parent = 0);

    void setDataItem(const AsterixBlock* block, const AsterixRecord* record, const AsterixDataItem* dataItem);

signals:

public slots:
    void clear() { m_webView->setContent(QByteArray()); }
    static QString printEnumeration(const UapField& uapField, int value);
    static QString applyUnitAndScale(const UapField& uapField, int value);

  protected:
    QString renderBitTable(const AsterixDataItem& dataItem, const UapField& uapField) const;

    QSize sizeHint() const { return QSize(100, 300); }

    QString      m_css;
    QWebEngineView* m_webView;
};

#endif // DATAITEMWIDGET_H
