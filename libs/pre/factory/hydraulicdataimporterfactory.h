#ifndef HYDRAULICDATAIMPORTERFACTORY_H
#define HYDRAULICDATAIMPORTERFACTORY_H

#include <QObject>
#include <QList>

class HydraulicDataImporter;

class HydraulicDataImporterFactory : public QObject
{

private:
	/// Constructor
	/**
	 * this function is made to be private (Singleton pattern)
	 */
	HydraulicDataImporterFactory();

public:
	static HydraulicDataImporterFactory& instance();
	const QList<HydraulicDataImporter*> importers() const;

private:
	QList<HydraulicDataImporter*> m_importers;
	static HydraulicDataImporterFactory* m_instance;
};

#endif // GEODATAFACTORY_H
