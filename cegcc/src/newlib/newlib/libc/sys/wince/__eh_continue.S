/* ANSI concatenation macros.  */
#define CONCAT(a, b)  CONCAT2(a, b)
#define CONCAT2(a, b) a##b

#ifndef __USER_LABEL_PREFIX__
#error  __USER_LABEL_PREFIX__ not defined
#endif

#define SYM(x) CONCAT (__USER_LABEL_PREFIX__, x)

#ifdef __ELF__
#define TYPE(x) .type SYM(x),function
#define SIZE(x) .size SYM(x), . - SYM(x)
#else
#define TYPE(x) .def SYM(x); .scl 2; .type 32; .endef
#define SIZE(x)
#endif

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

		.extern SYM(call_raise_c)
		
@ Args:	r0: context pointer	
	FUNC_START __eh_continue
		add	r0, r0, #4
		@Post-incremental load
        ldmfd   r0, {r0-r15}
	FUNC_END __eh_continue

	FUNC_START _call_raise_asm
		stmdb sp!, {r11}
		ldr r11, _Pcall_raise_c
		mov lr, pc
		mov pc, r11
		ldmia sp!, {r11}
		
		ldmfd  sp!, {r0}  
		msr CPSR, r0
		ldmia sp, {r0-r15}   
	FUNC_END _call_raise_asm

_Pcall_raise_c:  .word SYM(call_raise_c)
