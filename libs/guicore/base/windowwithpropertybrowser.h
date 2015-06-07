#ifndef WINDOWWITHPROPERTYBROWSER_H
#define WINDOWWITHPROPERTYBROWSER_H

class PropertyBrowser;

class WindowWithPropertyBrowser
{

public:
	PropertyBrowser* propertyBrowser() const {
		return m_propertyBrowser;
	}

protected:
	PropertyBrowser* m_propertyBrowser;

};

#endif // WINDOWWITHPROPERTYBROWSER_H
