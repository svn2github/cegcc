#ifdef __GNUC__
# define _tmain main
# define LL(X) X ## LL
#else
# define LL(X) X ## i64
#endif

#include "common.h"
#include "msvc.h"

struct struct64 dd64 = { LL(0x1122334455667788) };
struct struct32 dd32 = { 0x11223344 };

int
_tmain (void)
{
    print_offsets ();

    test_struct64 ("", dd64);
    test_struct32 ("", dd32);

    struct struct64 ldd64 = test_ret_struct64 ();
    struct struct32 ldd32 = test_ret_struct32 ();

    long double ldouble = test_ret_ldouble();
    double doubl = test_ret_double();

    printf("test_ret_ldouble, ldouble = %lf\n", ldouble);
    printf("test_ret_double, doubl = %f\n", doubl);

    test_ldouble("", 0.12345678);
    test_uint64("", LL(0x1122334455667788));

    test_odd_regs64(0, LL(0x1111111122222222),
	3, LL(0x4444444455555555),
	6, LL(0x7777777788888888));
}
