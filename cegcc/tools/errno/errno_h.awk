BEGIN {
  print "\
#ifndef _LIB_CE_ERRNO_H\n\
#define _LIB_CE_ERRNO_H\n\
\n\
#include <winerror.h>\n\
\n\
#define errno __liberrno_errno ()\n\
#define __set_errno(ERR) SetLastError (ERR)\n\
\n\
#ifdef __cplusplus\n\
extern \"C\" {\n\
#endif\n\
\n\
int __liberrno_errno (void);\n\
\n\
char *strerror (int error);\n\
void perror (const char *s);\n\
\n\
#ifdef __cplusplus\n\
}\n\
#endif\n\
\n";
}

{
  errno = $1;
  winerr = $2;

  if (errno != "" && substr (errno, 1, 1) != "#")
    {
      if (errno != prev_errno)
	print "#define " errno " " winerr;
      prev_errno = errno;
    }
}

END {
  print "";
  print "#endif /* _LIB_CE_ERRNO_H */";
}
