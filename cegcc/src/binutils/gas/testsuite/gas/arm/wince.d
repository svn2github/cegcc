#objdump: -dr --prefix-addresses --show-raw-insn
#name: ARM WinCE basic tests
#as: -mcpu=arm7m -EL
#source: wince.s
#not-skip: *-wince-*

# Some WinCE specific tests.

.*: +file format .*arm.*

Disassembly of section .text:
00000000 <global_data> 00000007 	andeq	r0, r0, r7
			0: ARM_32	global_data
00000004 <global_sym> e1a00000 	nop			\(mov r0,r0\)
00000008 <global_sym\+0x4> e1a00000 	nop			\(mov r0,r0\)
0000000c <global_sym\+0x8> e1a00000 	nop			\(mov r0,r0\)
00000010 <global_sym\+0xc> eafffffb 	b	fffffff8 <global_sym\+0xfffffff4>
			10: ARM_26D	global_sym\+0xfffffffc
00000018 <global_sym\+0x14> ebfffffa 	bl	fffffff4 <global_sym\+0xfffffff0>
			14: ARM_26D	global_sym\+0xfffffffc
0000001c <global_sym\+0x18> 0afffff9 	beq	fffffff0 <global_sym\+0xffffffec>
			18: ARM_26D	global_sym\+0xfffffffc
00000020 <global_sym\+0x1c> eafffff8 	b	00000008 <global_sym\+0x4>
00000024 <global_sym\+0x20> ebfffff7 	bl	00000008 <global_sym\+0x4>
00000028 <global_sym\+0x24> 0afffff6 	beq	00000008 <global_sym\+0x4>
0000002c <global_sym\+0x28> eafffff5 	b	00000008 <global_sym\+0x4>
00000030 <global_sym\+0x2c> ebfffff4 	bl	00000008 <global_sym\+0x4>
00000034 <global_sym\+0x30> e51f0034 	ldr	r0, \[pc, #-52\]	; 00000008 <global_sym\+0x4>
00000038 <global_sym\+0x34> e51f0038 	ldr	r0, \[pc, #-56\]	; 00000008 <global_sym\+0x4>
0000003c <global_sym\+0x38> e51f003c 	ldr	r0, \[pc, #-60\]	; 00000008 <global_sym\+0x4>
