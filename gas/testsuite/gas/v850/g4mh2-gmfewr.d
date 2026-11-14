#objdump: -d -M g4mh2 --prefix-addresses --show-raw-insn
#name: v850 RH850 G4MH2 GMFEWR
#as: -mv850e3v5 -mg4mh2

.*: +file format .*v850.*

Disassembly of section .text:
0x00000000 fd 0f 40 48[ 	]+stsr[ 	]+gmfewr, r1, 9
0x00000004 e2 ef 20 48[ 	]+ldsr[ 	]+r2, gmfewr, 9
