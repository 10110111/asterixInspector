#include "TextHexDumpEdit.h"
#include "ui_TextHexDumpEdit.h"
#include "global.h"
#include <QMessageBox>

TextHexDumpEdit::TextHexDumpEdit(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TextHexDumpEdit)
{
    ui->setupUi(this);
}

TextHexDumpEdit::~TextHexDumpEdit()
{
    delete ui;
}

void TextHexDumpEdit::on_pushButton_clicked()
{
    QTextCursor cursor = ui->textEdit->textCursor();
    QString hexText = cursor.selectedText();
    if(hexText.isEmpty())
      hexText = ui->textEdit->toPlainText();
    if(!QRegExp("^\\s*([0-9a-fA-F]{2}\\s+)+$").exactMatch(hexText+' '))
    {
        QMessageBox::critical(this, tr("Failed to parse hex dump"), tr("Hex dump must be a sequence of pairs of hexadecimal digits separated by white space."));
        return;
    }

    QTemporaryFile file;
    if(file.open())
    {
        file.write(QByteArray::fromHex(hexText.toLocal8Bit()));
        file.close();
        emit hexDumpLoaded(file.fileName());
    }
}
