#include "geodatapointmapwebimporterregionselectdialog.h"
#include "ui_geodatapointmapwebimporterregionselectdialog.h"

#include <guicore/tmsimage/tmsimagesetting.h>
#include <guicore/tmsimage/tmsimagesettingmanager.h>

GeoDataPointmapWebImporterRegionSelectDialog::GeoDataPointmapWebImporterRegionSelectDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::GeoDataPointmapWebImporterRegionSelectDialog)
{
	ui->setupUi(this);

	connect(ui->mapComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(switchMap(int)));

	connect(ui->zoomInButton, SIGNAL(clicked()), ui->mapWidget, SLOT(zoomIn()));
	connect(ui->zoomOutButton, SIGNAL(clicked()), ui->mapWidget, SLOT(zoomOut()));

	connect(ui->nextButton, SIGNAL(clicked()), this, SLOT(accept()));
	connect(ui->cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

	TmsImageSettingManager manager;

	for (auto s : manager.settings()) {
		if (! s.isActive()) {continue;}

		m_settings.push_back(s);
		ui->mapComboBox->addItem(s.caption());
	}
}

GeoDataPointmapWebImporterRegionSelectDialog::~GeoDataPointmapWebImporterRegionSelectDialog()
{
	delete ui;
}

void GeoDataPointmapWebImporterRegionSelectDialog::setCoordinateSystem(CoordinateSystem* cs)
{
	ui->mapWidget->setCoordinateSystem(cs);
}

double GeoDataPointmapWebImporterRegionSelectDialog::minLon() const
{
	return ui->mapWidget->minLon();
}

double GeoDataPointmapWebImporterRegionSelectDialog::maxLon() const
{
	return ui->mapWidget->maxLon();
}

double GeoDataPointmapWebImporterRegionSelectDialog::minLat() const
{
	return ui->mapWidget->minLat();
}

double GeoDataPointmapWebImporterRegionSelectDialog::maxLat() const
{
	return ui->mapWidget->maxLat();
}

void GeoDataPointmapWebImporterRegionSelectDialog::switchMap(int index)
{
	auto s = m_settings.at(index);
	ui->mapWidget->setMapSetting(s.setting());
}
