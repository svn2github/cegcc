#ifndef COMMON_H
#define COMMON_H

#include <windows.h>
struct struct64
{
    __int64 value;
};

struct struct32
{
    int value;
};

struct T1
{
    unsigned short f1;
    int f2;
    char f3;
    char f4;
};

struct T2
{
    unsigned short f1;
    __int64 f2;
    char f3;
    char f4;
};

struct T3
{
    unsigned short f1;
    double f2;
    char f3;
    char f4;
};

#endif
