#include "preferencepagetms.h"
#include "ui_preferencepagetms.h"
#include "private/preferencepagetmsadddialog.h"

#include <guicore/tmsimage/tmsimagesettingmanager.h>

PreferencePageTms::PreferencePageTms(QWidget *parent) :
	PreferencePage {parent},
	ui {new Ui::PreferencePageTms}
{
	ui->setupUi(this);

	TmsImageSettingManager manager;
	m_settings = manager.settings();

	connect(ui->addButton, SIGNAL(clicked()), this, SLOT(add()));
	connect(ui->deleteButton, SIGNAL(clicked()), this, SLOT(deleteSelected()));
	connect(ui->defaultButton, SIGNAL(clicked()), this, SLOT(restoreDefault()));

	connect(ui->upButton, SIGNAL(clicked()), this, SLOT(moveUpSelected()));
	connect(ui->downButton, SIGNAL(clicked()), this, SLOT(moveDownSelected()));

	connect(ui->listWidget, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(handleListWidgetItemChange(QListWidgetItem*)));

	updateList();
}

PreferencePageTms::~PreferencePageTms()
{
	delete ui;
}

void PreferencePageTms::update()
{
	TmsImageSettingManager manager;
	manager.setSettings(m_settings);
}

void PreferencePageTms::add()
{
	PreferencePageTmsAddDialog dialog(this);
	int ret = dialog.exec();
	if (ret == QDialog::Rejected) {return;}

	m_settings.push_back(dialog.setting());
	updateList();

	ui->listWidget->setCurrentRow(ui->listWidget->count() - 1);
}

void PreferencePageTms::deleteSelected()
{
	int row = ui->listWidget->currentRow();
	if (row == -1) {return;}

	m_settings.erase(m_settings.begin() + row);
	updateList();

	if (row >= ui->listWidget->count()){
		row = ui->listWidget->count() - 1;
	}
	ui->listWidget->setCurrentRow(row);
}

void PreferencePageTms::restoreDefault()
{
	TmsImageSettingManager manager;
	m_settings = manager.defaultSettings();

	updateList();
}

void PreferencePageTms::moveUpSelected()
{
	int row = ui->listWidget->currentRow();
	if (row < 1) {return;}

	auto s = m_settings.at(row);
	m_settings.erase(m_settings.begin() + row);
	m_settings.insert(m_settings.begin() + row - 1, s);

	updateList();
	ui->listWidget->setCurrentRow(row - 1);
}

void PreferencePageTms::moveDownSelected()
{
	int row = ui->listWidget->currentRow();
	if (row == ui->listWidget->count() - 1) {return;}

	auto s = m_settings.at(row);
	m_settings.erase(m_settings.begin() + row);
	m_settings.insert(m_settings.begin() + row + 1, s);

	updateList();
	ui->listWidget->setCurrentRow(row + 1);
}

void PreferencePageTms::handleListWidgetItemChange(QListWidgetItem *item)
{
	int changedRow = -1;
	for (int i = 0; i < ui->listWidget->count(); ++i) {
		auto rowItem = ui->listWidget->item(i);
		if (item == rowItem) {
			changedRow = i;
		}
	}
	if (changedRow == -1) {return;}

	TmsImageSetting& s = m_settings[changedRow];
	s.setIsActive(item->checkState() == Qt::Checked);
}

void PreferencePageTms::updateList()
{
	ui->listWidget->clear();
	for (const TmsImageSetting& setting : m_settings) {
		auto item = new QListWidgetItem(setting.caption());
		item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
		if (setting.isActive()) {
			item->setCheckState(Qt::Checked);
		} else {
			item->setCheckState(Qt::Unchecked);
		}
		ui->listWidget->addItem(item);
	}
}
