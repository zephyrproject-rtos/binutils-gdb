#objdump: -d -M g4 --prefix-addresses --show-raw-insn
#name: v850 unknown RH850 G4 system registers in objdump
#as: -mv850e3v5

.*: +file format .*v850.*

Disassembly of section .text:
0x00000000 e6 57 40 18[ 	]+stsr[ 	]+6, r10, 3
0x00000004 ff 0f 40 f8[ 	]+stsr[ 	]+31, r1, 31
0x00000008 e2 1f 20 48[ 	]+ldsr[ 	]+r2, 3, 9
