#ifndef POST3DWINDOWGRIDTYPEDATAITEM_H
#define POST3DWINDOWGRIDTYPEDATAITEM_H

#include "../post3dwindowdataitem.h"
#include <postbase/postwindowgridtypedataiteminterface.h>

#include <QList>
#include <QMap>

class QAction;
class SolverDefinitionGridType;
class Post3dWindowZoneDataItem;
class LookupTableContainer;

class Post3dWindowGridTypeDataItem : public Post3dWindowDataItem, public PostWindowGridTypeDataItemInterface
{
	Q_OBJECT

public:
	Post3dWindowGridTypeDataItem(SolverDefinitionGridType* type, GraphicsWindowDataItem* parent);
	~Post3dWindowGridTypeDataItem();
	const QList<Post3dWindowZoneDataItem*>& zoneDatas() const {return m_zoneDatas;}
	const std::string& name() const;
	Post3dWindowZoneDataItem* zoneData(const std::string& name) const {return m_zoneDataNameMap.value(name);}
	SolverDefinitionGridType* gridType() const {return m_gridType;}
	bool isChildCaptionAvailable(const QString& caption);
	LookupTableContainer* nodeLookupTable(const std::string& attName) const {return m_nodeLookupTables.value(attName, nullptr);}
	LookupTableContainer* particleLookupTable(const std::string& attName) const override {return m_particleLookupTables.value(attName, nullptr);}
	void setValueRange(const std::string& name);
	void setupZoneDataItems();
	void update();

private:
	void updateNodeLookupTableRanges();
	void updateParticleLookupTableRanges();

	void setupNodeScalarsToColors(const std::string& name);
	void setupParticleScalarsToColors(const std::string& name);

	void doLoadFromProjectMainFile(const QDomNode& node) override;
	void doSaveToProjectMainFile(QXmlStreamWriter& writer) override;

	SolverDefinitionGridType* m_gridType;
	QMap<std::string, LookupTableContainer*> m_nodeLookupTables;
	QMap<std::string, LookupTableContainer*> m_particleLookupTables;
	QMap<std::string, Post3dWindowZoneDataItem*> m_zoneDataNameMap;
	QList<Post3dWindowZoneDataItem*> m_zoneDatas;
	/// Action to add new condition.
	QAction* m_addConditionAction;
	bool m_isZoneDataItemsSetup;
};

#endif // POST3DWINDOWGRIDTYPEDATAITEM_H
