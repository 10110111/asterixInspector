/*
 * This application is free software and is released under the terms of
 * the BSD license. See LICENSE file for details.
 *
 * Copyright (c) 2010 Volker Poplawski (volker@openbios.org)
 */

#include "mainWindow.h"
#include "ui_mainWindow.h"

#include "global.h"
#include "uap.h"
#include "asterixReportGenerator.h"
#include "specificationSelectDialog.h"


#include "scaleExpressionParser.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
  m_mapper = 0;
  m_currentBlock = 0;
  m_blockModel = 0;
  m_recordModel = 0;
  m_currentBlockIndex = -1;

  g_mainWindow = this;

  g_logger = new Logger(this);

  ui->setupUi(this);
  connect(ui->actionOpenFile, SIGNAL(triggered()), this, SLOT(openFile()));

  connect(ui->actionGenerateXmlReport, &QAction::triggered, this, &MainWindow::generateXmlReport);
  connect(ui->actionSpecificationSelection, SIGNAL(triggered()), this, SLOT(showSpecificationSelectionDialog()));

  connect(ui->actionAbout_Qt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
  connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(showAboutDialog()));
  connect(ui->actionQuit, SIGNAL(triggered()), qApp, SLOT(quit()));


  m_blockCountLabel = new QLabel;
  g_blockCountLabel = m_blockCountLabel;
  m_blockCountLabel->setText( "Blocks:   " );
  statusBar()->addPermanentWidget( m_blockCountLabel );
  m_cursorPosLabel = new QLabel;
  m_cursorPosLabel->setText( "Cursor Pos:   " );
  statusBar()->addPermanentWidget( m_cursorPosLabel );

  createDockWidgets();

  g_uap = new Uap;

  // collect directories to scan for specification files
  QStringList specDirectories;

  if (QFileInfo("asterixSpecification").isDir())
  {
    specDirectories << "asterixSpecification";
  }

#ifdef __linux__
  if (QFileInfo("/usr/share/asterixInspector/asterixSpecification").isDir())
  {
    specDirectories << "/usr/share/asterixInspector/asterixSpecification";
  }
  if (QFileInfo(QDir::homePath()+"/opt/asterixInspector/share/asterixSpecification").isDir())
  {
      specDirectories << QDir::homePath()+"/opt/asterixInspector/share/asterixSpecification";
  }
#endif

  if (not g_specsDirectory.isEmpty())
  {
    specDirectories << g_specsDirectory;
  }

  for (const QString& dir : specDirectories)
  {
    g_uap->readXml(dir);
  }

  if (g_uap->m_categories.isEmpty())
  {
    logWarn("No specification xml files found. Check directory 'asterixSpecification/'.");
  }

  if (not g_openFileName.isEmpty())
  {
    readFile(g_openFileName);
  }
}


MainWindow::~MainWindow()
{
  delete ui;
}


void MainWindow::createDockWidgets()
{
  QDockWidget* hexDock = new QDockWidget("Hex");
  m_hexEdit = new HexEdit(this);
  connect(m_hexEdit, SIGNAL(rescanRequested(qint64)), this, SLOT(mapFile(qint64)));
  connect(m_hexEdit, SIGNAL(cursorPosition(qint64)), this, SLOT(hexCursorPosChanged(qint64)));
  hexDock->setWidget(m_hexEdit);
  addDockWidget(Qt::RightDockWidgetArea, hexDock);

  m_detailDock = new QDockWidget("Data Item");
  m_dataItemWidget = new DataItemWidget(this);
  m_detailDock->setWidget(m_dataItemWidget);
  addDockWidget(Qt::RightDockWidgetArea, m_detailDock);

  m_recordWidget = new RecordWidget(0);

  QDockWidget* logDock = new QDockWidget("Log");
  logDock->setWidget(g_logger->textWidget());
  addDockWidget(Qt::BottomDockWidgetArea, logDock);
}


void MainWindow::openFile()
{
  QString fileName = QFileDialog::getOpenFileName(this, "Choose Asterix File");
  if (not fileName.isEmpty())
  {
    readFile(fileName);
  }
}


void MainWindow::showSpecificationSelectionDialog()
{
  SpecificationSelectDialog* dialog = new SpecificationSelectDialog(this);
  dialog->exec();
}


void MainWindow::readFile(const QString &filename)
{
  m_file.close();
  m_file.setFileName(filename);

  if (not m_file.open(QIODevice::ReadOnly))
    return;

  setWindowTitle(filename + " - AsterixInspector");

  g_file = m_file.map(0, m_file.size());
  g_fileSize = m_file.size();

  ui->m_fieldTable->setModel(0);
  ui->m_recordTable->setModel(0);
  m_dataItemWidget->setDataItem(0, 0, 0);
  m_recordWidget->setRecord(0);
  m_hexEdit->setData(g_file, g_fileSize);
  m_hexEdit->setCursorPosition(g_file);
  m_dataItemWidget->clear();

  mapFile(0);
}


void MainWindow::mapFile(qint64 ofs)
{
  ui->m_blockTable->setModel(0);
  delete g_asterixModel;
  delete m_blockModel;
  m_blockModel = 0;
  delete m_recordModel;
  m_recordModel = 0;
  m_currentBlockIndex = -1;
  m_hexEdit->setHighlight( 0, 0 );

  g_asterixModel = new AsterixModel;
  ui->m_blockTable->setModel(g_asterixModel);

  connect(ui->m_blockTable->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), this, SLOT(blockClicked(QModelIndex)));

  if (m_mapper)
  {
    m_mapper->stop();
    m_mapper->wait();
  }
  delete m_mapper;
  m_mapper = new AsterixFileMapper(&m_file);
  m_mapper->setOffset(ofs);
  g_asterixModel->reserve(qMax(100LL, (g_fileSize - ofs)/250));
  connect(m_mapper, SIGNAL(asterixBlock(QList<AsterixBlockInfo>)), g_asterixModel, SLOT(asterixBlock(QList<AsterixBlockInfo>)));
  connect(m_mapper, SIGNAL(blocksScanned(qint64)), g_asterixModel, SLOT(blocksScanned(qint64)));
  connect(m_mapper, SIGNAL(finished()), g_asterixModel, SLOT(squeeze()));

  m_mapper->start();
}



/** a asterix block has been selected in block-table
 **/
void MainWindow::blockClicked(const QModelIndex &index)
{
  if (index.row() != m_currentBlockIndex)
  {
    const AsterixBlockInfo blockInfo = g_asterixModel->blockInfo(index);
    m_currentBlockIndex = index.row();

    delete m_currentBlock;
    m_currentBlock = new AsterixBlock(g_file + blockInfo.offset, *g_uap);

    AsterixBlockModel* model = new AsterixBlockModel(g_file, *m_currentBlock, this);
    ui->m_recordTable->setModel(model);
    ui->m_fieldTable->setModel(0);
    m_dataItemWidget->setDataItem(0, 0, 0);
    m_recordWidget->setRecord(0);

    connect(ui->m_recordTable->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), this, SLOT(recordClicked(QModelIndex)));

    delete m_blockModel;
    m_blockModel = model;
  }

  m_hexEdit->setHighlight(m_currentBlock->data(), m_currentBlock->length(), true);
  m_hexEdit->setCursorPosition(m_currentBlock->data());
}


/** a block record has been selected in record-table
 **/
void MainWindow::recordClicked(const QModelIndex &index)
{
  m_currentRecord = &m_currentBlock->record(index.row());

  AsterixRecordModel* model = new AsterixRecordModel(g_file, *m_currentRecord, this);
  ui->m_fieldTable->setModel(model);
  m_dataItemWidget->setDataItem(0, 0, 0);

  connect(ui->m_fieldTable->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), this, SLOT(fieldClicked(QModelIndex)));

  delete m_recordModel;
  m_recordModel = model;

  m_hexEdit->setHighlight(m_currentRecord->data(), m_currentRecord->length(), true);
  m_hexEdit->setCursorPosition(m_currentRecord->data());

  m_detailDock->setWidget(m_recordWidget);
  m_detailDock->setWindowTitle("Record");
  m_recordWidget->setRecord(m_currentRecord);
}


void MainWindow::fieldClicked(const QModelIndex &index)
{
  m_currentDataItem = &m_recordModel->dataItemAtRow(index.row());

  m_hexEdit->setHighlight(m_currentDataItem->data(), m_currentDataItem->length(), true);
  m_hexEdit->setCursorPosition(m_currentDataItem->data());

  m_detailDock->setWidget(m_dataItemWidget);
  m_detailDock->setWindowTitle("Data Item");
  m_dataItemWidget->setDataItem(m_currentBlock, m_currentRecord, m_currentDataItem);
}


void MainWindow::hexCursorPosChanged(qint64 pos)
{
  m_cursorPosLabel->setText( QString("Cursor Pos: 0x%1").arg(pos, 0, 16) );
}


void MainWindow::generateXmlReport()
{
  QString fileName = QFileDialog::getSaveFileName(this, "Specify output file");
  if (fileName.isEmpty())
    return;

  QFile file(fileName);
  if (not file.open( QIODevice::WriteOnly | QIODevice::Text))
  {
    QMessageBox::warning(this, "Error", "Unable to open '" + fileName + "' for writing.");
    return;
  }

  QTextStream out(&file);
  AsterixReportGenerator reportGenerator;
  reportGenerator.createReport(out, AsterixReportGenerator::XML);
}


void MainWindow::showAboutDialog()
{
  const QString text = "<h3>AsterixInspector 0.12.3</h3>"
                 "displays contents of files in <a href=\"http://www.eurocontrol.int/asterix/\">Eurocontrol Asterix</a> format."
                 "<p>Written by Volker Poplawski and licensed under BSD license (see below)</p>"
                 "<p>Report bugs and suggestions to volker@openbios.org.</p>"
                 "<p><pre>Copyright (c) 2018, Volker Poplawski. All rights reserved.\n"
                 "\n"
                 "Redistribution and use in source and binary forms, with or without\n"
                 "modification, are permitted provided that the following conditions are met:\n"
                 "  * Redistributions of source code must retain the above copyright\n"
                 "    notice, this list of conditions and the following disclaimer.\n"
                 "  * Redistributions in binary form must reproduce the above copyright\n"
                 "    notice, this list of conditions and the following disclaimer in the\n"
                 "    documentation and/or other materials provided with the distribution.\n"
                 "  * Neither the name of the copyright holder nor the\n"
                 "    names of its contributors may be used to endorse or promote products\n"
                 "    derived from this software without specific prior written permission.\n"
                 "\n"
                 "THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\" AND\n"
                 "ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED\n"
                 "WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE\n"
                 "DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER(S) BE LIABLE FOR ANY\n"
                 "DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES\n"
                 "(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;\n"
                 "LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND\n"
                 "ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT\n"
                 "(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS\n"
                 "SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\n"
                 "</pre></p>";

  QMessageBox::about(this, "About AsterixInspector", text);
}
