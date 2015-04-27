#include "../main/iricmainwindow.h"
#include "continuoussnapshotfilepropertypage.h"
#include "continuoussnapshotgoogleearthsettingpage.h"
#include "continuoussnapshotmoviepropertypage.h"
#include "continuoussnapshottimestepsettingpage.h"
#include "continuoussnapshotwindowselectionpage.h"
#include "continuoussnapshotwizard.h"

#include <guicore/base/snapshotenabledwindow.h>

#include <QDir>
#include <QLabel>
#include <QListWidget>
#include <QMdiSubWindow>
#include <QPushButton>
#include <QVBoxLayout>

ContinuousSnapshotWizard::ContinuousSnapshotWizard(QWidget* parent)
	: QWizard(parent)
{
	addPage(new ContinuousSnapshotIntroductionPage(this));
	addPage(new ContinuousSnapshotWindowSelectionPage(this));
	addPage(new ContinuousSnapshotFilePropertyPage(this));
	addPage(new ContinuousSnapshotMoviePropertyPage(this));
	addPage(new ContinuousSnapshotTimestepSettingPage(this));
	addPage(new ContinuousSnapshotGoogleEarthSettingPage(this));
	addPage(new ContinuousSnapshotConfirmPage(this));
	setWindowTitle(tr("Continuous Snapshot / Movie / Google Earth Export Wizard"));
	resize(600, 500);

	m_mainWindow = dynamic_cast<iRICMainWindow*>(parent);
}

void ContinuousSnapshotWizard::accept()
{
	QWizard::accept();
}

ContinuousSnapshotIntroductionPage::ContinuousSnapshotIntroductionPage(QWidget *parent)
	: QWizardPage(parent)
{
	setTitle(tr("Introduction"));
	m_label = new QLabel(tr(
			"This wizard will generate continuous snapshots."
			"You need to specify some informations for that."
				));
	m_label->setWordWrap(true);
	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(m_label);
	setLayout(layout);
}

ContinuousSnapshotConfirmPage::ContinuousSnapshotConfirmPage(QWidget *parent)
	: QWizardPage(parent)
{
	setTitle(tr("Confirm the result"));
	setSubTitle(tr(
			"The files in the list below will be created or updated. "
				));

	m_fileList = new QListWidget(this);
	m_fileList->setEditTriggers(QAbstractItemView::NoEditTriggers);
	m_fileList->setSelectionMode(QAbstractItemView::NoSelection);

	QVBoxLayout* layout = new QVBoxLayout();
	layout->addWidget(m_fileList);
	setLayout(layout);

	m_wizard = dynamic_cast<ContinuousSnapshotWizard*>(parent);
}

void ContinuousSnapshotConfirmPage::initializePage()
{
	m_fileList->clear();
	m_wizard->clearFileList();

	int len = m_wizard->suffixLength();
	int step = m_wizard->start();
	int rate = m_wizard->samplingRate();
	int index = 1;
	while (step <= m_wizard->stop()){
		for (QString prefix : m_wizard->prefixList()){
			prefix.append(QString("%1").arg(index, len, 10, QChar('0')));
			prefix.append(m_wizard->extension());
			QString absolute = QDir(m_wizard->fileIODirectory()).absoluteFilePath(prefix);
			m_fileList->addItem(QDir::toNativeSeparators(absolute));
			m_wizard->addFileList(absolute);
		}
		++index;
		step += rate;
	}
	if (m_wizard->outputMovie()){
		for (const QString& prefix : m_wizard->prefixList()){
			QString movieFilename = QString("%1.wmv").arg(prefix);
			QString absMovieFilename = QDir(m_wizard->fileIODirectory()).absoluteFilePath(movieFilename);
			m_fileList->addItem(QDir::toNativeSeparators(absMovieFilename));
		}
	}
	m_fileList->sortItems();

	// kml file
	if (m_wizard->googleEarth()){
		QString kml = QDir(m_wizard->fileIODirectory()).absoluteFilePath(m_wizard->kmlFilename());
		m_fileList->addItem(QDir::toNativeSeparators(kml));
	}
}
