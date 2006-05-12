	.text

	.globl __EH_FRAME_BEGIN__
	.globl DllMainCRTStartup

@ for kernel exception handler		
	.globl __EH_HANDLER__
	.globl __EH_CODE_START__
		
	.extern _DllMainCRTStartup
	.extern _eh_handler
	
@ dummy, needed for eh!?

__EH_FRAME_BEGIN__: .word 0

@ for kernel exception handler, must be directly before ___EH_CODE_START__
__EH_HANDLER__:		
	.word _eh_handler
	.word 0
		
__EH_CODE_START__:		
DllMainCRTStartup:
@ branch unconditionally to _DllMainCRTStartup implemented in c
	stmdb sp!, {r11, lr}
	ldr r11, _PMyMain
	mov lr, pc
	mov pc, r11
	ldmia sp!, {r11, pc}

@	B _PMyMain

_PMyMain:  .word _DllMainCRTStartup

@ for kernel exception handler
	.section .pdata
	.word __EH_CODE_START__
@ max 22 bits for number of instructions
	.word 0xc0000002 | (0xFFFFF << 8)

