#ifndef PREPROCESSORGEODATADATAITEMINTERFACE_H
#define PREPROCESSORGEODATADATAITEMINTERFACE_H

#include "preprocessordataitem.h"
class GeoData;
class QStandardItem;

class PreProcessorRawdataDataItemInterface : public PreProcessorDataItem
{

public:
	PreProcessorRawdataDataItemInterface(const QString& itemlabel, const QIcon& icon, GraphicsWindowDataItem* parent)
		: PreProcessorDataItem(itemlabel, icon, parent)
	{}
	virtual GeoData* geoData() = 0;
	virtual void informValueRangeChange() = 0;
	virtual void informDataChange() = 0;

	friend class GeoData;
};

#endif // PREPROCESSORGEODATADATAITEMINTERFACE_H