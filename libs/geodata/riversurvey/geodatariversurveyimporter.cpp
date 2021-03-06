#include "geodatarivercrosssection.h"
#include "geodatariverpathpoint.h"
#include "geodatariversurvey.h"
#include "geodatariversurveyimporter.h"
#include "geodatariversurveyimportersettingdialog.h"

#include <misc/errormessage.h>
#include <misc/stringtool.h>

#include <QFile>
#include <QMessageBox>
#include <QMultiMap>
#include <QTextStream>
#include <QVector2D>

#include <cmath>
#include <iomanip>
#include <sstream>

#define	SEPARATOR	" \t"

namespace {

const double WRONG_KP = -9999;

} // namespace

PRivPath GeoDataRiverSurveyImporter::RivAlloc(double KP, const char* str)
{
	PRivPath node, p;

	// Search
	for (p = m_RivRoot; p != NULL; p = p->next) {
		if (KP == WRONG_KP) {
			if (p->strKP == str){
				return p;
			}
		} else {
			if (p->KP == KP) {
				return p;
			}
		}
	}

	// Not found.
	if ((node = new RivPath()) == nullptr) {return nullptr;}

	node->KP    = KP;
	node->strKP = str;
	node->bank = NULL;
	node->np   = 0;
	node->pt   = NULL;
	node->next = NULL;

	// add to link list.
	if (! m_RivRoot) {
		m_RivRoot = node;
	} else {
		for (p = m_RivRoot; p->next != NULL; p = p->next)
			;
		p->next = node;
	}

	return node;
}

// Free memory
void GeoDataRiverSurveyImporter::RivFree(PRivPath node)
{
	PRivPath p;

	if (node->bank) { free(node->bank); }
	if (node->pt) { free(node->pt); }

	if (node == m_RivRoot) {
		m_RivRoot = node->next;
	} else {
		for (p = m_RivRoot; p->next != node; p = p->next)
			;
		p->next = node->next;
	}

	node->next = NULL;
	free(node);
}

// Free all memory.
void GeoDataRiverSurveyImporter::RivFreeAll()
{
	PRivPath p, pn;

	p = m_RivRoot;
	while (p) {
		if (p->bank) { free(p->bank); }
		if (p->pt) { free(p->pt); }
		pn = p->next;
		free(p);
		p = pn;
	}

	m_RivRoot = NULL;
}

// Set left bank position and right bank position
bool GeoDataRiverSurveyImporter::RivSetBank(PRivPath node, PPoint2D left, PPoint2D right)
{
	if (! node) {
		return false;
	}

	if ((node->bank = (PPoint2D) malloc(sizeof(Point2D) * 2)) == NULL) {
		return false;
	}

	node->bank[0] = *left;
	node->bank[1] = *right;

	return true;
}

// Set the center position
bool GeoDataRiverSurveyImporter::RivSetPath(PRivPath node, int np, PPoint2D pt)
{
	if (! node) {
		return false;
	}

	node->np = np;
	node->pt = pt;

	return true;
}

bool GeoDataRiverSurveyImporter::RivSort(void)
{
	PRivPath p, pn, *pa;
	int i, j, k, n;

	// Delete the data that does not have both position info and crosssection info
	p = m_RivRoot;
	n = 0;
	while (p) {
		pn = p->next;
		if (! p->bank || p->np == 0) {
			RivFree(p);
		} else {
			n++;
		}
		p = pn;
	}

	if ((pa = (PRivPath*) malloc(sizeof(PRivPath) * n)) == NULL) {
		return false;
	}

	n = 0;
	for (p = m_RivRoot; p != NULL; p = p->next) {
		pa[n] = p;
		n++;
	}

	// There is no crosssection, or maybe the file is corrupted. Error.
	if (n < 1) {
		return false;
	}

	if (! m_allNamesAreNumber) {
		m_RivRoot = pa[0];
		return true;
	}

	// Sort by KP
	for (i = 0; i < n; i++) {
		k = i;
		for (j = i + 1; j < n; j++) {
			if (pa[j]->KP > pa[k]->KP) {
				k = j;
			}
		}
		if (k != i) {
			p     = pa[i];
			pa[i] = pa[k];
			pa[k] = p;
		}
	}

	p = m_RivRoot = pa[0];
	for (i = 1; i < n; i++) {
		p->next = pa[i];
		p       = pa[i];
	}
	p->next = NULL;
	return true;
}

// Read river survey data
bool GeoDataRiverSurveyImporter::RivRead(const QString& name, bool* with4points)
{
	QFile file(name);
	char*         buf;
	char*         tok;
	char*         strKP;
	int mode = 0;
	double KP;
	int np, i, j;
	int divIndices[4];
	Point2D right, left;
	PPoint2D pt;
	PRivPath node;
	QString qstrKP;
	bool ok;

	m_allNamesAreNumber = true;

	*with4points = true;
	m_RivRoot = nullptr;
	// Open river survey data file
	file.open(QIODevice::ReadOnly | QIODevice::Text);

	while (! file.atEnd()) {
		QByteArray line = file.readLine();
		line = line.replace(static_cast<char>(10), "\0");
		buf = line.data();

		// separate the buffer with SEPARATOR.
		tok = strtok(buf, SEPARATOR);

		if (tok != NULL) {
			if (QString(tok) == "#survey") {
				mode = 1;
				continue;
			} else if (QString(tok) == "#x-section") {
				mode = 2;
				continue;
			}
			switch (mode) {
			case 1:
				strKP = tok;
				qstrKP = strKP;
				KP = qstrKP.toDouble(&ok);
				if (! ok) {
					m_allNamesAreNumber = false;
					KP = WRONG_KP;
				}
				tok = strtok(NULL, SEPARATOR); left.x  = (double) atof(tok);
				tok = strtok(NULL, SEPARATOR); left.y  = (double) atof(tok);
				tok = strtok(NULL, SEPARATOR); right.x = (double) atof(tok);
				tok = strtok(NULL, SEPARATOR); right.y = (double) atof(tok);
				node = RivAlloc(KP, strKP);
				RivSetBank(node, &left, &right);
				break;

			case 2:
				strKP = tok;
				qstrKP = strKP;
				KP = qstrKP.toDouble(&ok);
				if (! ok) {
					m_allNamesAreNumber = false;
					KP = WRONG_KP;
				}
				tok = strtok(NULL, SEPARATOR);
				if (tok == NULL) {break;}
				np = atoi(tok);
				// Tries to read four indices values.
				for (int k = 0; k < 4; ++k) {
					if (NULL != (tok = strtok(NULL, SEPARATOR))) {
						divIndices[k] = atoi(tok);
					} else {
						*with4points = false;
					}
				}
				node = RivAlloc(KP, strKP);
				//printf( "%f,%d\n", KP,np );
				if ((pt = (PPoint2D) malloc(sizeof(Point2D) * np)) == NULL) {
					printf("error\n");
				}
				i = j = 0;
				while (i < np) {
					if (file.atEnd()) {
						break;
					}
					line = file.readLine();
					line = line.replace(static_cast<char>(10), "\0");
					buf = line.data();
					tok = strtok(buf, SEPARATOR);
					while (tok && i < np) {
						switch (j) {
						case 0: pt[i].x = (double) atof(tok); j = 1; break;
						case 1: pt[i].y = (double) atof(tok); j = 0; i++; break;
						}
						tok = strtok(NULL, SEPARATOR);
					}
				}
				if (*with4points) {
					for (int i = 0; i < 4; ++i) {
						node->divIndices[i] = divIndices[i];
					}
				}
				RivSetPath(node, np, pt);
				break;

			default:
				// ERROR!
				;
			}

		}

	}
	file.close();
	// reading succeeded only when mode == 2.
	if (mode != 2) {return false;}

	return RivSort();
}

GeoDataRiverSurveyImporter::GeoDataRiverSurveyImporter(GeoDataCreator* creator) :
	GeoDataImporter("riversurvey", tr("River Survey data (*.riv)"), creator)
{}

bool GeoDataRiverSurveyImporter::importData(GeoData* data, int /*index*/, QWidget* w)
{
	bool ret;
	bool with4points;
	// Read river survey data
	ret = RivRead(filename(), &with4points);
	if (! ret) {return false;}

	GeoDataRiverSurveyImporterSettingDialog dialog(w);
	dialog.setWith4Points(with4points);
	int dialogret = dialog.exec();
	if (dialogret == QDialog::Rejected) {return false;}
	m_cpSetting = dialog.centerPointSetting();

	GeoDataRiverPathPoint* tail, *newpoint;

	GeoDataRiverSurvey* rs = dynamic_cast<GeoDataRiverSurvey*>(data);
	tail = rs->m_headPoint;

	PRivPath p;
	double max = 0, left = 0, right = 0;
	double minpos = 0, minval = 0;
	int i;
	bool ok = true;

	for (p = m_RivRoot; p != NULL; p = p->next) {
		if (with4points) {
			// Define river center position as the mid-point of the left bank and right bank.
			newpoint = new GeoDataRiverPathPoint((p->bank[0].x + p->bank[1].x) * 0.5, (p->bank[0].y + p->bank[1].y) * 0.5);
			double offset = std::sqrt((p->bank[1].x - p->bank[0].x) * (p->bank[1].x - p->bank[0].x) + (p->bank[1].y - p->bank[0].y) * (p->bank[1].y - p->bank[0].y)) / 2.;

			// Defines direction vector (from left bank to right bank)
			QVector2D dir(p->bank[1].x - p->bank[0].x, p->bank[1].y - p->bank[0].y);
			// Normalize direction vector
			dir.normalize();

			std::ostringstream oss;
			std::ostringstream oss2;
			oss << std::showpoint << std::fixed << std::setprecision(3) << p->KP;
			oss2 << p->strKP;

			newpoint->setName(oss2.str().c_str());
			newpoint->InhibitInterpolatorUpdate = true;
			newpoint->setCrosssectionDirection(dir);
			max = p->pt[0].y;
			GeoDataRiverCrosssection::Altitude oldalt(0, 0);
			for (i = 0; i < p->np; i++) {
				if (p->pt[i].y > max) {
					max = p->pt[i].y;
				}
				GeoDataRiverCrosssection::Altitude alt(p->pt[i].x - offset, p->pt[i].y);
				// Left points, Rights points are removed.
				if (i + 1 < p->divIndices[0] || i + 1 > p->divIndices[3]) {continue;}
				// River center is set to the center of point 1, and 2
				if (i + 1 == p->divIndices[1]) {
					left = p->pt[i].x - offset;
					minpos = p->pt[i].x - offset;
					minval = p->pt[i].y;
				}
				if (i + 1 == p->divIndices[2]) {right = p->pt[i].x - offset;}
				if (p->pt[i].y < minval) {
					minpos = p->pt[i].x - offset;
					minval = p->pt[i].y;
				}
				newpoint->crosssection().AltitudeInfo().push_back(alt);
				if (i >= p->divIndices[0] && oldalt.position() > alt.position() && ok) {
					QMessageBox::warning(w, tr("Warning"), tr("Crosssection data is not ordered correctly at %1.").arg(newpoint->name()));
					ok = false;
				}
				oldalt = alt;
			}
			double shiftValue = 0;

			// no option to select lowest elevation point.
			shiftValue = (left + right) * 0.5;

			double leftPoint = (left - shiftValue) /
												 (newpoint->crosssection().leftBank().position() - shiftValue);
			double rightPoint = (right - shiftValue) /
													(newpoint->crosssection().rightBank().position() - shiftValue);
			newpoint->crosssection().setLeftShift(offset);
			newpoint->shiftCenter(shiftValue);
			newpoint->CenterToLeftCtrlPoints.push_back(leftPoint);
			newpoint->CenterToRightCtrlPoints.push_back(rightPoint);
			newpoint->InhibitInterpolatorUpdate = false;

			if (rs->m_headPoint == nullptr) {
				rs->m_headPoint = newpoint;
			} else {
				tail->addPathPoint(newpoint);
			}
			tail = newpoint;
		} else {
			// River center point
			newpoint = new GeoDataRiverPathPoint((p->bank[0].x + p->bank[1].x) * 0.5, (p->bank[0].y + p->bank[1].y) * 0.5);
			double offset = std::sqrt((p->bank[1].x - p->bank[0].x) * (p->bank[1].x - p->bank[0].x) + (p->bank[1].y - p->bank[0].y) * (p->bank[1].y - p->bank[0].y)) / 2.;

			// vector of direction fron left bank to right bank.
			QVector2D dir(p->bank[1].x - p->bank[0].x, p->bank[1].y - p->bank[0].y);
			dir.normalize();
			std::ostringstream oss;
			std::ostringstream oss2;
			oss << std::showpoint << std::fixed << std::setprecision(3) << p->KP;
			oss2 << p->strKP;

			newpoint->setName(oss2.str().c_str());
			newpoint->InhibitInterpolatorUpdate = true;
			newpoint->setCrosssectionDirection(dir);
			max = p->pt[0].y;
			GeoDataRiverCrosssection::Altitude oldalt(0, 0);
			minpos = p->pt[0].x - offset;
			minval = p->pt[0].y;
			for (i = 0; i < p->np; i++) {
				if (p->pt[i].y > max) {
					max = p->pt[i].y;
				}
				if (p->pt[i].y < minval) {
					minpos = p->pt[i].x - offset;
					minval = p->pt[i].y;
				}
				GeoDataRiverCrosssection::Altitude alt(p->pt[i].x - offset, p->pt[i].y);
				newpoint->crosssection().AltitudeInfo().push_back(alt);
				if (i > 0 && oldalt.position() > alt.position() && ok) {
					QMessageBox::warning(w, tr("Warning"), tr("Crosssection data is not ordered correctly at %1.").arg(newpoint->name()));
					ok = false;
				}
				oldalt = alt;
			}
			left  = newpoint->crosssection().leftBank().position();
			right = newpoint->crosssection().rightBank().position();

			double shiftValue = 0;
			if (m_cpSetting == GeoDataRiverSurveyImporterSettingDialog::cpMiddle) {
				shiftValue = (left + right) * 0.5;
			} else if (m_cpSetting == GeoDataRiverSurveyImporterSettingDialog::cpElevation) {
				shiftValue = minpos;
			}

			newpoint->crosssection().setLeftShift(offset);
			newpoint->shiftCenter(shiftValue);
			newpoint->InhibitInterpolatorUpdate = false;

			if (rs->m_headPoint == nullptr) {
				rs->m_headPoint = newpoint;
			} else {
				tail->addPathPoint(newpoint);
			}
			tail = newpoint;
		}
	}

	// Free all data
	RivFreeAll();

	// update interpolators
	if (ok) {
		rs->updateInterpolators();
	}
	return ok;
}

const QStringList GeoDataRiverSurveyImporter::fileDialogFilters()
{
	QStringList ret;
	ret << tr("River Survey data (*.riv)");
	return ret;
}

const QStringList GeoDataRiverSurveyImporter::acceptableExtensions()
{
	QStringList ret;
	ret << "riv";
	return ret;
}
