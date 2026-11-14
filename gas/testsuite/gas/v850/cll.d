#objdump: -d --prefix-addresses --show-raw-insn
#name: v850 cll
#as: -mv850e3v5

.*: +file format .*v850.*

Disassembly of section .text:
0x00000000 ff ff 60 f1[ 	]+cll[ 	]*$
