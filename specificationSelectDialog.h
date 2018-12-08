#ifndef SPECIFICATIONSELECTDIALOG_H
#define SPECIFICATIONSELECTDIALOG_H

#include <QtWidgets>

namespace Ui {
class SpecificationSelectDialog;
}

class UapCategory;

class SpecificationSelectDialog : public QDialog
{
  Q_OBJECT

  public:
    explicit SpecificationSelectDialog(QWidget *parent = 0);
    ~SpecificationSelectDialog();

    void populateTable();

  public slots:
    virtual void accept() override;

  private:
    Ui::SpecificationSelectDialog           *ui;
    QWidget                                 *m_scrollWidget;
    QMap<int, QButtonGroup*>                 m_radioButtonGroups;
    QMap<QAbstractButton*, const UapCategory*>  m_radioButtonCategories;
};

#endif // SPECIFICATIONSELECTDIALOG_H
