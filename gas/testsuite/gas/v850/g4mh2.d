#objdump: -d --prefix-addresses --show-raw-insn
#name: v850 RH850 G4MH2 instructions
#as: -mv850e3v5 -mg4mh2

.*: +file format .*v850.*

Disassembly of section .text:
0x00000000 e1 17 66 29[ 	]+ldm\.mp[ 	]+\[r1\], r2-r5
0x00000004 f4 07 66 f9[ 	]+ldm\.mp[ 	]+\[r20\], r0-lp
0x00000008 e1 17 64 29[ 	]+stm\.mp[ 	]+r2-r5, \[r1\]
0x0000000c f4 07 64 f9[ 	]+stm\.mp[ 	]+r0-lp, \[r20\]
0x00000010 e7 07 60 99[ 	]+ldm\.gsr[ 	]+\[r7\]
0x00000014 ff 07 60 99[ 	]+ldm\.gsr[ 	]+\[lp\]
0x00000018 e7 07 60 91[ 	]+stm\.gsr[ 	]+\[r7\]
0x0000001c ff 07 60 91[ 	]+stm\.gsr[ 	]+\[lp\]
