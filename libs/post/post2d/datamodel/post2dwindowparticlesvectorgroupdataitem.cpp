#include "../post2dwindowgraphicsview.h"
#include "post2dwindowgridtypedataitem.h"
#include "post2dwindowparticlesvectorgroupdataitem.h"
#include "post2dwindowparticlesvectordataitem.h"
#include "post2dwindowzonedataitem.h"

#include <guibase/vtkdatasetattributestool.h>
#include <guicore/named/namedgraphicswindowdataitemtool.h>
#include <guicore/misc/targeted/targeteditemsettargetcommandtool.h>
#include <guicore/postcontainer/postzonedatacontainer.h>
#include <guicore/scalarstocolors/lookuptablecontainer.h>
#include <guicore/solverdef/solverdefinitiongridtype.h>
#include <postbase/postparticlevectorpropertydialog.h>
#include <misc/iricundostack.h>
#include <misc/stringtool.h>

#include <QDomElement>
#include <QFile>
#include <QMenu>
#include <QMouseEvent>
#include <QSettings>
#include <QStandardItem>
#include <QTextStream>
#include <QUndoCommand>
#include <QVector2D>
#include <QXmlStreamWriter>

#include <vtkActor2DCollection.h>
#include <vtkCamera.h>
#include <vtkDataSetMapper.h>
#include <vtkDiskSource.h>
#include <vtkGeometryFilter.h>
#include <vtkLine.h>
#include <vtkLineSource.h>
#include <vtkPointData.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkProperty.h>
#include <vtkProperty2D.h>
#include <vtkRenderer.h>
#include <vtkStructuredGrid.h>
#include <vtkTextProperty.h>
#include <vtkTriangle.h>

#include <cmath>

Post2dWindowParticlesVectorGroupDataItem::Post2dWindowParticlesVectorGroupDataItem(Post2dWindowDataItem* p) :
	Post2dWindowDataItem(tr("Vector"), QIcon(":/libs/guibase/images/iconFolder.png"), p)
{
	setupStandardItem(Checked, NotReorderable, NotDeletable);
	m_oldCameraScale = 1;

	QSettings setting;
	m_scaleFactor = setting.value("graphics/vectorfactor", 1).value<double>();

	PostZoneDataContainer* cont = dynamic_cast<Post2dWindowZoneDataItem*>(parent()->parent())->dataContainer();
	SolverDefinitionGridType* gt = cont->gridType();
	for (std::string name : vtkDataSetAttributesTool::getArrayNamesWithMultipleComponents(cont->data()->GetPointData())) {
		auto item = new Post2dWindowParticlesVectorDataItem(name, gt->solutionCaption(name), this);
		m_childItems.append(item);
	}
	setupActors();
}

Post2dWindowParticlesVectorGroupDataItem::~Post2dWindowParticlesVectorGroupDataItem()
{
	renderer()->RemoveActor(m_arrowActor);
}

void Post2dWindowParticlesVectorGroupDataItem::handleNamedItemChange(NamedGraphicWindowDataItem* item)
{
	if (m_isCommandExecuting) {return;}

	auto cmd = TargetedItemSetTargetCommandTool::buildFromNamedItem(item, this, tr("Vector Attribute Select Change"));
	pushRenderCommand(cmd, this, true);
}

void Post2dWindowParticlesVectorGroupDataItem::setupActors()
{
	m_arrowActor = vtkSmartPointer<vtkActor>::New();
	renderer()->AddActor(m_arrowActor);
	m_arrowActor->GetProperty()->LightingOff();

	m_arrowMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	m_arrowActor->SetMapper(m_arrowMapper);

	m_hedgeHog = vtkSmartPointer<vtkHedgeHog>::New();
	m_hedgeHog->SetVectorModeToUseVector();
	m_hedgeHog->SetScaleFactor(m_scaleFactor);

	m_warpVector = vtkSmartPointer<vtkWarpVector>::New();

	m_arrowGlyph = vtkSmartPointer<vtkGlyph3D>::New();
	m_arrowGlyph->SetScaleModeToDataScalingOff();
	m_arrowGlyph->SetVectorModeToUseVector();
	m_arrowGlyph->SetInputConnection(m_warpVector->GetOutputPort());

	m_arrowSource = vtkSmartPointer<vtkConeSource>::New();
	m_arrowGlyph->SetSourceConnection(m_arrowSource->GetOutputPort());

	m_appendPolyData = vtkSmartPointer<vtkAppendPolyData>::New();
	m_appendPolyData->AddInputConnection(m_hedgeHog->GetOutputPort());
	m_appendPolyData->AddInputConnection(m_arrowGlyph->GetOutputPort());

	m_polyData = vtkSmartPointer<vtkPolyData>::New();
	m_arrowMapper->SetInputData(m_polyData);

	m_arrowActor->VisibilityOff();

	m_legendTextActor = vtkSmartPointer<vtkTextActor>::New();
	m_legendTextActor->SetTextScaleModeToNone();
	m_legendTextActor->GetPositionCoordinate()->SetCoordinateSystemToNormalizedViewport();
	m_legendTextActor->SetPosition(0.75, 0.02);
	vtkTextProperty* prop = m_legendTextActor->GetTextProperty();
	prop->SetColor(0, 0, 0);
	prop->SetFontFamilyToArial();
	prop->SetJustificationToCentered();
	prop->SetVerticalJustificationToBottom();

	m_legendTextActor->VisibilityOff();
	renderer()->AddActor2D(m_legendTextActor);

	m_baseArrowPolyData = vtkSmartPointer<vtkUnstructuredGrid>::New();

	vtkSmartPointer<vtkPolyDataMapper2D> mapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
	vtkSmartPointer<vtkGeometryFilter> f = vtkSmartPointer<vtkGeometryFilter>::New();
	f->SetInputData(m_baseArrowPolyData);
	mapper->SetInputConnection(f->GetOutputPort());

	m_baseArrowActor = vtkSmartPointer<vtkActor2D>::New();
	m_baseArrowActor->SetMapper(mapper);

	m_baseArrowActor->GetPositionCoordinate()->SetCoordinateSystemToNormalizedViewport();
	m_baseArrowActor->GetPositionCoordinate()->SetValue(.75, .02);
	m_baseArrowActor->GetProperty()->SetColor(0, 0, 0);
	m_baseArrowActor->VisibilityOff();

	PostZoneDataContainer* cont = dynamic_cast<Post2dWindowZoneDataItem*>(parent()->parent())->dataContainer();
	if (cont == nullptr || cont->particleData() == nullptr) {return;}

	m_arrowMask = vtkSmartPointer<vtkMaskPoints>::New();
	m_arrowMask->SetInputData(cont->particleData());

	m_hedgeHog->SetInputConnection(m_arrowMask->GetOutputPort());
	m_warpVector->SetInputConnection(m_arrowMask->GetOutputPort());

	renderer()->AddActor2D(m_baseArrowActor);
}

void Post2dWindowParticlesVectorGroupDataItem::calculateStandardValue()
{
	if (m_arrowSetting.lengthMode == ArrowSettingContainer::LengthMode::Custom) {return;}
	QVector<double> lenVec;
	PostZoneDataContainer* cont = dynamic_cast<Post2dWindowZoneDataItem*>(parent()->parent())->dataContainer();
	if (cont == nullptr || cont->particleData() == nullptr) {return;}
	vtkPointSet* ps = cont->particleData();
	if (m_arrowSetting.target == "") {return;}

	vtkPointData* pd = ps->GetPointData();
	vtkDataArray* da = pd->GetArray(m_arrowSetting.target);
	for (vtkIdType i = 0; i < da->GetNumberOfTuples(); ++i) {
		double* v = da->GetTuple3(i);
		QVector2D vec(*(v), *(v + 1));
		lenVec.append(vec.length());
	}
	qSort(lenVec);
	double sum = 0;
	int count = AUTO_AVERAGECOUNT;
	if (count > lenVec.count()) {count = lenVec.count();}
	for (int i = 0; i < count; ++i) {
		sum += lenVec.at(lenVec.count() - i - 1);
	}
	double average = sum / count;
	if (average == 0) {average = 1;}

	int p = 0;
	double p2 = 1;
	while (average > 10) {
		average /= 10.;
		++ p;
		p2 = 10;
	}
	while (average < 1) {
		average *= 10;
		++ p;
		p2 = 0.1;
	}
	average = static_cast<int>(average);
	for (int i = 0; i < p; ++i) {
		average *= p2;
	}
	// now average is calculated.
	m_arrowSetting.setStandardValue(average);
	// minimum value is always 0.001 * standard value when auto mode.
	m_arrowSetting.setMinimumValue(average * 0.001);
}

void Post2dWindowParticlesVectorGroupDataItem::informGridUpdate()
{
	updateActorSettings();
}

void Post2dWindowParticlesVectorGroupDataItem::updateActorSettings()
{
	m_arrowActor->VisibilityOff();
	m_legendTextActor->VisibilityOff();
	m_baseArrowActor->VisibilityOff();

	m_actorCollection->RemoveAllItems();
	m_actor2DCollection->RemoveAllItems();

	PostZoneDataContainer* cont = dynamic_cast<Post2dWindowZoneDataItem*>(parent()->parent())->dataContainer();
	if (cont == nullptr || cont->particleData() == nullptr) {return;}
	vtkPointSet* ps = cont->particleData();
	if (m_arrowSetting.target == "") {return;}
	vtkPointData* pd = ps->GetPointData();
	if (pd->GetNumberOfArrays() == 0) {return;}
	pd->SetActiveVectors(m_arrowSetting.target);

	calculateStandardValue();
	updateMaskSetting();
	updateColorSetting();
	updatePolyData();
	updateLegendData();

	m_actorCollection->AddItem(m_arrowActor);
	m_actor2DCollection->AddItem(m_legendTextActor);
	m_actor2DCollection->AddItem(m_baseArrowActor);
	updateVisibilityWithoutRendering();
}

void Post2dWindowParticlesVectorGroupDataItem::updateColorSetting()
{
	Post2dWindowGridTypeDataItem* typedi = dynamic_cast<Post2dWindowGridTypeDataItem*>(parent()->parent()->parent());
	const QColor& cColor = m_arrowSetting.customColor();
	switch (m_arrowSetting.colorMode()) {
	case ArrowSettingContainer::ColorMode::Custom:
		m_arrowMapper->ScalarVisibilityOff();
		m_arrowActor->GetProperty()->SetColor(cColor.redF(), cColor.greenF(), cColor.blueF());
		break;
	case ArrowSettingContainer::ColorMode::ByScalar:
		m_arrowMapper->ScalarVisibilityOn();
		LookupTableContainer* stc = typedi->particleLookupTable(m_arrowSetting.colorTarget);
		m_arrowMapper->SetScalarModeToUsePointFieldData();
		m_arrowMapper->SelectColorArray(m_arrowSetting.colorTarget.c_str());
		m_arrowMapper->SetLookupTable(stc->vtkObj());
		m_arrowMapper->UseLookupTableScalarRangeOn();
		break;
	}
}

void Post2dWindowParticlesVectorGroupDataItem::updateMaskSetting()
{
	PostZoneDataContainer* cont = dynamic_cast<Post2dWindowZoneDataItem*>(parent()->parent())->dataContainer();
	vtkPolyData* data = cont->particleData();
	switch (ArrowSettingContainer::SamplingMode(m_arrowSetting.samplingMode)) {
	case ArrowSettingContainer::SamplingMode::All:
		m_arrowMask->SetOnRatio(1);
		m_arrowMask->RandomModeOff();
		m_arrowMask->SetMaximumNumberOfPoints(data->GetNumberOfPoints());
		break;
	case ArrowSettingContainer::SamplingMode::Rate:
		m_arrowMask->SetOnRatio(m_arrowSetting.samplingRate);
		m_arrowMask->SetMaximumNumberOfPoints(data->GetNumberOfPoints());
		break;
	case ArrowSettingContainer::SamplingMode::Number:
		m_arrowMask->RandomModeOn();
		m_arrowMask->SetMaximumNumberOfPoints(m_arrowSetting.samplingNumber);
		break;
	}
}

void Post2dWindowParticlesVectorGroupDataItem::updateZDepthRangeItemCount()
{
	m_zDepthRange.setItemCount(2);
}

void Post2dWindowParticlesVectorGroupDataItem::assignActionZValues(const ZDepthRange& range)
{
	m_arrowActor->SetPosition(0, 0, range.max());
}

void Post2dWindowParticlesVectorGroupDataItem::update()
{
	informGridUpdate();
}

std::string Post2dWindowParticlesVectorGroupDataItem::target() const
{
	return m_target;
}

void Post2dWindowParticlesVectorGroupDataItem::setTarget(const std::string& target)
{
	NamedGraphicsWindowDataItemTool::checkItemWithName(target, m_childItems);
	m_arrowSetting.target = target;
	updateActorSettings();
}

void Post2dWindowParticlesVectorGroupDataItem::innerUpdate2Ds()
{
	vtkCamera* cam = renderer()->GetActiveCamera();
	double scale = cam->GetParallelScale();
	if (scale != m_oldCameraScale) {
		updatePolyData();
		updateLegendData();
	}
	m_oldCameraScale = scale;
}

void Post2dWindowParticlesVectorGroupDataItem::updatePolyData()
{
	PostZoneDataContainer* cont = dynamic_cast<Post2dWindowZoneDataItem*>(parent()->parent())->dataContainer();
	if (cont == nullptr || cont->particleData() == nullptr) {return;}
	if (m_arrowSetting.target == "") {return;}
	updateScaleFactor();
	double height = dataModel()->graphicsView()->stdRadius(m_arrowShape.arrowSize());
	m_hedgeHog->SetScaleFactor(m_scaleFactor);
	m_warpVector->SetScaleFactor(m_scaleFactor);
	m_arrowSource->SetHeight(height);
	m_arrowSource->SetAngle(15);
	m_arrowSource->Modified();

	m_appendPolyData->Update();
	m_polyData->DeepCopy(m_appendPolyData->GetOutput());
	m_arrowActor->GetProperty()->SetLineWidth(m_arrowShape.lineWidth());
	m_baseArrowActor->GetProperty()->SetLineWidth(m_arrowShape.lineWidth());
}

void Post2dWindowParticlesVectorGroupDataItem::updateScaleFactor()
{
	const auto& s = m_arrowSetting;
	double a = 1.0 / dataModel()->graphicsView()->stdRadius(1.0);
	m_scaleFactor = s.legendLength() / (a * s.standardValue());
}

void Post2dWindowParticlesVectorGroupDataItem::updateLegendData()
{
	double vectorOffset = 18;
	double arrowLen = m_arrowSetting.legendLength();
	m_baseArrowPolyData->Initialize();
	m_baseArrowPolyData->Allocate(3);

	vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
	points->SetDataTypeToDouble();
	m_baseArrowPolyData->SetPoints(points);
	// add line
	points->InsertNextPoint(- arrowLen * .5, vectorOffset, 0);
	points->InsertNextPoint(arrowLen * .5, vectorOffset, 0);
	vtkSmartPointer<vtkLine> line = vtkSmartPointer<vtkLine>::New();
	line->GetPointIds()->SetId(0, 0);
	line->GetPointIds()->SetId(1, 1);
	m_baseArrowPolyData->InsertNextCell(line->GetCellType(), line->GetPointIds());

	// add triangle
	points->InsertNextPoint(arrowLen * .5 - 8, vectorOffset + 3, 0);
	points->InsertNextPoint(arrowLen * .5 - 8, vectorOffset - 3, 0);
	vtkSmartPointer<vtkTriangle> tri = vtkSmartPointer<vtkTriangle>::New();
	tri->GetPointIds()->SetId(0, 1);
	tri->GetPointIds()->SetId(1, 2);
	tri->GetPointIds()->SetId(2, 3);
	m_baseArrowPolyData->InsertNextCell(tri->GetCellType(), tri->GetPointIds());

	const auto& s = m_arrowSetting;
	QString lenStr = QString("%1\n\n%2").arg(s.target).arg(s.standardValue);
	m_legendTextActor->SetInput(iRIC::toStr(lenStr).c_str());

	if (s.colorMode() == ArrowSettingContainer::ColorMode::Custom) {
		// specified color.
		const QColor& cColor = s.customColor();
		m_baseArrowActor->GetProperty()->SetColor(cColor.red() / 255., cColor.green() / 255., cColor.blue() / 255.);
	} else if (s.colorMode() == ArrowSettingContainer::ColorMode::ByScalar) {
		// always black.
		m_baseArrowActor->GetProperty()->SetColor(0, 0, 0);
	}
}

void Post2dWindowParticlesVectorGroupDataItem::doLoadFromProjectMainFile(const QDomNode& node)
{
	QDomElement elem = node.toElement();
	m_arrowSetting.load(node);
	m_arrowShape.load(node);
	m_oldCameraScale = elem.attribute("oldCameraScale").toDouble();
	updateScaleFactor();

	setTarget(m_arrowSetting.target);

	updateActorSettings();
}

void Post2dWindowParticlesVectorGroupDataItem::doSaveToProjectMainFile(QXmlStreamWriter& writer)
{
	m_arrowSetting.save(writer);
	m_arrowShape.save(writer);

	writer.writeAttribute("oldCameraScale", QString::number(m_oldCameraScale));
}

void Post2dWindowParticlesVectorGroupDataItem::informSelection(VTKGraphicsView* v)
{
	dynamic_cast<Post2dWindowZoneDataItem*>(parent()->parent())->initParticleBrowser();
}

void Post2dWindowParticlesVectorGroupDataItem::informDeselection(VTKGraphicsView* v)
{
	dynamic_cast<Post2dWindowZoneDataItem*>(parent()->parent())->clearParticleBrowser();
}

void Post2dWindowParticlesVectorGroupDataItem::mouseMoveEvent(QMouseEvent* event, VTKGraphicsView* v)
{
	v->standardMouseMoveEvent(event);
	dynamic_cast<Post2dWindowZoneDataItem*>(parent()->parent())->updateParticleBrowser(QPoint(event->x(), event->y()), v);
}

void Post2dWindowParticlesVectorGroupDataItem::mousePressEvent(QMouseEvent* event, VTKGraphicsView* v)
{
	v->standardMousePressEvent(event);
}

void Post2dWindowParticlesVectorGroupDataItem::mouseReleaseEvent(QMouseEvent* event, VTKGraphicsView* v)
{
	v->standardMouseReleaseEvent(event);
	dynamic_cast<Post2dWindowZoneDataItem*>(parent()->parent())->fixParticleBrowser(QPoint(event->x(), event->y()), v);
}

void Post2dWindowParticlesVectorGroupDataItem::addCustomMenuItems(QMenu* menu)
{
	QAction* abAction = dynamic_cast<Post2dWindowZoneDataItem*>(parent()->parent())->showParticleBrowserAction();
	menu->addAction(abAction);
}

QDialog* Post2dWindowParticlesVectorGroupDataItem::propertyDialog(QWidget* p)
{
	PostParticleVectorPropertyDialog* dialog = new PostParticleVectorPropertyDialog(p);
	Post2dWindowGridTypeDataItem* gtItem = dynamic_cast<Post2dWindowGridTypeDataItem*> (parent()->parent()->parent());
	PostZoneDataContainer* cont = dynamic_cast<Post2dWindowZoneDataItem*>(parent()->parent())->dataContainer();
	dialog->setZoneData(cont);

	dialog->setArrowSetting(m_arrowSetting);
	dialog->setArrowShape(m_arrowShape);

	return dialog;
}


class Post2dWindowParticlesVectorSetProperty : public QUndoCommand
{
public:
	Post2dWindowParticlesVectorSetProperty(const ArrowSettingContainer& newSetting, const ArrowShapeContainer& newShape, Post2dWindowParticlesVectorGroupDataItem* item)
		: QUndoCommand(QObject::tr("Update Vector Setting")) {

		m_newSetting = newSetting;
		m_newShape = newShape;

		m_oldSetting = item->m_arrowSetting;
		m_oldShape = item->m_arrowShape;
		m_item = item;
	}
	void undo() {
		m_item->setIsCommandExecuting(true);
		m_item->m_arrowSetting = m_oldSetting;
		m_item->m_arrowShape = m_oldShape;
		m_item->setAttribute(m_item->m_arrowSetting.attribute());
		m_item->updateActorSettings();
		m_item->renderGraphicsView();
		m_item->setIsCommandExecuting(false);
	}
	void redo() {
		m_item->setIsCommandExecuting(true);
		m_item->m_arrowSetting = m_newSetting;
		m_item->m_arrowShape = m_newShape;
		m_item->setAttribute(m_item->m_arrowSetting.attribute());
		m_item->updateActorSettings();
		m_item->renderGraphicsView();
		m_item->setIsCommandExecuting(false);
	}
private:
	ArrowSettingContainer m_oldSetting;
	ArrowShapeContainer m_oldShape;

	ArrowSettingContainer m_newSetting;
	ArrowShapeContainer m_newShape;

	Post2dWindowParticlesVectorGroupDataItem* m_item;
};

void Post2dWindowParticlesVectorGroupDataItem::handlePropertyDialogAccepted(QDialog* propDialog)
{
	PostParticleVectorPropertyDialog* dialog = dynamic_cast<PostParticleVectorPropertyDialog*> (propDialog);
	iRICUndoStack::instance().push(new Post2dWindowParticlesVectorSetProperty(dialog->arrowSetting(), dialog->arrowShape(), this));
}
