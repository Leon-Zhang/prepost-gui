#include "preprocessorbcgroupdataitem.h"
#include "preprocessorbcgroupdataitem.h"
#include "preprocessorgridattributecellgroupdataitem.h"
#include "preprocessorgridattributenodegroupdataitem.h"
#include "preprocessorgridtypedataitem.h"
#include "preprocessorstructured2dgriddataitem.h"
#include "preprocessorstructured2dgridshapedataitem.h"

#include <guicore/solverdef/solverdefinitiongridtype.h>

#include <QAction>
#include <QMenu>

PreProcessorStructured2dGridDataItem::PreProcessorStructured2dGridDataItem(PreProcessorDataItem* parent) :
	PreProcessorGridDataItem {parent}
{
	m_shapeDataItem = new PreProcessorStructured2dGridShapeDataItem(this);
	m_childItems.push_back(m_shapeDataItem);
	m_nodeGroupDataItem = new PreProcessorGridAttributeNodeGroupDataItem(this);
	m_childItems.push_back(m_nodeGroupDataItem);
	m_cellGroupDataItem = new PreProcessorGridAttributeCellGroupDataItem(this);
	m_childItems.push_back(m_cellGroupDataItem);

	m_selectMenu = nullptr;
	m_regionSelectAction = new QAction(PreProcessorStructured2dGridDataItem::tr("Select I-J &Region..."), this);
	// @todo not implemented, so disabled.
	m_regionSelectAction->setDisabled(true);
	setupMenu();
	updateObjectBrowserTree();
}

PreProcessorStructured2dGridDataItem::~PreProcessorStructured2dGridDataItem()
{}

void PreProcessorStructured2dGridDataItem::setupMenu()
{
	m_editMenu = m_menu->addMenu(tr("&Edit"));
	m_editMenu->addAction(m_shapeDataItem->editAction());
	m_editMenu->addAction(m_nodeEditAction);
	m_editMenu->addAction(m_cellEditAction);

	if (m_bcGroupDataItem != nullptr) {
		m_editMenu->addMenu(m_bcGroupDataItem->bcMenu());
	}
	m_menu->addAction(m_deleteAction);
	m_menu->addSeparator();

	m_displayMenu = m_menu->addMenu(tr("Dis&play Setting..."));
	m_displayMenu->addAction(m_displaySettingAction);
	m_displayMenu->addAction(m_nodeDisplaySettingAction);
	m_displayMenu->addAction(m_cellDisplaySettingAction);
	m_displayMenu->addSeparator();
	m_displayMenu->addAction(m_setupScalarBarAction);
}

void PreProcessorStructured2dGridDataItem::updateActionStatus()
{
	m_editMenu->setEnabled(m_grid != nullptr);
	m_displayMenu->setEnabled(m_grid != nullptr);

	PreProcessorGridDataItem::updateActionStatus();
}
