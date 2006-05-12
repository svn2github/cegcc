#include "asmdefs.h"

	.text

	.globl __EH_FRAME_BEGIN__
	.globl WinMainCRTStartup
	.globl _mainCRTStartup

@ for kernel exception handler		
	.globl __EH_HANDLER__
	.globl __EH_CODE_START__
		
	.extern _startup
	.extern WinMain
	.extern main
	.extern _eh_handler
	.extern __set_runtime_thread_mode
	
@ dummy, needed for eh!?

__EH_FRAME_BEGIN__: .word 0

@ for kernel exception handler, must be directly before ___EH_CODE_START__
	FUNC_START __EH_HANDLER__
	.word _eh_handler @handler
	.word 0           @handler data
		
__EH_CODE_START__:
WinMainCRTStartup:
_mainCRTStartup:
	stmdb sp!, {r11, lr}
	ldr r11, _P__set_runtime_thread_mode
	mov lr, pc
	mov pc, r11
	ldr r0, _PMyMain
	ldr r11, _PStartup
	mov lr, pc
	mov pc, r11
	ldmia sp!, {r11, pc}

_P__set_runtime_thread_mode: .word __set_runtime_thread_mode
_PStartup:  .word _startup
_PMyMain:   .word main

@ for kernel exception handler
	.section .pdata
	.word __EH_CODE_START__
@ max 22 bits for number of instructions
	.word 0xc0000002 | (0xFFFFF << 8)
