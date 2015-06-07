#ifndef RAWDATAPOINTMAPREALIMPORTER_H
#define RAWDATAPOINTMAPREALIMPORTER_H

#include <guicore/pre/rawdata/rawdataimporter.h>
#include <QMap>

class GDALDataset;
class GDALRasterBand;

class RawDataPointmapRealImporter : public RawDataImporter
{
	Q_OBJECT
private:
	enum filterString {
		dotTopo,      ///< Topography
		dotDat,       ///< RIC-Nays DEM
		dotAdf,       ///< USGS NED(*.adf)
		dotStl        ///< STL (*.stl)
	};
public:
	/// Constructor
	RawDataPointmapRealImporter(RawDataCreator* creator);
	bool importData(RawData* data, int index, QWidget* w) override;
	const QStringList fileDialogFilters() override;
	const QStringList acceptableExtensions() override;
	GDALDataset* poDataset;
	GDALRasterBand* poBand;

protected:
	bool doInit(const QString& filename, const QString& selectedFilter, int* count, SolverDefinitionGridAttribute* condition, PreProcessorRawDataGroupDataItemInterface* item, QWidget* w) override;

private:
	int m_filterValue;
};

#endif // RAWDATAPOINTMAPREALIMPORTER_H
