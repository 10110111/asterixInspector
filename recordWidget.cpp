/*
 * This application is free software and is released under the terms of
 * the BSD license. See LICENSE file for details.
 *
 * Copyright (c) 2010 Volker Poplawski (volker@openbios.org)
 */
#include "recordWidget.h"

RecordWidget::RecordWidget(QWidget *parent) :
    QWidget(parent)
{
  QVBoxLayout* vbox = new QVBoxLayout;
  setLayout(vbox);

  m_textEdit = new QTextEdit(this);
  m_textEdit->setReadOnly(true);
  layout()->addWidget(m_textEdit);
}


void RecordWidget::setRecord(const AsterixRecord *record)
{
  m_textEdit->clear();

  if (record == 0)
    return;

  const uchar* const p = record->data();  // points to fspec bytes

  int numFspec = 1 + AsterixBlock::countExtends(p);   // get number of fspec bytes
  QList<int> frns = AsterixBlock::decodeFspecSection(p, numFspec);

  QString html;
  html += "<h3>Record field specification</h3>";
  html += "<p><table cellspacing=\"0\" border=\"0\" style=\"\">";

  html += "<tr style=\"font-family:monospace; font-size:large;\">";
  html += "<td>&nbsp;</td>";  // row header
  for (int f = 0; f < numFspec; f++)
  {
    QString byteHex = QString("%1").arg((uint)p[f], 2, 16, QChar('0')).toUpper();
    html += "<td colspan=\"4\" align=\"center\" style=\"background-color:#e1ebfa; padding-top:2px; padding-bottom:2px;\">" + byteHex[0] + "</td>"
            "<td colspan=\"4\" align=\"center\" style=\"background-color:#e1ebfa; padding-top:2px; padding-bottom:2px;\">" + byteHex[1] + "</td>";
    html += "<td>&nbsp;</td>";

  }
  html += "</tr>";

  html += "<tr style=\"font-family:monospace;\">";
  html += "<td>&nbsp;</td>";  // row header
  for (int f = 0; f < numFspec; f++)
  {
    QString byteBin = QString("%1").arg((uint)p[f], 8, 2, QChar('0'));
    for (int b = 0; b < 8; b++)
    {
      html += "<td align=\"center\">" + byteBin[b] + "</td>";
    }
    html += "<td>&nbsp;</td>";
  }
  html += "</tr>";

  html += "<tr>";
  html += "<td align=\"right\" style=\"white-space:pre; padding-top:5px; padding-right:2px;\">FRNs</td>";   // row header
  for (int f = 0; f < numFspec; f++)
  {
    for (int frn = f * 7 + 1; frn < f * 7 + 1 + 7; frn++)
    {
      html += "<td align=\"center\" style=\"white-space:pre; padding-top:5px; padding-left:2px; padding-right:2px;\"><nobr>";

      if (frn < 10)
        html += "&nbsp;";

      if (frns.contains(frn))
        html += "<u>";

      html += QString::number(frn);

      if (frns.contains(frn))
        html += "</u>";

      if (frn < 10)
        html += "&nbsp;";

      html += "</nobr></td>";
    }

    if (f < numFspec - 1)
      html += "<td style=\"white-space:pre; padding-top:5px; padding-left:2px; padding-right:2px;\"><u>fx</u></td>";
    else
      html += "<td style=\"white-space:pre; padding-top:5px; padding-left:2px; padding-right:2px;\">fx</td>";
    html += "<td>&nbsp;</td>";
  }
  html += "</tr>";

  html += "</tr>";
  html += "</table></p>";

  m_textEdit->setHtml(html);
}
