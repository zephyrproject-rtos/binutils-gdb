#objdump: -d -M g4mh2 --prefix-addresses --show-raw-insn
#name: v850 .g4mh2 directive
#as: -mv850e3v5

.*: +file format .*v850.*

Disassembly of section .text:
0x00000000 e1 17 66 29[ 	]+ldm\.mp[ 	]+\[r1\], r2-r5
0x00000004 e0 57 40 70[ 	]+stsr[ 	]+pmctrl0, r10, 14
0x00000008 e1 17 08 00[ 	]+clip\.b[ 	]+r1, r2
