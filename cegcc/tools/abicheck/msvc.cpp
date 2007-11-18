#include <windows.h>
#include <stdio.h>

#include "common.h"
#include "msvc.h"

void
print_offsets (void)
{
#define poffset(T, F) \
    printf("offsetof (%s::%s) = %i\n", #T, #F, offsetof (T, F))

    printf("sizeof (struct T1) = %i\n", sizeof (struct T1));
    poffset(struct T1, f1);
    poffset(struct T1, f2);
    poffset(struct T1, f3);
    poffset(struct T1, f4);

    printf("sizeof (struct T2) = %i\n", sizeof (struct T2));
    poffset(struct T2, f1);
    poffset(struct T2, f2);
    poffset(struct T2, f3);
    poffset(struct T2, f4);

    printf("sizeof (struct T3) = %i\n", sizeof (struct T3));
    poffset(struct T3, f1);
    poffset(struct T3, f2);
    poffset(struct T3, f3);
    poffset(struct T3, f4);
}

void
test_struct64 (const char *s, struct struct64 dd)
{
    printf("test_struct64, dd = %I64x\n", dd);
}

void
test_struct32 (const char *s, struct struct32 dd)
{
    printf("test_struct32, dd = %x\n", dd);
}

void
test_ldouble(const char *s, long double ld)
{
    printf("test_uint64, ld = %lf\n", ld);
}

void
test_uint64(const char *s, unsigned __int64 ll)
{
    printf("test_uint64, ll = %I64x\n", ll);
}

struct struct64
test_ret_struct64 (void)
{
    struct struct64 ddd = { 0x1122334455667788i64 };
    return ddd;
}

struct struct32
test_ret_struct32 (void)
{
    struct struct32 ddd = { 0x11223344 };
    return ddd;
}

long double
test_ret_ldouble (void)
{
    return 1.2345678;
}

double
test_ret_double (void)
{
    return 2.3456789;
}

void
test_odd_regs64 (char a, __int64 b, char c, __int64 d, char e, __int64 f)
{
    printf("test_odd_regs64\n"
	    "\ta = %d\n"
	    "\tb = %I64x\n"
	    "\tc = %d\n"
	    "\td = %I64x\n"
	    "\te = %d\n"
	    "\tf = %I64x\n",
	    (int)a, b, (int)c, d, (int)e, f);
}
