#ifndef POST2DWINDOWNODEVECTORARROWGROUPDATAITEM_H
#define POST2DWINDOWNODEVECTORARROWGROUPDATAITEM_H

#include "../post2dwindowdataitem.h"
#include <guibase/structuredgridregion.h>
#include <misc/arrowsettingcontainer.h>
#include <misc/compositecontainer.h>
#include <misc/colorcontainer.h>
#include <misc/stringcontainer.h>
#include <misc/doublecontainer.h>
#include <misc/enumcontainert.h>

#include <vtkSmartPointer.h>
#include <vtkActor.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkGlyph3D.h>
#include <vtkHedgeHog.h>
#include <vtkMaskPoints.h>
#include <vtkArrowSource.h>
#include <vtkConeSource.h>
#include <vtkWarpVector.h>
#include <vtkExtractGrid.h>
#include <vtkAppendPolyData.h>
#include <vtkActor2D.h>
#include <vtkTextActor.h>
#include <vtkClipPolyData.h>

#include <QColor>

class vtkPointSet;
class Post2dWindowNodeVectorArrowDataItem;

class Post2dWindowNodeVectorArrowGroupDataItem : public Post2dWindowDataItem
{
	Q_OBJECT

public:
	const static int STANDARD_LENGTH = 100;
	const static int AUTO_AVERAGECOUNT = 20;
	const static double MINLIMIT;
	enum Mapping {Specific, Scalar};
	enum LegendMode {lmAuto, lmCustom};
	enum LengthMode {lenAuto, lenCustom};

	struct Setting : public CompositeContainer
	{
		/// Constructor
		Setting();

		StringContainer scalarValueName;
		StringContainer currentSolution;
		ColorContainer color;
		DoubleContainer oldCameraScale;
		DoubleContainer scaleFactor;
		EnumContainerT<StructuredGridRegion::RegionMode> regionMode;
		EnumContainerT<Mapping> mapping;
		EnumContainerT<LegendMode> legendMode;
		EnumContainerT<LengthMode> lengthMode;
		DoubleContainer standardValue;
		DoubleContainer legendLength;
		DoubleContainer minimumValue;
		ArrowSettingContainer arrowSetting;
	};


	/// Constructor
	Post2dWindowNodeVectorArrowGroupDataItem(Post2dWindowDataItem* parent);
	~Post2dWindowNodeVectorArrowGroupDataItem();
	void informSelection(VTKGraphicsView* v) override;
	void informDeselection(VTKGraphicsView* v) override;
	void mouseMoveEvent(QMouseEvent* event, VTKGraphicsView* v) override;
	void mouseReleaseEvent(QMouseEvent* event, VTKGraphicsView* v) override;
	void updateActorSettings();
	void updateZDepthRangeItemCount() override;
	void assignActorZValues(const ZDepthRange& range) override;
	void addCustomMenuItems(QMenu* menu) override;
	void update();

public slots:
	void exclusivelyCheck(Post2dWindowNodeVectorArrowDataItem*);

protected:
	void innerUpdate2Ds() override;
	void updateColorSetting();
	void updatePolyData();
	void updateLegendData();
	void informGridUpdate();
	virtual void updateActivePoints() = 0;
	void setCurrentSolution(const QString& currentSol);
	QString currentSolution() const {return m_setting.currentSolution;}
	void doLoadFromProjectMainFile(const QDomNode& node) override;
	void doSaveToProjectMainFile(QXmlStreamWriter& writer) override;

private:
	void setupActors();
	void calculateStandardValue();
	void updateScaleFactor();

protected:
	vtkSmartPointer<vtkActor> m_arrowActor;
	vtkSmartPointer<vtkPolyDataMapper> m_arrowMapper;
	vtkSmartPointer<vtkAppendPolyData> m_appendPolyData;
	vtkSmartPointer<vtkPolyData> m_polyData;

	vtkSmartPointer<vtkHedgeHog> m_hedgeHog;
	vtkSmartPointer<vtkGlyph3D> m_arrowGlyph;
	vtkSmartPointer<vtkWarpVector> m_warpVector;
	vtkSmartPointer<vtkConeSource> m_arrowSource;

	vtkSmartPointer<vtkTextActor> m_legendTextActor;

	vtkSmartPointer<vtkUnstructuredGrid> m_baseArrowPolyData;
	vtkSmartPointer<vtkActor2D> m_baseArrowActor;
	vtkSmartPointer<vtkPolyData> m_activePoints;

	Setting m_setting;
/*
	QString m_scalarValueName;
	QString m_currentSolution;
	QColor m_color;
	double m_oldCameraScale;
	double m_scaleFactor;
	StructuredGridRegion::RegionMode m_regionMode;
	Mapping m_mapping;
	LegendMode m_legendMode;
	LengthMode m_lengthMode;
	double m_standardValue;
	double m_legendLength;
	double m_minimumValue;
	ArrowSettingContainer m_arrowSetting;
*/

public:
	friend class Post2dWindowGridArrowSelectSolution;
	friend class Post2dWindowArrowStructuredSetProperty;
	friend class Post2dWindowArrowUnstructuredSetProperty;
};

#endif // POST2DWINDOWNODEVECTORARROWGROUPDATAITEM_H
