#include "../post2dwindow.h"
#include "post2dwindowgridtypedataitem.h"
#include "post2dwindowgeodatatopdataitem.h"
#include "post2dwindowzonedataitem.h"

#include <guibase/vtkdatasetattributestool.h>
#include <guicore/base/iricmainwindowinterface.h>
#include <guicore/postcontainer/postsolutioninfo.h>
#include <guicore/postcontainer/postzonedatacontainer.h>
#include <guicore/pre/base/preprocessorgeodatatopdataiteminterface.h>
#include <guicore/pre/base/preprocessorwindowinterface.h>
#include <guicore/pre/grid/grid.h>
#include <guicore/pre/gridcond/base/gridattributecontainer.h>
#include <guicore/scalarstocolors/lookuptablecontainer.h>
#include <guicore/scalarstocolors/scalarstocolorscontainerutil.h>
#include <guicore/solverdef/solverdefinitiongridattribute.h>
#include <guicore/solverdef/solverdefinitiongridtype.h>
#include <misc/stringtool.h>
#include <misc/xmlsupport.h>

#include <QAction>
#include <QDomNode>
#include <QList>
#include <QMenu>
#include <QStandardItem>
#include <QXmlStreamWriter>

#include <vtkPointData.h>

namespace {

PostZoneDataContainer* getContainerWithZoneType(const QList<PostZoneDataContainer*>& conts, SolverDefinitionGridType* gt)
{
	for (auto container : conts) {
		if (container->gridType() == gt) {
			return container;
		}
	}
	return nullptr;
}

} // namespace

Post2dWindowGridTypeDataItem::Post2dWindowGridTypeDataItem(SolverDefinitionGridType* type, GraphicsWindowDataItem* parent) :
	Post2dWindowDataItem {type->caption(), QIcon(":/libs/guibase/images/iconFolder.png"), parent},
	m_gridType {type},
	m_isZoneDataItemsSetup {false}
{
	setupStandardItem(Checked, NotReorderable, NotDeletable);

	setSubPath(type->name().c_str());

	PreProcessorGeoDataTopDataItemInterface* tItem = dataModel()->iricMainWindow()->preProcessorWindow()->dataModel()->geoDataTopDataItem(type->name());
	if (tItem != nullptr) {
		m_geoDataItem = new Post2dWindowGeoDataTopDataItem(tItem, this);
		m_childItems.push_back(m_geoDataItem);
	}

	setupZoneDataItems();
}

Post2dWindowGridTypeDataItem::~Post2dWindowGridTypeDataItem()
{
	for (auto z_it = m_zoneDatas.begin(); z_it != m_zoneDatas.end(); ++z_it) {
		delete *z_it;
	}
}

const std::string& Post2dWindowGridTypeDataItem::name()
{
	return m_gridType->name();
}

void Post2dWindowGridTypeDataItem::setupZoneDataItems()
{
	// first, clear the current zonedata.
	for (auto z_it = m_zoneDatas.begin(); z_it != m_zoneDatas.end(); ++z_it) {
		delete *z_it;
	}
	m_zoneDatas.clear();
	const QList<PostZoneDataContainer*>& zones = postSolutionInfo()->zoneContainers2D();
	int num = 0;
	int zoneNum = 0;
	for (auto it = zones.begin(); it != zones.end(); ++it) {
		const PostZoneDataContainer* cont = (*it);
		if (cont->gridType() == m_gridType) {
			Post2dWindowZoneDataItem* zdata = new Post2dWindowZoneDataItem(cont->zoneName(), num++, this);
			m_zoneDatas.append(zdata);
			m_zoneDataNameMap.insert(cont->zoneName(), zdata);
			m_childItems.push_back(zdata);
			++ zoneNum;
		}
	}

	PostZoneDataContainer* zCont = getContainerWithZoneType(zones, m_gridType);

	if (zCont != nullptr) {
		if (m_nodeLookupTables.count() == 0 && zones.size() != 0) {
			vtkPointData* pd = zCont->data()->GetPointData();
			for (std::string name : vtkDataSetAttributesTool::getArrayNamesWithOneComponent(pd)) {
				setupNodeScalarsToColors(name);
			}
			updateNodeLookupTableRanges();
		}
		if (zCont->particleData() != nullptr && m_particleLookupTables.count() == 0 && zones.size() != 0) {
			vtkPointData* pd = zCont->particleData()->GetPointData();
			for (std::string name : vtkDataSetAttributesTool::getArrayNamesWithOneComponent(pd)) {
				setupParticleScalarsToColors(name);
			}
			updateParticleLookupTableRanges();
		}
	}
	assignActorZValues(m_zDepthRange);
	m_isZoneDataItemsSetup = (zoneNum != 0);
	updateItemMap();
}

void Post2dWindowGridTypeDataItem::update()
{
	if (! m_isZoneDataItemsSetup) {
		setupZoneDataItems();
	}
	// update LookupTable range.
	updateNodeLookupTableRanges();
	updateParticleLookupTableRanges();

	// update child items.
	for (Post2dWindowZoneDataItem* item : m_zoneDatas) {
		item->update();
	}
}

void Post2dWindowGridTypeDataItem::updateNodeLookupTableRanges()
{
	for (auto it = m_nodeLookupTables.begin(); it != m_nodeLookupTables.end(); ++it) {
		std::string name = it.key();
		ScalarsToColorsContainer* cont = it.value();
		std::vector<vtkDataArray*> da;
		for (auto zit = m_zoneDatas.begin(); zit != m_zoneDatas.end(); ++zit) {
			Post2dWindowZoneDataItem* zitem = *zit;
			if (zitem->dataContainer() == nullptr || zitem->dataContainer()->data() == nullptr) {continue;}
			vtkDataArray* dArray = zitem->dataContainer()->data()->GetPointData()->GetArray(name.c_str());
			if (dArray == nullptr) {continue;}
			da.push_back(dArray);
		}
		ScalarsToColorsContainerUtil::setValueRange(cont, da);
	}
}

void Post2dWindowGridTypeDataItem::updateParticleLookupTableRanges()
{
	for (auto it = m_particleLookupTables.begin(); it != m_particleLookupTables.end(); ++it) {
		std::string name = it.key();
		ScalarsToColorsContainer* cont = it.value();
		std::vector<vtkDataArray*> da;
		for (auto zit = m_zoneDatas.begin(); zit != m_zoneDatas.end(); ++zit) {
			Post2dWindowZoneDataItem* zitem = *zit;
			if (zitem->dataContainer() == nullptr || zitem->dataContainer()->particleData() == nullptr) {continue;}
			vtkDataArray* dArray = zitem->dataContainer()->particleData()->GetPointData()->GetArray(name.c_str());
			if (dArray == nullptr) {continue;}
			da.push_back(dArray);
		}
		ScalarsToColorsContainerUtil::setValueRange(cont, da);
	}
}

void Post2dWindowGridTypeDataItem::doLoadFromProjectMainFile(const QDomNode& node)
{
	QDomNode ltNode = iRIC::getChildNode(node, "LookupTables");
	if (! ltNode.isNull()) {
		QDomNodeList tables = ltNode.childNodes();
		for (int i = 0; i < tables.length(); ++i) {
			QDomNode ltNode = tables.at(i);
			std::string ltName = iRIC::toStr(ltNode.toElement().attribute("name"));
			LookupTableContainer* cont = m_nodeLookupTables.value(ltName, 0);
			if (cont != nullptr) {
				cont->loadFromProjectMainFile(ltNode);
			}
		}
	}
	QDomNode pltNode = iRIC::getChildNode(node, "ParticleLookupTables");
	if (!pltNode.isNull()) {
		QDomNodeList tables = pltNode.childNodes();
		for (int i = 0; i < tables.length(); ++i) {
			QDomNode ltNode = tables.at(i);
			std::string ltName = iRIC::toStr(ltNode.toElement().attribute("name"));
			LookupTableContainer* cont = m_particleLookupTables.value(ltName, nullptr);
			if (cont != nullptr) {
				cont->loadFromProjectMainFile(ltNode);
			}
		}
	}
	QDomNode rNode = iRIC::getChildNode(node, "GeoData");
	if (! rNode.isNull()) {
		m_geoDataItem->loadFromProjectMainFile(rNode);
	}
	QDomNode zonesNode = iRIC::getChildNode(node, "Zones");
	if (! zonesNode.isNull()) {
		QDomNodeList zones = zonesNode.childNodes();
		for (int i = 0; i < zones.length(); ++i) {
			QDomNode zoneNode = zones.at(i);
			std::string zoneName = iRIC::toStr(zoneNode.toElement().attribute("name"));
			Post2dWindowZoneDataItem* zdi = m_zoneDataNameMap.value(zoneName, nullptr);
			if (zdi != nullptr) {
				zdi->loadFromProjectMainFile(zoneNode);
			}
		}
	}
}

void Post2dWindowGridTypeDataItem::doSaveToProjectMainFile(QXmlStreamWriter& writer)
{
	writer.writeAttribute("name", m_gridType->name().c_str());
	writer.writeStartElement("LookupTables");
	for (auto lit = m_nodeLookupTables.begin(); lit != m_nodeLookupTables.end(); ++lit) {
		writer.writeStartElement("LookupTable");
		writer.writeAttribute("name", lit.key().c_str());
		LookupTableContainer* cont = lit.value();
		cont->saveToProjectMainFile(writer);
		writer.writeEndElement();
	}
	writer.writeEndElement();
	writer.writeStartElement("ParticleLookupTables");
	for (auto lit = m_particleLookupTables.begin(); lit != m_particleLookupTables.end(); ++lit) {
		writer.writeStartElement("LookupTable");
		writer.writeAttribute("name", lit.key().c_str());
		LookupTableContainer* cont = lit.value();
		cont->saveToProjectMainFile(writer);
		writer.writeEndElement();
	}
	writer.writeEndElement();
	writer.writeStartElement("GeoData");
	m_geoDataItem->saveToProjectMainFile(writer);
	writer.writeEndElement();
	writer.writeStartElement("Zones");
	for (auto zit = m_zoneDatas.begin(); zit != m_zoneDatas.end(); ++zit) {
		Post2dWindowZoneDataItem* zitem = *zit;
		writer.writeStartElement("Zone");
		zitem->saveToProjectMainFile(writer);
		writer.writeEndElement();
	}
	writer.writeEndElement();
}

void Post2dWindowGridTypeDataItem::setupNodeScalarsToColors(const std::string& name)
{
	LookupTableContainer* c = new LookupTableContainer(this);
	m_nodeLookupTables.insert(name, c);
}

void Post2dWindowGridTypeDataItem::setupParticleScalarsToColors(const std::string& name)
{
	LookupTableContainer* c = new LookupTableContainer(this);
	m_particleLookupTables.insert(name, c);
}
