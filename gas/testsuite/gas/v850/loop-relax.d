#objdump: -d --prefix-addresses --show-raw-insn
#name: v850 loop relaxation
#as: -mv850e3v5

.*: +file format .*v850.*

Disassembly of section .text:
#...
0x0+000fffe e1 06 ff ff[ 	]+loop[ 	]+r1, 0x0+
0x0+0010002 5f 12[ 	]+add[ 	]+-1, r2
0x0+0010004 ea 07 05 01[ 	]+bne[ 	]+0x0+10108
#...
0x0+0210106 5f 22[ 	]+add[ 	]+-1, gp
0x0+0210108 b2 05[ 	]+be[ 	]+0x0+21010e
0x0+021010a a0 07 00 00[ 	]+jr[ 	]+0x0+1010a
#pass
