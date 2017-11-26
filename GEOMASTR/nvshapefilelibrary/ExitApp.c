//  ExitApp.c
//  NVShapefileLibrary

#include "ExitApp.h"
#include "Assert.h"
//#include <execinfo.h>

void exitApp(const char *callingFunctionSignature)
{
    printf("exitApp was called by %s\n", callingFunctionSignature);
    assert(1 < 0);
}