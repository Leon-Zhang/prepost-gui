#include "crosssectionwindow.h"
#include "ui_crosssectionwindow.h"

#include "../../data/crosssection/crosssection.h"
#include "../../data/crosssections/crosssections.h"
#include "../../data/project/project.h"
#include "../../misc/qwtcanvaswithpositionsignal.h"

#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_zoomer.h>
#include <qwt_symbol.h>

#include <QColor>
#include <QPen>
#include <QVector2D>

#include <map>

CrossSectionWindow::CrossSectionWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::CrossSectionWindow),
	m_currentCrossSection {nullptr}
{
	ui->setupUi(this);
	connect(ui->crossSectionComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(handleSelectionChange(int)));
	connect(ui->resetZoomButton, SIGNAL(clicked()), this, SLOT(resetZoom()));

	QwtCanvasWithPositionSignal* w = new QwtCanvasWithPositionSignal(ui->qwtWidget);
	connect(w, SIGNAL(positionChangedForStatusBar(QPointF)), this, SIGNAL(positionChangedForStatusBar(QPointF)));
	ui->qwtWidget->setCanvas(w);

	initCurve();
}

CrossSectionWindow::~CrossSectionWindow()
{
	delete m_waterElevationMarker;
	delete m_lbHWM;
	delete m_rbHWM;
	delete ui;
}

void CrossSectionWindow::setProject(Project* project)
{
	m_project = project;
	updateView();
}

void CrossSectionWindow::resetZoom()
{
	m_zoomer->zoom(0);
}

void CrossSectionWindow::updateView()
{
	// update combobox
	auto csVec = m_project->crossSections().crossSectionVector();
	if (csVec.size() == 0) {
		parentWidget()->close();
		return;
	}

	ui->crossSectionComboBox->blockSignals(true);

	int idx = -1;
	ui->crossSectionComboBox->clear();
	for (int i = 0; i < csVec.size(); ++i) {
		CrossSection* cs = csVec.at(i);
		ui->crossSectionComboBox->addItem(cs->name());
		if (m_currentCrossSection == cs) {
			idx = i;
		}
	}
	if (idx == -1) {
		idx = 0;
		m_currentCrossSection = csVec.at(0);
	}
	ui->crossSectionComboBox->setCurrentIndex(idx);
	ui->crossSectionComboBox->blockSignals(false);

	updateCurve();
	updateWindowTitle();
}

void CrossSectionWindow::handleSelectionChange(int selected)
{
	auto csVec = m_project->crossSections().crossSectionVector();
	m_currentCrossSection = csVec.at(selected);

	updateView();
}

void CrossSectionWindow::initCurve()
{
	auto qwtW = ui->qwtWidget;
	qwtW->setAxisTitle(QwtPlot::xBottom, tr("Distance from Left bank"));
	qwtW->enableAxis(QwtPlot::xBottom);

	qwtW->setAxisTitle(QwtPlot::yLeft, tr("Elevation"));
	qwtW->enableAxis(QwtPlot::yLeft);

	QwtPlotGrid* grid = new QwtPlotGrid();
	grid->enableXMin(true);
	grid->enableYMin(true);
	grid->setMajorPen(Qt::lightGray, 1.0, Qt::SolidLine);
	grid->setMinorPen(Qt::lightGray, 1.0, Qt::DashLine);
	grid->attach(ui->qwtWidget);

	m_curve = new QwtPlotCurve();
	m_curve->setPen(Qt::black);

	QwtSymbol* s = new QwtSymbol(QwtSymbol::Ellipse, QBrush(Qt::black), QPen(Qt::black), QSize(7, 7));
	m_curve->setSymbol(s);
	m_curve->attach(ui->qwtWidget);

	m_waterElevationMarker = new QwtPlotMarker();
	m_waterElevationMarker->setYValue(0);
	m_waterElevationMarker->setLineStyle(QwtPlotMarker::HLine);
	m_waterElevationMarker->setAxes(QwtPlot::xBottom, QwtPlot::yLeft);
	m_waterElevationMarker->setLinePen(QPen(Qt::blue));
	m_waterElevationMarker->attach(ui->qwtWidget);

	m_lbHWM = new QwtPlotMarker();
	m_lbHWM->setYValue(0);
	m_lbHWM->setLineStyle(QwtPlotMarker::HLine);
	m_lbHWM->setAxes(QwtPlot::xBottom, QwtPlot::yLeft);
	m_lbHWM->setLinePen(QPen(Qt::darkBlue, 1, Qt::DashLine));
	m_lbHWM->setLabelOrientation(Qt::Horizontal);
	m_lbHWM->setLabelAlignment(Qt::AlignRight | Qt::AlignBaseline);
	m_lbHWM->setLabel(tr("Left bank HWM"));
	m_lbHWM->attach(ui->qwtWidget);

	m_rbHWM = new QwtPlotMarker();
	m_rbHWM->setYValue(0);
	m_rbHWM->setLineStyle(QwtPlotMarker::HLine);
	m_rbHWM->setAxes(QwtPlot::xBottom, QwtPlot::yLeft);
	m_rbHWM->setLinePen(QPen(Qt::darkBlue, 1, Qt::DashLine));
	m_rbHWM->setLabelOrientation(Qt::Horizontal);
	m_rbHWM->setLabelAlignment(Qt::AlignRight | Qt::AlignBaseline);
	m_rbHWM->setLabel(tr("Right bank HWM"));
	m_rbHWM->attach(ui->qwtWidget);

	m_zoomer = new QwtPlotZoomer(ui->qwtWidget->canvas());
	m_zoomer->setRubberBandPen(QPen(Qt::black));
	m_zoomer->setTrackerPen(QPen(Qt::darkBlue));
	m_zoomer->setMousePattern(QwtEventPattern::MouseSelect1, Qt::LeftButton);
}

void CrossSectionWindow::updateCurve()
{
	if (m_currentCrossSection == nullptr) {return;}

	double xmin, xmax, ymin, ymax;


	QVector<QPointF> samples;

	bool first = true;
	for (auto p : m_currentCrossSection->mappedPoints()){
		samples.push_back(QPointF(p.x(), p.y()));

		if (first || p.x() < xmin) {xmin = p.x();}
		if (first || p.x() > xmax) {xmax = p.x();}
		if (first || p.y() < ymin) {ymin = p.y();}
		if (first || p.y() > ymax) {ymax = p.y();}

		first = false;
	}
	double xWidth = (xmax - xmin);
	double yWidth = (ymax - ymin);
	double marginRate = 0.08;

	xmin -= xWidth * marginRate;
	xmax += xWidth * marginRate;
	ymin -= yWidth * marginRate;
	ymax += yWidth * marginRate;

	m_curve->setSamples(samples);

	m_waterElevationMarker->setYValue(m_currentCrossSection->waterElevation());

	updateHWMs();

	ui->qwtWidget->setAxisScale(QwtPlot::xBottom, xmin, xmax);
	ui->qwtWidget->setAxisScale(QwtPlot::yLeft, ymin, ymax);

	ui->qwtWidget->replot();
	m_zoomer->setZoomBase(QRectF(QPointF(xmin, ymin), QPointF(xmax, ymax)));
}

void CrossSectionWindow::updateHWMs()
{
	m_lbHWM->detach();
	m_rbHWM->detach();

	double lbhwm = m_project->calcLeftBankHWMAtCrossSection(m_currentCrossSection);
	if (lbhwm != Project::INVALID_HWM) {
		m_lbHWM->setYValue(lbhwm);
		m_lbHWM->attach(ui->qwtWidget);
	}
	double rbhwm = m_project->calcRightBankHWMAtCrossSection(m_currentCrossSection);
	if (rbhwm != Project::INVALID_HWM) {
		m_rbHWM->setYValue(rbhwm);
		m_rbHWM->attach(ui->qwtWidget);
	}
}

void CrossSectionWindow::updateWindowTitle()
{
	if (m_currentCrossSection == nullptr) {
		setWindowTitle(tr("Cross Section Window"));
		return;
	}
	auto title = tr("Cross Section Window : %1").arg(m_currentCrossSection->name());
	setWindowTitle(title);
}
