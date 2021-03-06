#include "inputconditioncontainerstring.h"
#include "private/inputconditioncontainerstring_impl.h"

#include <misc/stringtool.h>

#include <QDomNode>
#include <QTextStream>

#include <yaml-cpp/yaml.h>

#include <iriclib.h>

InputConditionContainerString::InputConditionContainerString() :
	InputConditionContainer(),
	impl {new Impl {}}
{}

InputConditionContainerString::InputConditionContainerString(const std::string& n, const QString& c, const QDomNode& defNode) :
	InputConditionContainer(n, c),
	impl {new Impl {}}
{
	setup(defNode);
}

InputConditionContainerString::InputConditionContainerString(const InputConditionContainerString& i) :
	InputConditionContainer(i),
	impl {new Impl {}}
{
	copyValues(i);
}

InputConditionContainerString::~InputConditionContainerString()
{
	delete impl;
}

InputConditionContainerString& InputConditionContainerString::operator=(const InputConditionContainerString& i)
{
	copyValues(i);
	return *this;
}

const QString& InputConditionContainerString::value() const
{
	return impl->m_value;
}

void InputConditionContainerString::setValue(const QString& v)
{
	if (impl->m_value != v) {
		impl->m_value = v;
		emit valueChanged(impl->m_value);
		emit valueChanged();
	}
}

const QString& InputConditionContainerString::defaultValue() const
{
	return impl->m_default;
}

void InputConditionContainerString::setDefaultValue(const QString& v)
{
	impl->m_default = v;
}

int InputConditionContainerString::load()
{
	char* buffer;
	int ret;
	int length;

	if (isBoundaryCondition()){
		ret = cg_iRIC_Read_BC_StringLen(const_cast<char*>(bcName().c_str()), bcIndex(), const_cast<char*>(name().c_str()), &length);
	} else if (isComplexCondition()){
		ret = cg_iRIC_Read_Complex_StringLen(const_cast<char*>(complexName().c_str()), complexIndex(), const_cast<char*>(name().c_str()), &length);
	} else {
		ret = cg_iRIC_Read_StringLen(const_cast<char*>(name().c_str()), &length);
	}

	if (ret != 0){
		clear();
		return ret;
	}

	buffer = new char[length + 1];

	if (isBoundaryCondition()){
		ret = cg_iRIC_Read_BC_String(const_cast<char*>(bcName().c_str()), bcIndex(), const_cast<char*>(name().c_str()), buffer);
	} else if (isComplexCondition()){
		ret = cg_iRIC_Read_Complex_String(const_cast<char*>(complexName().c_str()), complexIndex(), const_cast<char*>(name().c_str()), buffer);
	} else {
		ret = cg_iRIC_Read_String(const_cast<char*>(name().c_str()), buffer);
	}
	if (ret != 0){
		clear();
	} else {
		impl->m_value = buffer;
		emit valueChanged(impl->m_value);
		emit valueChanged();
	}
	delete buffer;

	return ret;
}

int InputConditionContainerString::save()
{
	std::string value = impl->m_value.toUtf8().constData();
	if (isBoundaryCondition()) {
		return cg_iRIC_Write_BC_String(toC(bcName()), bcIndex(), toC(name()), toC(value));
	} else if (isComplexCondition()) {
		return cg_iRIC_Write_Complex_String(toC(complexName()), complexIndex(), toC(name()), toC(value));
	} else {
		return cg_iRIC_Write_String(toC(name()), toC(value));
	}
}

void InputConditionContainerString::clear()
{
	impl->m_value = impl->m_default;
	emit valueChanged(impl->m_value);
}

QVariant InputConditionContainerString::variantValue() const
{
	return QVariant(impl->m_value);
}

void InputConditionContainerString::importFromYaml(const YAML::Node& doc, const QDir&)
{
	if (doc[name()]) {
		impl->m_value = doc[name()].as<std::string>().c_str();
		emit valueChanged(impl->m_value);
		emit valueChanged();
	}
}

void InputConditionContainerString::exportToYaml(QTextStream* stream, const QDir&)
{
	*stream << name().c_str() << ": " << impl->m_value << "\t#[string] " << caption() << "\r\n";
}

void InputConditionContainerString::setup(const QDomNode& defNode)
{
	QDomElement e = defNode.toElement();
	impl->m_default = e.attribute("default", "");
	impl->m_value = impl->m_default;
}

void InputConditionContainerString::copyValues(const InputConditionContainerString& i)
{
	InputConditionContainer::copyValues(i);
	setValue(i.value());
	setDefaultValue(i.defaultValue());
}
