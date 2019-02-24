#ifndef _MAKESYSTABLE_H
#define _MAKESYSTABLE_H

#include "windows.h"
#include <string>

using namespace std;

typedef struct _Service
{
    string *Name;
    int nArguments;
    string *sArguments;
} Service;

#endif // _MAKESYSTABLE_H