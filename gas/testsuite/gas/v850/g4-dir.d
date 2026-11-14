#objdump: -d -M g4 --prefix-addresses --show-raw-insn
#name: v850 .g4 directive
#as: -mv850e3v5

.*: +file format .*v850.*

Disassembly of section .text:
0x00000000 e1 17 08 00[ 	]+clip\.b[ 	]+r1, r2
0x00000004 e6 57 40 08[ 	]+stsr[ 	]+pid, r10, 1
0x00000008 e0 07 60 81[ 	]+resbank[ 	]*
