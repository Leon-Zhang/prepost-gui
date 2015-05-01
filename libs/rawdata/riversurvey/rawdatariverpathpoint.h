#ifndef RAWDATARIVERPATHPOINT_H
#define RAWDATARIVERPATHPOINT_H

#include "rd_riversurvey_global.h"
#include "rawdatarivercrosssection.h"
#include <misc/versionnumber.h>
#include <QVector2D>
#include <QVector>
#include <set>

#include <vtkStructuredGrid.h>
#include <vtkSmartPointer.h>

class Grid;
class Interpolator2D1;
class LinearLXSecInterpolator;
class LinearRXSecInterpolator;
class Structured2DGrid;
class RawDataRiverSurveyBackgroundGridCreateThread;
//class RiverGridInterpolator;
//class BStream;
namespace iRICLib
{
	class RiverPathPoint;
}

struct CtrlPointSelectionInfo;

class RD_RIVERSURVEY_EXPORT RawDataRiverPathPoint
{
public:
	enum Bank {
		bk_LeftBank,
		bk_RightBank
	};
	enum ErrorCodes {
		ec_PreviousPointDontExist,  ///< @brief �O�̓_�����݂��Ȃ����߂ɑ��삪���s����
		ec_OutOfCrosssectionRange,  ///< @brief �f�ʂ̑��݂���͈͊O�������Ƃ��Ďw�肳�ꂽ
		ec_OutOfCtrlPointRange,     ///< @brief �i�q��������_�̑��݂���͈͊O��index�������Ƃ��Ďw�肳�ꂽ
		ec_OutOfCtrlZoneRange,      ///< @brief �i�q��������_�̑��݂���͈͊O��index�������Ƃ��Ďw�肳�ꂽ
		ec_InvalidGridSize          ///< @brief �i�q�̃T�C�Y���s�K��
	};
	enum CtrlPointPosition {
		pposCenterToLeft,
		pposCenterToRight,
		pposCenterLine,
		pposLeftBank,
		pposRightBank
	};
	enum CtrlZonePosition {
		zposCenterToLeft,
		zposCenterToRight,
		zposCenterLine,
		zposLeftBank,
		zposRightBank
	};
	struct CtrlPointsAddMethod {
		enum {
			am_Uniform,          ///< Uniform
			am_EqRatio_Ratio     ///< Equal ratio
		} method;
		unsigned int number; ///< Number of points to add
		double param; ///< parameter
	};
	/// Constructor
	RawDataRiverPathPoint() {
		initializeInnerValues();
		initializeInterpolators();
	}
	/// Constructor
	RawDataRiverPathPoint(double x, double y) {
		initializeInnerValues();
		initializeInterpolators();
		m_position = QVector2D(x, y);
	}
	/// Constructor
	RawDataRiverPathPoint(const QString& name, double x, double y) {
		initializeInnerValues();
		initializeInterpolators();
		m_name = name;
		m_position = QVector2D(x, y);
	}
	/// River center position
	const QVector2D& position() const {return m_position;}
	void load(QDataStream& s, const VersionNumber& number);
	void save(QDataStream& s);
	/// Shift the river center position
	void shiftCenter(double shiftWidth);
	/// Set new river center position
	void setPosition(const QVector2D& v);
	/**
	 * @brief �͐쒆�S�_�̍��W��ύX����B�������A�f�ʏ�̕ʂ̓_�ɁA���W��ύX����B
	 * @param offset �f�ʕ����̈ړ��ʁB�����ƉE�ݕ���
	 */
	void movePosition(double offset);
	/**
	 * @brief �ŏ��̓_���ǂ�����Ԃ��B
	 *
	 * �ŏ��̓_���ƁA�f�ʂ̏���ێ����邱�Ƃ��ł��Ȃ��B
	 */
	bool firstPoint() const {return m_firstPoint;}
	QVector2D crosssectionPosition(double x);
	/// Add new river path point before this point.
	void insertPathPoint(RawDataRiverPathPoint* p);
	/// Add new river path point after this point.
	void addPathPoint(RawDataRiverPathPoint* p);
	/// Remove this point.
	void remove();
	void setCrosssectionDirection(const QVector2D& v);
	/// The pointer to the previous river path point.
	RawDataRiverPathPoint* previousPoint() {return m_previousPoint;}
	/// The pointer to the next river path point.
	RawDataRiverPathPoint* nextPoint() {return m_nextPoint;}
	/// Name
	const QString& name() const {return m_name;}
	/// Set new name
	void setName(const QString& newname);
	RawDataRiverCrosssection& crosssection() {return m_crosssection;}
	/**
	 * @brief �f�ʂ́A���݂���E�݂Ɍ���������
	 */
	const QVector2D& crosssectionDirection() const {return m_crosssectionDirection;}
	const QVector2D& crosssectionDirectionL() const {return m_crosssectionDirectionL;}
	const QVector2D& crosssectionDirectionR() const {return m_crosssectionDirectionR;}
	/**
	 * @brief �f�ʂ́A���݂���E�݂Ɍ�����������ݒ肷��B
	 */
	void setCrosssectionAngle(double angle) /* throw (ErrorCodes)*/;
	/**
	 * @brief �f�ʂ́A���݂���E�݂Ɍ�����������ݒ肷��B
	 */
	void setCrosssectiondirection(const QVector2D& v);
	/**
	 * @brief �f�ʍ��݂̉����x�N�g���̌�����ύX
	 */
	void setCrosssectionDirectionL(const QVector2D& direction);
	/**
	 * @brief �f�ʍ��݂̉����x�N�g���̌�����ύX
	 */
	void setCrosssectionDirectionR(const QVector2D& direction);
	QVector2D GridCtrlPosition(Bank bank, unsigned int index1, double param);
	QVector2D backgroundGridCtrlPosition(Bank bank, unsigned int index1, double param);
	void addExtentionPointLeft(const QVector2D& pos);
	void addExtentionPointRight(const QVector2D& pos);
	void moveExtentionPointLeft(const QVector2D& pos);
	void moveExtentionPointRight(const QVector2D& pos);
	void removeExtentionPointLeft();
	void removeExtentionPointRight();
	void updateAllXSecInterpolators();
	/**
	 * @brief ���̓_�ɍX�V�����������Ƃ��㗬�A�������̓_�ɒʒm����B
	 */
	void updateXSecInterpolators();
	void updateRiverShapeInterpolators();
	void UpdateGridInterpolators();

	unsigned int gridCounts(RawDataRiverPathPoint* end) {
		if (this == end) {
			return 1;
		} else {
			if (! m_gridSkip) {
				return 1 + static_cast<unsigned int>(CenterLineCtrlPoints.size()) + m_nextPoint->gridCounts(end);
			} else {
				return m_nextPoint->gridCounts(end);
			}
		}
	}
	void createGrid(Structured2DGrid* grid, unsigned int initcount, bool elevmapping, bool last = false);

	bool IsSelected;
	/**
	 * @brief UpdareRiverShapeInterpolators() �𖳌��ɂ���
	 */
	bool InhibitInterpolatorUpdate;
	/**
	 * @brief ���̓_���܂݁A���̓_��艺�����̒f�ʂ̂����A�I������Ă���
	 * �_�̐���Ԃ��B
	 */
	int selectedPoints() {
		int selectedpoints;
		if (IsSelected) {
			selectedpoints = 1;
		} else {
			selectedpoints = 0;
		}
		if (m_nextPoint != 0) {
			return selectedpoints + m_nextPoint->selectedPoints();
		} else {
			return selectedpoints;
		}
	}
	/**
	 * @brief �̈�I��(������)
	 * @param point0 ��_�|�C���g
	 * @param v0 �x�N�g��1
	 * @param v1 �x�N�g��2
	 */
	void selectRegion(const QVector2D& point0, const QVector2D& v0, const QVector2D& v1);
	void XORSelectRegion(const QVector2D& point0, const QVector2D& v0, const QVector2D& v1);
	void SelectCtrlPointsRegion(const QVector2D& point0, const QVector2D& v0, const QVector2D& v1, std::list<CtrlPointSelectionInfo>& info);

	/// Clear selection.
	void clearSelection() {
		IsSelected = false;
		if (m_nextPoint != 0) {
			m_nextPoint->clearSelection();
		}
	}
	void addCtrlPoints(CtrlZonePosition position, unsigned int index, CtrlPointsAddMethod method);
	void reposCtrlPoints(CtrlPointPosition position, int minindex, int maxindex, CtrlPointsAddMethod method);
	void removeCtrlPoints(CtrlZonePosition position, std::set<int> indices);

	QVector2D CtrlPointPosition2D(CtrlPointPosition pos, unsigned int index);
	QVector2D CtrlPointPosition2D(CtrlPointPosition pos, double d);
	void clearWaterSurfaceElevation();
	void setWaterSurfaceElevation(double value);
	bool waterSurfaceElevationSpecified() const;
	double waterSurfaceElevationValue() const;

	QList<QVector2D> CtrlZonePoints(CtrlZonePosition position, unsigned int index, int num);
	bool gridSkip() const {return m_gridSkip;}
	void setGridSkip(bool skip) {
		if (m_nextPoint == 0) {return;}
		if (firstPoint()) {return;}
		if (m_previousPoint->firstPoint()) {return;}
		m_gridSkip = skip;
	}
	/**
	 * @brief �͐쒆�S����`�悷�邽�߂́A��ԃI�u�W�F�N�g
	 */
	Interpolator2D1* riverCenter() {return m_riverCenter;}
	void setRiverCenter(Interpolator2D1* interpolator) {m_riverCenter = interpolator;}
	/**
	 * @brief �͐썶�݂�`�悷�邽�߂́A��ԃI�u�W�F�N�g
	 */
	Interpolator2D1* leftBank() {return m_leftBank;}
	void setLeftBank(Interpolator2D1* interpolator) {m_leftBank = interpolator;}
	/**
	 * @brief �͐�E�݂�`�悷�邽�߂́A��ԃI�u�W�F�N�g
	 */
	Interpolator2D1* rightBank() {return m_rightBank;}
	void setRightBank(Interpolator2D1* interpolator) {m_rightBank = interpolator;}
	LinearLXSecInterpolator* lXSec() {return m_lXSec;}
	LinearRXSecInterpolator* rXSec() {return m_rXSec;}
	QVector<Interpolator2D1*>& LGridLines() {return m_LGridLines;}
	QVector<Interpolator2D1*>& RGridLines() {return m_RGridLines;}
	QVector<Interpolator2D1*>& backgroundLGridLines() {return m_backgroundLGridLines;}
	QVector<Interpolator2D1*>& backgroundRGridLines() {return m_backgroundRGridLines;}
	Interpolator2D1* LGridLine(unsigned int index) {return m_LGridLines[index];}
	Interpolator2D1* RGridLine(unsigned int index) {return m_RGridLines[index];}
	Interpolator2D1* backgroundLGridLine(unsigned int index) {return m_backgroundLGridLines[index];}
	Interpolator2D1* backgroundRGridLine(unsigned int index) {return m_backgroundRGridLines[index];}
	//	void UpdateInterpolators();
	/// Division points between river center and left bank
	QVector<double> CenterToLeftCtrlPoints;
	/// Division points between river center and right bank
	QVector<double> CenterToRightCtrlPoints;
	/// Division points on River center line
	QVector<double> CenterLineCtrlPoints;
	/// Division points on left bank
	QVector<double> LeftBankCtrlPoints;
	/// Division points on right bank
	QVector<double> RightBankCtrlPoints;
	QVector<double>& CtrlPoints(CtrlZonePosition position) {
		switch (position) {
		case zposCenterToLeft:
			return CenterToLeftCtrlPoints;
			break;
		case zposCenterToRight:
			return CenterToRightCtrlPoints;
			break;
		case zposCenterLine:
			return CenterLineCtrlPoints;
			break;
		case zposLeftBank:
			return LeftBankCtrlPoints;
			break;
		case zposRightBank:
			return RightBankCtrlPoints;
			break;
			// Never used. this is written to avoid warning in compiling.
		default:
			return RightBankCtrlPoints;
		}
	}
	QVector<double>& CtrlPoints(CtrlPointPosition position) {
		switch (position) {
		case pposCenterToLeft:
			return CenterToLeftCtrlPoints;
			break;
		case pposCenterToRight:
			return CenterToRightCtrlPoints;
			break;
		case pposCenterLine:
			return CenterLineCtrlPoints;
			break;
		case pposLeftBank:
			return LeftBankCtrlPoints;
			break;
		case pposRightBank:
			return RightBankCtrlPoints;
			break;
			// Never used. this is written to avoid warning in compiling.
		default:
			return RightBankCtrlPoints;
		}
	}
	void centerToBanksRegion(QVector2D& mins, QVector2D& maxs);
	void thisToNextRegion(QVector2D& mins, QVector2D& maxs);
	void UpdateCtrlSections();
	vtkStructuredGrid* areaGrid() {return m_areaGrid;}
	QList<int> getPointsToInactivateUsingWaterElevation();
	void loadFromiRICLibObject(const iRICLib::RiverPathPoint* p);
	void saveToiRICLibObject(iRICLib::RiverPathPoint* p);
protected:
	void initializeInnerValues();
	// ��Ԃ��s���I�u�W�F�N�g������������B
	virtual void initializeInterpolators();
private:
	double GridCtrlParameter(Bank bank, int index1, int index2);
	QVector2D myCtrlPointPosition2D(Interpolator2D1 * (RawDataRiverPathPoint::* f)(), double d);
	QVector2D myCtrlPointPosition2D(Interpolator2D1 * (RawDataRiverPathPoint::* f)(unsigned int), unsigned int index, double d);
	QVector2D myBgCtrlPointPosition2D(Interpolator2D1 * (RawDataRiverPathPoint::* f)(unsigned int), unsigned int index, double d);
	double myHeight(RawDataRiverPathPoint::Bank bank, double t0, double t1, double d);

	QString m_name;
	RawDataRiverPathPoint* m_previousPoint;
	RawDataRiverPathPoint* m_nextPoint;
	QVector2D m_position;
	bool m_waterSurfaceElevationSpecified;
	double m_waterSurfaceElevationValue;

	QVector2D m_crosssectionDirection;

	QVector2D m_crosssectionDirectionL;
	QVector2D m_crosssectionDirectionR;

	bool m_firstPoint;

	bool m_gridSkip;
	QVector<double> m_CtrlSections;
	vtkSmartPointer<vtkStructuredGrid> m_areaGrid;

	RawDataRiverCrosssection m_crosssection;

	Interpolator2D1* m_riverCenter;

	Interpolator2D1* m_leftBank;

	Interpolator2D1* m_rightBank;

	LinearLXSecInterpolator* m_lXSec;

	LinearRXSecInterpolator* m_rXSec;

	QVector<Interpolator2D1*> m_LGridLines;

	QVector<Interpolator2D1*> m_RGridLines;

	QVector<Interpolator2D1*> m_backgroundLGridLines;

	QVector<Interpolator2D1*> m_backgroundRGridLines;
public:
	friend class RawDataRiverSurveyBackgroundGridCreateThread;
};

/**
 * @brief �i�q��������_�̑I����Ԃ̏���ێ�
 */
struct CtrlPointSelectionInfo {
	/// @brief �i�q��������_�̐e �͐쉡�f��
	RawDataRiverPathPoint* Point;
	/// @brief �i�q��������_�̂̂��Ă�����̈ʒu
	RawDataRiverPathPoint::CtrlPointPosition Position;
	/// @brief ����_�� Index
	unsigned int Index;
	bool operator==(CtrlPointSelectionInfo& info) const {
		return (
						 Point == info.Point &&
						 Position == info.Position &&
						 Index == info.Index
					 );
	}
};

/// Selection information of grid control zone
struct CtrlZoneSelectionInfo {
	/// River path point
	RawDataRiverPathPoint* Point;
	/// @brief �i�q��������_�ǉ��ʒu�̂̂��Ă�����̈ʒu
	RawDataRiverPathPoint::CtrlZonePosition Position;
	/// Index
	int Index;
};


#endif // RAWDATARIVERPATHPOINT_H
