#ifndef ASM_DEFS_H
#define ASM_DEFS_H

/* ANSI concatenation macros.  */
#define CONCAT(a, b)  CONCAT2(a, b)
#define CONCAT2(a, b) a##b

#ifndef __USER_LABEL_PREFIX__
#error  __USER_LABEL_PREFIX__ not defined
#endif

#define SYM(x) CONCAT (__USER_LABEL_PREFIX__, x)

#define TYPE(x) .def SYM(x); .scl 2; .type 32; .endef
#define SIZE(x)

#define EQUIV .set

.macro FUNC_START name
  .text
  .align 2
  .globl SYM (\name)
  TYPE (\name)
  SYM (\name):
.endm

.macro FUNC_END name
  SIZE (\name)
.endm

.macro	FUNC_ALIAS new old
  .globl	SYM (\new)
  TYPE (\new)
  EQUIV	SYM (\new), SYM (\old)
.endm

#endif
