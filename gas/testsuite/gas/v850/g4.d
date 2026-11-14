#objdump: -d --prefix-addresses --show-raw-insn
#name: v850 RH850 G4 instructions
#as: -mv850e3v5 -mg4

.*: +file format .*v850.*

Disassembly of section .text:
0x00000000 e1 17 08 00[ 	]+clip\.b[ 	]+r1, r2
0x00000004 ea ef 08 00[ 	]+clip\.b[ 	]+r10, r29
0x00000008 e1 17 0a 00[ 	]+clip\.bu[ 	]+r1, r2
0x0000000c ea ef 0a 00[ 	]+clip\.bu[ 	]+r10, r29
0x00000010 e1 17 0c 00[ 	]+clip\.h[ 	]+r1, r2
0x00000014 ea ef 0c 00[ 	]+clip\.h[ 	]+r10, r29
0x00000018 e1 17 0e 00[ 	]+clip\.hu[ 	]+r1, r2
0x0000001c ea ef 0e 00[ 	]+clip\.hu[ 	]+r10, r29
0x00000020 ff ff 60 f1[ 	]+cll[ 	]*$
0x00000024 e1 0f 70 33[ 	]+ldl\.bu[ 	]+\[r1\], r6
0x00000028 f4 0f 70 5b[ 	]+ldl\.bu[ 	]+\[r20\], r11
0x0000002c e1 0f 74 33[ 	]+ldl\.hu[ 	]+\[r1\], r6
0x00000030 f4 0f 74 5b[ 	]+ldl\.hu[ 	]+\[r20\], r11
0x00000034 e1 07 72 33[ 	]+stc\.b[ 	]+r6, \[r1\]
0x00000038 f4 07 72 5b[ 	]+stc\.b[ 	]+r11, \[r20\]
0x0000003c e1 07 76 33[ 	]+stc\.h[ 	]+r6, \[r1\]
0x00000040 f4 07 76 5b[ 	]+stc\.h[ 	]+r11, \[r20\]
0x00000044 e0 07 60 81[ 	]+resbank[ 	]*$
