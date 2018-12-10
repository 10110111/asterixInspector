#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include <QWidget>

namespace Ui
{
class TextHexDumpEdit;
}

class TextHexDumpEdit : public QWidget
{
    Q_OBJECT

public:
    explicit TextHexDumpEdit(QWidget *parent = nullptr);
    ~TextHexDumpEdit();
signals:
    void hexDumpLoaded(const QString &filename);
private slots:
    void on_pushButton_clicked();

private:
    Ui::TextHexDumpEdit *ui;
};

#endif // TEXTEDIT_H
