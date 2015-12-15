#include "post2dwindowparticlesscalardataitem.h"
#include "post2dwindowzonedataitem.h"

#include <guicore/datamodel/vtkgraphicsview.h>

#include <QMenu>
#include <QMouseEvent>

Post2dWindowParticlesScalarDataItem::Post2dWindowParticlesScalarDataItem(const std::string& name, const QString& caption, GraphicsWindowDataItem* parent) :
	NamedGraphicWindowDataItem(name, caption, parent)
{}

Post2dWindowParticlesScalarDataItem::~Post2dWindowParticlesScalarDataItem()
{}

void Post2dWindowParticlesScalarDataItem::informSelection(VTKGraphicsView*)
{
	dynamic_cast<Post2dWindowZoneDataItem*>(parent()->parent()->parent())->initParticleBrowser();
}

void Post2dWindowParticlesScalarDataItem::informDeselection(VTKGraphicsView*)
{
	dynamic_cast<Post2dWindowZoneDataItem*>(parent()->parent()->parent())->clearParticleBrowser();
}

void Post2dWindowParticlesScalarDataItem::mouseMoveEvent(QMouseEvent* event, VTKGraphicsView* v)
{
	v->standardMouseMoveEvent(event);
	dynamic_cast<Post2dWindowZoneDataItem*>(parent()->parent()->parent())->updateParticleBrowser(QPoint(event->x(), event->y()), v);
}

void Post2dWindowParticlesScalarDataItem::mousePressEvent(QMouseEvent* event, VTKGraphicsView* v)
{
	v->standardMousePressEvent(event);
}

void Post2dWindowParticlesScalarDataItem::mouseReleaseEvent(QMouseEvent* event, VTKGraphicsView* v)
{
	v->standardMouseReleaseEvent(event);
	dynamic_cast<Post2dWindowZoneDataItem*>(parent()->parent()->parent())->fixParticleBrowser(QPoint(event->x(), event->y()), v);
}

void Post2dWindowParticlesScalarDataItem::addCustomMenuItems(QMenu* menu)
{
	QAction* abAction = dynamic_cast<Post2dWindowZoneDataItem*>(parent()->parent()->parent())->showParticleBrowserAction();
	menu->addAction(abAction);
}

void Post2dWindowParticlesScalarDataItem::doLoadFromProjectMainFile(const QDomNode&)
{}

void Post2dWindowParticlesScalarDataItem::doSaveToProjectMainFile(QXmlStreamWriter&)
{}
