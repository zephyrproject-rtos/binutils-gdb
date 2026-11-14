#objdump: -d -M g4mh2 --prefix-addresses --show-raw-insn
#name: v850 RH850 FXU system register names in objdump
#source: fxu-sreg.s
#as: -mv850e3v5 -mg4mh2

.*: +file format .*v850.*

Disassembly of section .text:
0x00000000 [0-9a-f ]+[ 	]+stsr[ 	]+fxsr, r1, 10
0x00000004 [0-9a-f ]+[ 	]+ldsr[ 	]+r2, fxsr, 10
0x00000008 [0-9a-f ]+[ 	]+stsr[ 	]+fxst, sp, 10
0x0000000c [0-9a-f ]+[ 	]+stsr[ 	]+fxinfo, gp, 10
0x00000010 [0-9a-f ]+[ 	]+stsr[ 	]+fxcfg, r5, 10
0x00000014 [0-9a-f ]+[ 	]+ldsr[ 	]+r6, fxcfg, 10
0x00000018 [0-9a-f ]+[ 	]+stsr[ 	]+fxxc, r7, 10
0x0000001c [0-9a-f ]+[ 	]+stsr[ 	]+fxxp, r8, 10
