#include "../post3dwindow.h"
#include "post3dwindowgridtypedataitem.h"
#include "post3dwindowzonedataitem.h"

#include <guibase/vtkdatasetattributestool.h>
#include <guicore/postcontainer/postsolutioninfo.h>
#include <guicore/postcontainer/postzonedatacontainer.h>
#include <guicore/pre/grid/grid.h>
#include <guicore/pre/gridcond/base/gridattributecontainer.h>
#include <guicore/scalarstocolors/lookuptablecontainer.h>
#include <guicore/solverdef/solverdefinitiongridattribute.h>
#include <guicore/solverdef/solverdefinitiongridtype.h>
#include <misc/stringtool.h>
#include <misc/xmlsupport.h>

#include <QAction>
#include <QList>
#include <QMenu>
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

Post3dWindowGridTypeDataItem::Post3dWindowGridTypeDataItem(SolverDefinitionGridType* type, GraphicsWindowDataItem* parent) :
	Post3dWindowDataItem {type->caption(), parent},
	m_gridType {type},
	m_isZoneDataItemsSetup {false}
{
	setupStandardItem(Checked, NotReorderable, NotDeletable);

	setSubPath(type->name().c_str());

	setupZoneDataItems();
}

Post3dWindowGridTypeDataItem::~Post3dWindowGridTypeDataItem()
{
	for (auto z_it = m_zoneDatas.begin(); z_it != m_zoneDatas.end(); ++z_it) {
		delete *z_it;
	}
}

const std::string& Post3dWindowGridTypeDataItem::name() const
{
	return m_gridType->name();
}

void Post3dWindowGridTypeDataItem::setupZoneDataItems()
{
	// first, clear the current zonedata.
	for (auto z_it = m_zoneDatas.begin(); z_it != m_zoneDatas.end(); ++z_it) {
		delete *z_it;
	}
	m_zoneDatas.clear();
	const QList<PostZoneDataContainer*>& zones = postSolutionInfo()->zoneContainers3D();
	int num = 0;
	int zoneNum = 0;
	for (auto it = zones.begin(); it != zones.end(); ++it) {
		const PostZoneDataContainer* cont = (*it);
		if (cont->data() == nullptr) {continue;}
		if (cont->gridType() == m_gridType) {
			Post3dWindowZoneDataItem* zdata = new Post3dWindowZoneDataItem(cont->zoneName(), num++, this);
			m_zoneDatas.append(zdata);
			m_zoneDataNameMap.insert(cont->zoneName(), zdata);
			m_childItems.append(zdata);
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
		if (m_particleLookupTables.count() == 0 && zones.size() != 0) {
			vtkPointData* pd = zCont->particleData()->GetPointData();
			for (std::string name : vtkDataSetAttributesTool::getArrayNamesWithOneComponent(pd)) {
				setupParticleScalarsToColors(name);
			}
			updateParticleLookupTableRanges();
		}
	}
	m_isZoneDataItemsSetup = (zoneNum != 0);
}

void Post3dWindowGridTypeDataItem::update()
{
	if (! m_isZoneDataItemsSetup) {
		setupZoneDataItems();
	}
	// update LookupTable range.
	updateNodeLookupTableRanges();
	updateParticleLookupTableRanges();

	for (Post3dWindowZoneDataItem* item : m_zoneDatas) {
		item->update();
	}
}

void Post3dWindowGridTypeDataItem::updateNodeLookupTableRanges()
{
	for (auto it = m_nodeLookupTables.begin(); it != m_nodeLookupTables.end(); ++it) {
		std::string name = it.key();
		ScalarsToColorsContainer* cont = it.value();
		bool first = true;
		double range[2], min = 0, max = 0;
		for (auto zit = m_zoneDatas.begin(); zit != m_zoneDatas.end(); ++zit) {
			Post3dWindowZoneDataItem* zitem = *zit;
			PostZoneDataContainer* cont = zitem->dataContainer();
			if (cont == nullptr) {continue;}
			if (cont->data() == nullptr) {continue;}
			vtkDataArray* dArray = cont->data()->GetPointData()->GetArray(name.c_str());
			if (dArray != nullptr) {
				dArray->GetRange(range);
				if (first || range[0] < min) {min = range[0];}
				if (first || range[1] > max) {max = range[1];}
				first = false;
			}
		}
		if (max - min < 1E-4) {
			// the width is too small.
			double mid = (min + max) * 0.5;
			double width = mid * 0.01;
			if (width < 1E-4) {
				width = 1E-4;
			}
			min = mid - width;
			max = mid + width;
		}
		cont->setValueRange(min, max);
	}
}

void Post3dWindowGridTypeDataItem::updateParticleLookupTableRanges()
{
	for (auto it = m_particleLookupTables.begin(); it != m_particleLookupTables.end(); ++it) {
		std::string name = it.key();
		ScalarsToColorsContainer* cont = it.value();
		bool first = true;
		double range[2], min, max;
		min = 0; max = 0;
		for (auto zit = m_zoneDatas.begin(); zit != m_zoneDatas.end(); ++zit) {
			Post3dWindowZoneDataItem* zitem = *zit;
			if (zitem->dataContainer() == nullptr || zitem->dataContainer()->particleData() == nullptr) {continue;}
			vtkDataArray* dArray = zitem->dataContainer()->particleData()->GetPointData()->GetArray(name.c_str());
			if (dArray != nullptr) {
				dArray->GetRange(range);
				if (first || range[0] < min) {min = range[0];}
				if (first || range[1] > max) {max = range[1];}
				first = false;
			}
		}
		if (max - min < 1E-4) {
			// the width is too small.
			double mid = (min + max) * 0.5;
			double width = mid * 0.01;
			if (width < 1E-4) {
				width = 1E-4;
			}
			min = mid - width;
			max = mid + width;
		}
		cont->setValueRange(min, max);
	}
}

void Post3dWindowGridTypeDataItem::doLoadFromProjectMainFile(const QDomNode& node)
{
	QDomNode ltNode = iRIC::getChildNode(node, "LookupTables");
	if (! ltNode.isNull()) {
		QDomNodeList tables = ltNode.childNodes();
		for (int i = 0; i < tables.length(); ++i) {
			QDomNode ltNode = tables.at(i);
			std::string ltName = iRIC::toStr(ltNode.toElement().attribute("name"));
			LookupTableContainer* cont = m_nodeLookupTables.value(ltName, nullptr);
			if (cont != nullptr) {
				cont->loadFromProjectMainFile(ltNode);
			}
		}
	}
	QDomNode pltNode = iRIC::getChildNode(node, "ParticleLookupTables");
	if (! pltNode.isNull()) {
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
	QDomNode zonesNode = iRIC::getChildNode(node, "Zones");
	if (! zonesNode.isNull()) {
		QDomNodeList zones = zonesNode.childNodes();
		for (int i = 0; i < zones.length(); ++i) {
			QDomNode zoneNode = zones.at(i);
			std::string zoneName = iRIC::toStr(zoneNode.toElement().attribute("name"));
			Post3dWindowZoneDataItem* zdi = m_zoneDataNameMap.value(zoneName, nullptr);
			if (zdi != nullptr) {
				zdi->loadFromProjectMainFile(zoneNode);
			}
		}
	}
}

void Post3dWindowGridTypeDataItem::doSaveToProjectMainFile(QXmlStreamWriter& writer)
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
	writer.writeStartElement("Zones");
	for (auto zit = m_zoneDatas.begin(); zit != m_zoneDatas.end(); ++zit) {
		Post3dWindowZoneDataItem* zitem = *zit;
		writer.writeStartElement("Zone");
		zitem->saveToProjectMainFile(writer);
		writer.writeEndElement();
	}
	writer.writeEndElement();
}

void Post3dWindowGridTypeDataItem::setupNodeScalarsToColors(const std::string& name)
{
	LookupTableContainer* c = new LookupTableContainer(this);
	m_nodeLookupTables.insert(name, c);
}

void Post3dWindowGridTypeDataItem::setupParticleScalarsToColors(const std::string& name)
{
	LookupTableContainer* c = new LookupTableContainer(this);
	m_particleLookupTables.insert(name, c);
}

void Post3dWindowGridTypeDataItem::setValueRange(const std::string& name)
{
	double min = -0.000001;
	double max = 0.000001;

	QList<PostZoneDataContainer*> containers = postSolutionInfo()->zoneContainers3D();

	bool first = true;
	for (auto it = containers.begin(); it != containers.end(); ++it) {
		vtkPointSet* ps = (*it)->data();
		if (ps == nullptr) { break; }
		double range[3];
		vtkDataArray* da = ps->GetPointData()->GetArray(name.c_str());
		if (da == nullptr) { break; }
		da->GetRange(range);
		if (first || range[0] < min) {min = range[0];}
		if (first || range[1] > max) {max = range[1];}
		first = false;
	}
	// Now, new min, max are stored.
	m_nodeLookupTables.value(name)->setValueRange(min, max);
}
