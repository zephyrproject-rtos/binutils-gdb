#objdump: -d --prefix-addresses --show-raw-insn
#name: v850 system register names without -mg4
#as: -mv850e3v5

.*: +file format .*v850.*

Disassembly of section .text:
0x00000000 e6 0f 40 00[ 	]+stsr[ 	]+.*
0x00000004 e5 0f 40 00[ 	]+stsr[ 	]+.*
0x00000008 e6 0f 40 00[ 	]+stsr[ 	]+.*
0x0000000c ed 0f 40 00[ 	]+stsr[ 	]+.*
0x00000010 e0 0f 40 00[ 	]+stsr[ 	]+.*
0x00000014 e4 0f 40 00[ 	]+stsr[ 	]+.*
0x00000018 f4 0f 40 00[ 	]+stsr[ 	]+.*
