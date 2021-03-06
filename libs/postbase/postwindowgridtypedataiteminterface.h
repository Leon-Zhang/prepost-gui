#ifndef POSTWINDOWGRIDTYPEDATAITEMINTERFACE_H
#define POSTWINDOWGRIDTYPEDATAITEMINTERFACE_H

#include <string>

class SolverDefinitionGridType;
class LookupTableContainer;

class PostWindowGridTypeDataItemInterface
{
public:
	virtual ~PostWindowGridTypeDataItemInterface() {}
	virtual SolverDefinitionGridType* gridType() const = 0;
	virtual LookupTableContainer* nodeLookupTable(const std::string& attName) const = 0;
	virtual LookupTableContainer* particleLookupTable(const std::string& attName) const = 0;
};

#endif // POSTWINDOWGRIDTYPEDATAITEMINTERFACE_H

