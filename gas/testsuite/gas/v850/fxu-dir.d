#objdump: -d --prefix-addresses --show-raw-insn
#name: v850 RH850 .fxu directive
#as: -mv850e3v5

.*: +file format .*v850.*

Disassembly of section .text:
0x00000000 e1 17 a4 1d[ 	]+addf\.s4[ 	]+wr1, wr2, wr3
0x00000004 a1 07 9d 12 10 00[ 	]+ldv\.qw[ 	]+16\[r1\], wr2
0x0000000a e6 0f 40 50[ 	]+stsr[ 	]+.*
