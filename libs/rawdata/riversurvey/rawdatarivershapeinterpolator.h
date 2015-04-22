#ifndef RAWDATARIVERSHAPEINTERPOLATOR_H
#define RAWDATARIVERSHAPEINTERPOLATOR_H

#include "rd_riversurvey_global.h"
#include <misc/linearinterpolator.h>
#include "rawdatarivercrosssection.h"
#include <QList>
#include <QVector>
#include <QMap>

class RawDataRiverPathPoint;

class AltitudeInterpolator : public InterpolatorBase {
public:
	AltitudeInterpolator(){}
	virtual ~AltitudeInterpolator(){}
	virtual void updateParameters(){}
	virtual RawDataRiverCrosssection::Altitude interpolate(double t) = 0;
};

class LinearAltitudeInterpolator : public AltitudeInterpolator {
public:
	LinearAltitudeInterpolator(RawDataRiverCrosssection::Altitude& v0, RawDataRiverCrosssection::Altitude& v1){
		m_value0 = v0;
		m_value1 = v1;
	}
	LinearAltitudeInterpolator(double t0, RawDataRiverCrosssection::Altitude& v0, double t1, RawDataRiverCrosssection::Altitude& v1);
	virtual ~LinearAltitudeInterpolator(){}
	void setValues(RawDataRiverCrosssection::Altitude& v0, RawDataRiverCrosssection::Altitude& v1){
		m_value0 = v0;
		m_value1 = v1;
	}
	void setValues(double t0, RawDataRiverCrosssection::Altitude& v0, double t1, RawDataRiverCrosssection::Altitude& v1);
	RawDataRiverCrosssection::Altitude interpolate(double t);
private:
	RawDataRiverCrosssection::Altitude m_value0;
	RawDataRiverCrosssection::Altitude m_value1;
};

class LinearLXSecInterpolator : public AltitudeInterpolator {
public:
	enum ErrorCodes {
		ec_OutOfInterpolationRange
	};
	/**
	 * @brief �R���X�g���N�^
	 */
	LinearLXSecInterpolator(RawDataRiverPathPoint* parent);
	/**
	 * @brief �f�X�g���N�^
	 */
	virtual ~LinearLXSecInterpolator(){}
	void updateParameters();
	RawDataRiverCrosssection::Altitude interpolate(double t) /* throw (ErrorCodes)*/;
protected:
	std::map<double, LinearAltitudeInterpolator*> m_interpolators;
	RawDataRiverPathPoint* m_parent;
};

class LinearRXSecInterpolator : public AltitudeInterpolator {
public:
	enum ErrorCodes {
		ec_OutOfInterpolationRange
	};
	LinearRXSecInterpolator(RawDataRiverPathPoint* parent);
	virtual ~LinearRXSecInterpolator(){}
	void updateParameters();
	RawDataRiverCrosssection::Altitude interpolate(double t) /* throw (ErrorCodes)*/;
protected:
	std::map<double, LinearAltitudeInterpolator*> m_interpolators;
	RawDataRiverPathPoint* m_parent;
};

/**
 * @ingroup RiverShape
 * @brief �e�_�Ɋւ�����ƁA���_���ꂽ�_�ɍX�V����������A���ɓ`����ׂ����Ɋւ���
 * ����ێ�����B
 */
class RiverInterpolator2D1 : public Interpolator2D1 {
public:
	RiverInterpolator2D1(RawDataRiverPathPoint* parent){
		m_parent = parent;
	}
	virtual ~RiverInterpolator2D1(){}
	virtual void updateParameters() = 0;
	virtual QVector2D interpolate(double t) = 0;
	virtual RiverInterpolator2D1* copy() = 0;
protected:
	RawDataRiverPathPoint* m_parent;
};


/**
 * @ingroup RiverShape
 * @brief �͐�Ɋւ���O�����X�v���C���\���o�[
 *
 * �O����(�O���Ȑ�)�̃X�v���C���Ȑ��𐶐����邽�߂̃\���o�[�B
 * �͐쉡�f���̊Ԃ����炩�ɂȂ����X�v���C���Ȑ��𐶐����邽�߂�
 * �g����B���̃\���o�[�Ő��������X�v���C���Ȑ��́A
 * �ȉ��̋ǖʂŎg�p�����B
 *
 * - �͐쉡�f���̕ҏW���[�h�̎��ɁA��ʏ�� �͐쉡�f���̒��S�_���m�A
 *   ���ݓ��m�A�E�ݓ��m�����ꂼ�ꌋ�񂾋Ȑ���`�悷��B
 * - �i�q�̐������ɁA�i�q��������_���m�����炩�ɂȂ��悤�ɂ���
 *   �i�q�_�� XY �����̍��W���Z�o����B
 *
 */
class RD_RIVERSURVEY_EXPORT RiverSplineSolver {
public:
	/**
	 * @brief �R���X�g���N�^
	 */
	RiverSplineSolver(){
		m_headPoint = nullptr;
	}
	/**
	 * @brief �f�X�g���N�^
	 */
	virtual ~RiverSplineSolver(){}
	/**
	 * @brief �v�Z�Ώۂ̉͐쉡�f���Q (�݂��ɂȂ����Ă���) �́A
	 * �擪�ւ̃|�C���^��n��
	 */
	void setHeadPoint(RawDataRiverPathPoint* head){
		m_headPoint = head;
	}
	/**
	 * @brief �X�v���C���Ȑ��̃p�����[�^�v�Z�����s����
	 */
	void update();
	static bool linearMode(){return m_linearMode;}
	static void setLinearMode(bool linearmode, RawDataRiverPathPoint* head, bool noundo = false);
protected:
	/**
	 * @brief RawDataRiverPathPoint ����A�\���o�[�̌v�Z�ɕK�v�ȁA�ʉߓ_��XY���W���擾����B
	 */
	virtual QVector2D getVector(RawDataRiverPathPoint* p) = 0;
	/**
	 * @brief ���[�U�����ڃA�N�Z�X���đ��삷���ԗp�I�u�W�F�N�g�ƁA�Ή�����
	 * �͐쉡�f����o�^����B
	 * @param interpolator ��ԃI�u�W�F�N�g
	 * @param p interpolator ���S�������Ԃ̗��[���Ȃ�2�̉͐쉡�f���̂����A
	 * �㗬���̉͐쉡�f���ւ̃|�C���^
	 */
	virtual void setInterpolator(Interpolator2D1* interpolator, RawDataRiverPathPoint* p) = 0;
private:
	QVector<double> m_XA;
	QVector<double> m_XB;
	QVector<double> m_XC;
	QVector<double> m_XD;
	QVector<double> m_YA;
	QVector<double> m_YB;
	QVector<double> m_YC;
	QVector<double> m_YD;
	QVector<double> m_Dist;
	/**
	 * @brief Update() �ɂ���ĎZ�o���ꂽ�p�����[�^�����ƂɁA�X�v���C���Ȑ���ɂ���
	 * �_�̍��W��Ԃ��B
	 * @param index ��Ԃ� index�B�ŏ㗬���̉͐쉡�f���ƁA����ɗׂ荇���͐쉡�f���Ƃ�
	 * �Ԃ̋�Ԃ� 0�A�ȍ~ 1, 2, 3... �ƂȂ�B
	 * @param d ��ԏ�ł̈ʒu�B 0��1�̊Ԃ̒l��n���B
	 * 0 ���ƁA�㗬���̉͐쉡�f����, 1 ���Ɖ������̉͐쉡�f����̓_�ɂȂ�B
	 */
	QVector2D interpolate(int index, double d);
	/**
	 * @brief �v�Z�Ώۂ̉͐쉡�f���Q (�݂��ɂȂ����Ă���) �́A�擪�ւ̃|�C���^
	 */
	RawDataRiverPathPoint* m_headPoint;
	/**
	 * @brief ���`��ԃ��[�h�B true �̏ꍇ�A���`��Ԃ��s����
	 */
	static bool m_linearMode;
	friend class RiverSplineInterpolator;
};

/**
 * @brief �͐쉡�f���́A���S�_���m�����ԃX�v���C���Ȑ����v�Z����\���o�[
 */
class RiverCenterLineSolver : public RiverSplineSolver {
public:
	/**
	 * @brief �R���X�g���N�^
	 */
	RiverCenterLineSolver() : RiverSplineSolver(){}
	/**
	 * @brief �f�X�g���N�^
	 */
	~RiverCenterLineSolver(){}
protected:
	QVector2D getVector(RawDataRiverPathPoint* p);
	void setInterpolator(Interpolator2D1* interpolator, RawDataRiverPathPoint* p);
};

/**
 * @brief �͐쉡�f���́A���ݓ��m�����ԃX�v���C���Ȑ����v�Z����\���o�[
 */
class RiverLeftBankSolver : public RiverSplineSolver {
public:
	/**
	 * @brief �R���X�g���N�^
	 */
	RiverLeftBankSolver() : RiverSplineSolver(){}
	/**
	 * @brief �f�X�g���N�^
	 */
	~RiverLeftBankSolver(){}
protected:
	QVector2D getVector(RawDataRiverPathPoint* p);
	void setInterpolator(Interpolator2D1* interpolator, RawDataRiverPathPoint* p);
};

/**
 * @brief �͐쉡�f���́A�E�ݓ��m�����ԃX�v���C���Ȑ����v�Z����\���o�[
 */
class RiverRightBankSolver : public RiverSplineSolver {
public:
	/**
	 * @brief �R���X�g���N�^
	 */
	RiverRightBankSolver() : RiverSplineSolver(){}
	/**
	 * @brief �f�X�g���N�^
	 */
	~RiverRightBankSolver(){}
protected:
	QVector2D getVector(RawDataRiverPathPoint* p);
	void setInterpolator(Interpolator2D1* interpolator, RawDataRiverPathPoint* p);
};

/**
 * @brief �͐쉡�f����ɔz�u���ꂽ�i�q��������_���m������
 * �X�v���C���Ȑ����v�Z����\���o�[
 */
class RD_RIVERSURVEY_EXPORT RiverGridCtrlSolver : public RiverSplineSolver {
public:
	enum Bank {
		/**
		 * @brief ���ݑ�
		 */
		bs_LeftBank,
		/**
		 * @brief �E�ݑ�
		 */
		bs_RightBank
	};
	/**
	 * @brief �R���X�g���N�^
	 */
	RiverGridCtrlSolver() : RiverSplineSolver(){
		m_BankSide = bs_LeftBank;
		m_Index = 0;
	}
	/**
	 * @brief �f�X�g���N�^
	 */
	~RiverGridCtrlSolver(){}
	void SetIndex(int index){m_Index = index;}
	int Index() const {return m_Index;}
	/**
	 * @brief �ΏۂƂ���i�q��������_�����ݑ����A�E�ݑ�����ݒ�
	 */
	void SetBankSide(Bank side){
		m_BankSide = side;
	}
	Bank BankSide() const {return m_BankSide;}
protected:
	QVector2D getVector(RawDataRiverPathPoint* p);
	void setInterpolator(Interpolator2D1* interpolator, RawDataRiverPathPoint* p);
private:
	int m_Index;
	Bank m_BankSide;
};

/**
 * @brief �͐쉡�f����ɔz�u���ꂽ�i�q��������_���m������
 * �X�v���C���Ȑ����v�Z����\���o�[
 */
class RiverBackgroundGridCtrlSolver : public RiverSplineSolver {
public:
	enum Bank {
		/**
		 * @brief ���ݑ�
		 */
		bs_LeftBank,
		/**
		 * @brief �E�ݑ�
		 */
		bs_RightBank
	};
	/**
	 * @brief �R���X�g���N�^
	 */
	RiverBackgroundGridCtrlSolver() : RiverSplineSolver(){
		m_BankSide = bs_LeftBank;
		m_Parameter = 0;
		m_Index = 0;
	}
	/**
	 * @brief �f�X�g���N�^
	 */
	~RiverBackgroundGridCtrlSolver(){}
	void SetIndex(int index){m_Index = index;}
	int Index() const {return m_Index;}
	void setParameter(double param){m_Parameter = param;}
	double parameter() const {return m_Parameter;}
	/**
	 * @brief �ΏۂƂ���i�q��������_�����ݑ����A�E�ݑ�����ݒ�
	 */
	void SetBankSide(Bank side){
		m_BankSide = side;
	}
	Bank BankSide() const {return m_BankSide;}
protected:
	QVector2D getVector(RawDataRiverPathPoint* p);
	void setInterpolator(Interpolator2D1* interpolator, RawDataRiverPathPoint* p);
private:
	double m_Parameter;
	int m_Index;
	Bank m_BankSide;
};

/**
 * @brief �͐쉡�f�����m�����񂾃X�v���C���Ȑ���p���āA��ԏ������s���I�u�W�F�N�g
 *
 * ���̃I�u�W�F�N�g���g�̓X�v���C���Ȑ��̃p�����[�^�̒l�͎������A�R���X�g���N�^
 * �œn���ꂽ RiverSplineSolver �I�u�W�F�N�g�ɖ₢���킹�邱�Ƃɂ����
 * ���W�𓾂�B
 */
class RiverSplineInterpolator : public Interpolator2D1 {
public:
	/**
	 * @brief �R���X�g���N�^
	 * @param parent ���ۂ̃X�v���C���Ȑ��̃p�����[�^�̒l��ێ�����\���o�[�ւ̃|�C���^
	 * @param index parent �̃\���o�[���ێ�����X�v���C���Ȑ��Q��ŁA�ǂ̋�Ԃɂ��Ă�
	 * �l��Ԃ����� index�B 0 �̏ꍇ�A�ŏ㗬���̉͐쉡�f���Ƃ���ɗׂ荇���͐쉡�f��
	 * �Ƃ̊Ԃ̋�ԂƂȂ�B
	 */
	RiverSplineInterpolator(RiverSplineSolver* parent, int index){
		m_parent = parent;
		m_Index = index;
	}
	/**
	 * @brief �f�X�g���N�^
	 */
	virtual ~RiverSplineInterpolator(){}
	/**
	 * @brief �͐�`�󂪕ҏW����A�X�v���C���Ȑ����Čv�Z����K�v�����������Ƃ�e�\���o�[��
	 * �ʒm
	 */
	void updateParameters(){
		m_parent->update();
	}
	/**
	 * @brief ��Ԃ��ē�����_��Ԃ�
	 */
	QVector2D interpolate(double t){
		return m_parent->interpolate(m_Index, t);
	}
	/**
	 * @brief �I�u�W�F�N�g�̃R�s�[
	 */
	Interpolator2D1* copy();
protected:
	RiverSplineSolver* m_parent;
	int m_Index;
};

/**
 * @brief �͐쉡�f�����m�����񂾃X�v���C���Ȑ���p���āA��ԏ������s���I�u�W�F�N�g�́A�R�s�[
 *
 */
class RiverSplineInterpolatorCopy : public Interpolator2D1 {
public:
	/**
	 * @brief �R���X�g���N�^
	 */
	RiverSplineInterpolatorCopy(double d, double xa, double xb, double xc, double xd, double ya, double yb, double yc, double yd){
		m_D = d;
		m_XA = xa;
		m_XB = xb;
		m_XC = xc;
		m_XD = xd;
		m_YA = ya;
		m_YB = yb;
		m_YC = yc;
		m_YD = yd;
	}
	/**
	 * @brief �f�X�g���N�^
	 */
	virtual ~RiverSplineInterpolatorCopy(){}
	void updateParameters(){}
	/**
	 * @brief ��Ԃ��ē�����_��Ԃ�
	 */
	QVector2D interpolate(double t){
		t *= m_D;
		double x =
				m_XA +
				m_XB * t +
				m_XC * t * t +
				m_XD * t * t * t;
		double y =
				m_YA +
				m_YB * t +
				m_YC * t * t +
				m_YD * t * t * t;
		return QVector2D(x, y);
	}
	/**
	 * @brief �I�u�W�F�N�g�̃R�s�[
	 */
	virtual Interpolator2D1* copy(){
		Interpolator2D1* copy = new RiverSplineInterpolatorCopy(m_D, m_XA, m_XB, m_XC, m_XD, m_YA, m_YB, m_YC, m_YD);
		return copy;
	}
private:
	double m_D;
	double m_XA;
	double m_XB;
	double m_XC;
	double m_XD;
	double m_YA;
	double m_YB;
	double m_YC;
	double m_YD;
};

/**
 * @ingroup RiverShape
 * @brief �R���X�g���N�^�Ŏw�肳�ꂽ2�_����`��Ԃ���I�u�W�F�N�g�B
 * updateParameters() ���Ă΂��ƁA�e�\���o�� Interpolator �̍Đ������˗�����B
 * RiverShapeSolver::m_LinearMode = true �̂Ƃ��̂ݎg�p�����B
 */
class RiverLinearInterpolator : public Interpolator2D1 {
public:
	RiverLinearInterpolator(RiverSplineSolver* parent, const QVector2D& v0, const QVector2D& v1) : Interpolator2D1(){
		m_Parent = parent;
		m_Interpolator.setValues(v0, v1);
	}
	virtual ~RiverLinearInterpolator(){}
	virtual void updateParameters(){
		m_Parent->update();
	}
	virtual QVector2D interpolate(double t){
		return m_Interpolator.interpolate(t);
	}
	virtual Interpolator2D1* copy(){
		return new RiverLinearInterpolator(m_Parent, m_Interpolator.interpolate(0), m_Interpolator.interpolate(1));
	}
private:
	LinearInterpolator2D1 m_Interpolator;
	RiverSplineSolver* m_Parent;
};

/**
 * @brief �e�_�Ɋւ�����ƁA���_���ꂽ�_�ɍX�V����������A���ɓ`����ׂ����Ɋւ���
 * ����ێ�����B
 */
/*
	 class RiverGridInterpolator : public RiverInterpolator2D1{
	 public:
		enum Bank{
				bk_LeftBank,
				bk_RightBank
		};
		RiverGridInterpolator(RawDataRiverPathPoint* parent, Bank bank, int index) : RiverInterpolator2D1(parent){
				m_Bank = bank;
				m_Index = index;
		}
		void SetBankIndex(Bank bank, int index){
				m_Bank = bank;
				m_Index = index;
		}
		virtual ~RiverGridInterpolator(){}
		virtual void updateParameters() = 0;
		virtual QVector2D interpolate(double t) = 0;
		virtual RiverInterpolator2D1* copy() = 0;
	 protected:
		Bank m_Bank;
		int m_Index;
	 };
	 class LinearRiverGridInterpolator : public RiverGridInterpolator{
	 public:
		LinearRiverGridInterpolator(RawDataRiverPathPoint* parent, Bank bank, int index)
				: RiverGridInterpolator(parent, bank, index){}
		~LinearRiverGridInterpolator(){}
		void updateParameters() = 0;
		QVector2D interpolate(double t) = 0;
	 //	bool InformUp(int count){return count < 2;}
	 //	bool InformDown(int count){return count < 1;}
		virtual RiverInterpolator2D1* copy() = 0;
	 private:
		LinearInterpolator2D1 m_Interpolator;
	 };
 */

#endif // __RIVERSHAPEINTERPOLATOR_H__

