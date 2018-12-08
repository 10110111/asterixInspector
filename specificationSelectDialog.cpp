#include "specificationSelectDialog.h"
#include "ui_specificationSelectDialog.h"

#include <map>

#include "global.h"
#include "uap.h"


struct HeaderEntry
{
  QString title;
  QString url;
};


std::map<int, HeaderEntry> categoryHeader
{
  {  1, {"Monoradar Target Reports (Part 2a)", ""} },
  {  2, {"Monoradar Service Messages (Part2b)", ""} },
  {  3, {"Distribution of Synthetic Air Traffic Data", ""} },
  {  4, {"Safety Net Messages (Part 17)", ""} },
  {  7, {"Directed Interrogation Messages", ""} },
  {  8, {"Monoradar Derived Weather Information", ""} },
  {  9, {"Multisensor Derived Weather Information", ""} },
  { 10, {"Monosensor Surface Movement Data", ""} },
  { 11, {"Advanced-SMGCS Data", ""} },
  { 12, {"Monoradar Target Reports", ""} },
  { 13, {"Monoradar Service Messages", ""} },
  { 14, {"Monoradar Weather Reports", ""} },
  { 17, {"Mode S Surveillance Co-ordination Function messages", ""} },
  { 18, {"Mode S Data-Link Function messages", ""} },
  { 19, {"MLT System Status Messages", ""} },
  { 20, {"MLT Messages", ""} },
  { 21, {"ADS-B Messages", ""} },
  { 22, {"TIS-B Management Messages", ""} },
  { 23, {"CNS/ATM Ground Station Service Messages", ""} },
  { 24, {"ADS-C Messages", ""} },
  { 25, {"CNS/ATM Ground System Status Reports", ""} },
  { 30, {"Exchange of Air Situation Pictures", ""} },
  { 31, {"Sensors Information messages (transmission of surveillance biases to users)", ""} },
  { 32, {"Information provided by users to ARTAS", ""} },
  { 33, {"ADS-B Messages (Assigned for use by FAA)", ""} },
  { 34, {"Monoradar Service Messages (Part 2b - next version of Cat 002)", ""} },
  { 48, {"Monoradar Target Reports (Part 4 - next version of Cat 001)", ""} },
  { 61, {"SDPS Session and Service Control Messages", ""} },
  { 62, {"System Track Data (Part 9)", ""} },
  { 63, {"Sensor Status Messages", ""} },
  { 65, {"SDPS Service Status Messages", ""} },

  {128, {"Exchange of C3I messages", ""} },
  {150, {"Flight Data Messages", ""} },
  {151, {"Association Messages", ""} },
  {152, {"Time Stamp Messages", ""} },
  {153, {"Special Purpose Messages", ""} },
  {204, {"Recognised Air Picture", ""} },
  {221, {"Replaced by CAT024", ""} },
  {239, {"Foreign Object Debris (FOD)", ""} },
  {240, {"Radar Video Transmission", ""} },
  {247, {"Version Number Exchange", ""} },
};


SpecificationSelectDialog::SpecificationSelectDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::SpecificationSelectDialog)
{
  m_scrollWidget = 0;
  ui->setupUi(this);
  populateTable();
}


SpecificationSelectDialog::~SpecificationSelectDialog()
{
  delete ui;
}


void SpecificationSelectDialog::populateTable()
{
  if (!g_uap)
    return;

  ui->scrollArea->takeWidget();
  delete m_scrollWidget;
  m_radioButtonGroups.clear();
  m_radioButtonCategories.clear();

  m_scrollWidget = new QWidget;
  ui->scrollArea->setWidget(m_scrollWidget);
  m_scrollWidget->setLayout(new QVBoxLayout);

  foreach (int cat, g_uap->m_categories.uniqueKeys())
  {
    QFrame* frame = new QFrame;
    m_scrollWidget->layout()->addWidget(frame);
    frame->setFrameStyle(QFrame::Box | QFrame::Plain);
    QVBoxLayout* vbox1 = new QVBoxLayout;
    frame->setLayout(vbox1);

    QLabel* label = new QLabel(QString::number(cat));
    vbox1->addWidget(label);
    QFont font(label->font());
    font.setBold(true);
    label->setFont(font);

    if (categoryHeader.count(cat))
    {
      label->setText(QString("Cat %1: ").arg(cat) + categoryHeader[cat].title);
    }

    QVBoxLayout* vbox2 = new QVBoxLayout;
    vbox1->addLayout(vbox2);
    vbox2->setContentsMargins(8, 4, 0, 4);
    vbox2->setSpacing(8);

    m_radioButtonGroups.insert(cat, new QButtonGroup(m_scrollWidget));

    // iterate over defined UapCategory for current cat
    foreach (const UapCategory* uapCat, g_uap->m_categories.values(cat))
    {
      QRadioButton* button = new QRadioButton(uapCat->m_version + "\n" + uapCat->m_sourceFile);
      vbox2->addWidget(button);
      m_radioButtonCategories.insert(button, uapCat);
      m_radioButtonGroups[cat]->addButton(button);

      // if current UapCategory is selected for usage check button
      if (uapCat == g_uap->selectedUapCategory(cat))
      {
        button->setChecked(true);
      }
    }

    if (m_radioButtonGroups[cat]->checkedButton() == nullptr)
    {
      m_radioButtonGroups[cat]->buttons().first()->setChecked(true);
    }
  }

  m_scrollWidget->setStyleSheet("* {background:" + palette().color(QPalette::Base).name() + "}");  // set base color (usualy white) as background color
}


void SpecificationSelectDialog::accept()
{
  foreach (const QButtonGroup* group, m_radioButtonGroups)
  {
    Q_ASSERT(group->checkedButton());
    g_uap->selectUapCategory(m_radioButtonCategories[group->checkedButton()]);
  }

  QDialog::accept();
}

