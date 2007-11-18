#ifndef MSVC_H
#define MSVC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

void print_offsets (void);

void test_struct64 (const char *s, struct struct64 dd);
void test_struct32 (const char *s, struct struct32 dd);

struct struct64 test_ret_struct64 (void);
struct struct32 test_ret_struct32 (void);

long double test_ret_ldouble(void);
double test_ret_double(void);

void test_ldouble(const char *s, long double ld);
void test_uint64(const char *s, unsigned __int64 ll);

void test_odd_regs64(char a, __int64 b, char c, __int64 d, char e, __int64 f);

#ifdef __cplusplus
}
#endif

#endif
