#ifndef MEASUREDDATA_API_H
#define MEASUREDDATA_API_H

#include <QtCore/QtGlobal>

#if defined(MEASUREDDATA_LIBRARY)
#  define MEASUREDDATA_API Q_DECL_EXPORT
#else
#  define MEASUREDDATA_API Q_DECL_IMPORT
#endif

#endif // MEASUREDDATA_API_H