#ifndef CONTINUOUSSNAPSHOTWIZARD_H
#define CONTINUOUSSNAPSHOTWIZARD_H

#include <QWizard>
#include <QList>

class QLabel;
class QListWidget;
class QMdiSubWindow;

class iRICMainWindow;
class ContinuousSnapshotWizard;
class BackgroundImageInfo;

class ContinuousSnapshotIntroductionPage : public QWizardPage
{
	Q_OBJECT

public:
	ContinuousSnapshotIntroductionPage(QWidget* parent = nullptr);

private:
	QLabel* m_label;
};

class ContinuousSnapshotConfirmPage : public QWizardPage
{
	Q_OBJECT

public:
	ContinuousSnapshotConfirmPage(QWidget* parent = nullptr);
	void initializePage() override;

private:
	ContinuousSnapshotWizard* m_wizard;
	QListWidget* m_fileList;
};

class ContinuousSnapshotWizard : public QWizard
{
	Q_OBJECT

public:
	enum Output {Onefile, Respectively};
	enum Layout {Asis, Horizontally, Vertically};

	ContinuousSnapshotWizard(QWidget* parent);
	void accept() override;

	void setOutput(Output o) {m_output = o;}
	void setLayout(Layout l) {m_layout = l;}
	void setSnapshotSize(QSize s) {m_snapshotSize = s;}
	void setBeginPosition(QPoint p) {m_beginPosition = p;}
	void setTransparent(bool b) {m_transparent = b;}
	void setFileIODirectory(const QString& s) {m_fileIODirectory = s;}
	void setExtension(const QString& ex) {m_extension = ex;}
	void setSuffixLength(int s) {m_suffixLength = s;}
	void setOutputMovie(bool output) {m_movie = output;}
	void setMovieLengthMode(int mode) {m_movieLengthMode = mode;}
	void setFramesPerSecond(int fps) {m_framesPerSecond = fps;}
	void setMovieProfile(int profid) {m_movieProfile = profid;}
	void setMovieLength(int length) {m_movieLength = length;}
	void setTimesteps(QList<double> steps) {m_timeSteps = steps;}
	void setStart(int s) {m_start = s;}
	void setStop(int s) {m_stop = s;}
	void setSamplingRate(int r) {m_samplingRate = r;}
	void setGoogleEarth(bool on) {m_googleEarth = on;}
	void setLeftLatitude(double deg) {m_leftLatitude = deg;}
	void setLeftLongitude(double deg) {m_leftLongitude = deg;}
	void setRightLatitude(double deg) {m_rightLatitude = deg;}
	void setRightLongitude(double deg) {m_rightLongitude = deg;}
	void setTargetWindow(int i) {m_targetWindow = i;}
	void setTargetBackground(int i) {m_targetBackground = i;}
	void setKMLFilename(const QString& name) {m_kmlFilename = name;}
	void setBackgroundList(const QList<BackgroundImageInfo*>& list) {m_backgroundList = list;}
	void setAngle(double a) {m_angle = a;}
	void setNorth(double n) {m_north = n;}
	void setSouth(double s) {m_south = s;}
	void setEast(double e) {m_east = e;}
	void setWest(double w) {m_west = w;}

	Output output() {return m_output;}
	Layout layout() {return m_layout;}
	QSize snapshotSize() {return m_snapshotSize;}
	QPoint beginPosition() {return m_beginPosition;}
	bool transparent() {return m_transparent;}
	QString fileIODirectory() {return m_fileIODirectory;}
	QString extension() {return m_extension;}
	int suffixLength() {return m_suffixLength;}
	bool outputMovie() const {return m_movie;}
	int movieLengthMode() const {return m_movieLengthMode;}
	int framesPerSecond() const {return m_framesPerSecond;}
	int movieLength() const {return m_movieLength;}
	int movieProfile() const {return m_movieProfile;}
	const QList<double>& timeSteps() {return m_timeSteps;}
	int start() {return m_start;}
	int stop() {return m_stop;}
	int samplingRate() {return m_samplingRate;}
	bool googleEarth() {return m_googleEarth;}
	double leftLatitude() {return m_leftLatitude;}
	double leftLongitude() {return m_leftLongitude;}
	double rightLatitude() {return m_rightLatitude;}
	double rightLongitude() {return m_rightLongitude;}
	int targetWindow() {return m_targetWindow;}
	int targetBackground() {return m_targetBackground;}
	QString kmlFilename() {return m_kmlFilename;}
	const QList<BackgroundImageInfo*>& backgroundList() {return m_backgroundList;}
	double angle() {return m_angle;}
	double north() {return m_north;}
	double south() {return m_south;}
	double east() {return m_east;}
	double west() {return m_west;}

	void clearWindowList() {m_windowList.clear();}
	void addWindowList(QMdiSubWindow* sub) {m_windowList.append(sub);}
	const QList<QMdiSubWindow*>& windowList() {return m_windowList;}
	void clearPrefixList() {m_prefixList.clear();}
	void addPrefixList(const QString& pre) {m_prefixList.append(pre);}
	const QStringList& prefixList() {return m_prefixList;}
	void clearFileList() {m_fileList.clear();}
	void addFileList(const QString& name) {m_fileList.append(name);}
	const QStringList& fileList() {return m_fileList;}

signals:
	void requestChangeCurrentStepIndex(unsigned int);

private:
	iRICMainWindow* m_mainWindow;

	// window selection
	QList<QMdiSubWindow*> m_windowList;
	Output m_output;
	Layout m_layout;
	QSize m_snapshotSize;
	QPoint m_beginPosition;     // snapshot top-left position
	bool m_transparent;

	// file properties
	QString m_fileIODirectory;
	QStringList m_prefixList;
	QString m_extension;
	int m_suffixLength;

	// movie properties
	bool m_movie;
	int m_movieLengthMode;
	int m_framesPerSecond;
	int m_movieLength;
	int m_movieProfile;

	// timestep setting
	QList<double> m_timeSteps;
	int m_start;
	int m_stop;
	int m_samplingRate;

	// google earth
	bool m_googleEarth;
	double m_leftLatitude;
	double m_leftLongitude;
	double m_rightLatitude;
	double m_rightLongitude;
	int m_targetWindow;
	int m_targetBackground;
	QString m_kmlFilename;
	QList<BackgroundImageInfo*> m_backgroundList;
	double m_angle;
	double m_north;
	double m_south;
	double m_east;
	double m_west;

	// confirm the result
	QStringList m_fileList;
};

#endif // CONTINUOUSSNAPSHOTWIZARD_H
