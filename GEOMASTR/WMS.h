//  WMS.h
//  CCodeLibrary

#ifndef WMS_h
#define WMS_h
#include "CCodeTypes.h"

BOOL wmsRequest(const char *filepath, const char *layer, dbounds bounds, unsigned short width, unsigned short height);

#endif /* WMS_h */
