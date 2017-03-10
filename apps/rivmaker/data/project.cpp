#include "project.h"
#include "riversurveydata.h"
#include "riversurveydatacreator.h"

#include "private/project_impl.h"

Project::Impl::Impl() :
	m_riverSurveyData {nullptr}
{}

Project::Impl::~Impl()
{
	delete m_riverSurveyData;
}

// public interfraces

Project::Project() :
	impl {new Impl {}}
{}

Project::~Project()
{
	delete impl;
}

const ElevationPoints& Project::elevationPoints() const
{
	return impl->m_elevationPoints;
}

ElevationPoints& Project::elevationPoints()
{
	return impl->m_elevationPoints;
}

const WaterSurfaceElevationPoints& Project::waterSurfaceElevationPoints() const
{
	return impl->m_waterSurfaceElevationPoints;
}

WaterSurfaceElevationPoints& Project::waterSurfaceElevationPoints()
{
	return impl->m_waterSurfaceElevationPoints;
}

const BaseLine& Project::baseLine() const
{
	return impl->m_baseLine;
}

BaseLine Project::baseLine()
{
	return impl->m_baseLine;
}

bool Project::checkIfReadyToCreateRiverSurveyData(QWidget* w) const
{
	return RiverSurveyDataCreator::checkIfReadyToCreate(*this, w);
}

void Project::createRiverSurveyData()
{
	deleteRiverSurveyData();

	impl->m_riverSurveyData = RiverSurveyDataCreator::create(*this);
}

bool Project::hasRiverSurveyData() const
{
	return impl->m_riverSurveyData != nullptr;
}

RiverSurveyData* Project::riverSurveyData() const
{
	return impl->m_riverSurveyData;
}

void Project::deleteRiverSurveyData()
{
	if (impl->m_riverSurveyData == nullptr) {return;}

	delete impl->m_riverSurveyData;
	impl->m_riverSurveyData = nullptr;
}
