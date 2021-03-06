#ifndef GRIDATTRIBUTEVARIATIONEDITWIDGETT_H
#define GRIDATTRIBUTEVARIATIONEDITWIDGETT_H

#include "gridattributevariationeditwidget.h"
#include "../../../solverdef/solverdefinitiongridattributet.h"

/// Widget to Edit Grid related condition value
template <class V>
class GridAttributeVariationEditWidgetT : public GridAttributeVariationEditWidget
{
public:
	GridAttributeVariationEditWidgetT(QWidget* parent, SolverDefinitionGridAttributeT<V>* cond);

	V value();
	void setValue(V value);

	QVariant variantValue() override;
	void setVariantValue(const QVariant& v);

	void applyVariation(GridAttributeContainer* container, QVector<vtkIdType>& indices, vtkDataSetAttributes* atts, PreProcessorGridDataItemInterface* dItem) override;

protected:
	V m_value {0};
};

#include "private/gridattributevariationeditwidgett_detail.h"

#endif // GRIDATTRIBUTEEDITWIDGETT_H
