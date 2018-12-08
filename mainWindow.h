/*
 * This application is free software and is released under the terms of
 * the BSD license. See LICENSE file for details.
 *
 * Copyright (c) 2010 Volker Poplawski (volker@openbios.org)
 */
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui>

#include "asterixModel.h"
#include "hexedit.h"
#include "dataItemWidget.h"
#include "recordWidget.h"


namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected Q_SLOTS:
    void openFile();
    void showSpecificationSelectionDialog();
    void readFile( const QString& filename );
    void mapFile(qint64 ofs);

    void blockClicked(const QModelIndex& index);
    void recordClicked(const QModelIndex& index);
    void fieldClicked(const QModelIndex& index);

    void hexCursorPosChanged(qint64 pos);

    void generateXmlReport();
    void showAboutDialog();

protected:
    Ui::MainWindow *ui;
    void createDockWidgets();

    QFile                   m_file;
    AsterixFileMapper*      m_mapper;

    AsterixBlockModel*      m_blockModel;
    AsterixRecordModel*     m_recordModel;

    AsterixBlock*           m_currentBlock;
    int                     m_currentBlockIndex;
    const AsterixRecord*    m_currentRecord;
    const AsterixDataItem*  m_currentDataItem;

    QLabel*                 m_blockCountLabel;
    QLabel*                 m_cursorPosLabel;

    HexEdit*                m_hexEdit;
    QDockWidget*            m_detailDock;
    DataItemWidget*         m_dataItemWidget;
    RecordWidget*           m_recordWidget;
};

#endif // MAINWINDOW_H
