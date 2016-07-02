#include "geodatapolygon_modifyabstractpolygoncommand.h"
#include "geodatapolygon_removevertexcommand.h"
#include "../geodatapolygonabstractpolygon.h"

#include <guicore/pre/base/preprocessorgraphicsviewinterface.h>

GeoDataPolygon::RemoveVertexCommand::RemoveVertexCommand(vtkIdType vertexId, GeoDataPolygon* pol) :
	GeoDataPolygon::ModifyAbstractPolygonCommand(pol->m_selectedPolygon, pol, GeoDataPolygon::tr("Remove Polygon Vertex"))
{
	QPolygonF newPolygon = pol->m_selectedPolygon->polygon();
	newPolygon.removeAt(vertexId);
	if (vertexId == 0) {
		newPolygon.removeLast();
		newPolygon.push_back(newPolygon.at(0));
	}
}
