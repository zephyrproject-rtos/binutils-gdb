#objdump: -d --prefix-addresses --show-raw-insn
#name: v850 RH850 FXU system register names
#as: -mv850e3v5 -mg4mh2

.*: +file format .*v850.*

Disassembly of section .text:
0x00000000 e6 0f 40 50[ 	]+stsr[ 	]+.*
0x00000004 e2 37 20 50[ 	]+ldsr[ 	]+.*
0x00000008 e8 1f 40 50[ 	]+stsr[ 	]+.*
0x0000000c e9 27 40 50[ 	]+stsr[ 	]+.*
0x00000010 ea 2f 40 50[ 	]+stsr[ 	]+.*
0x00000014 e6 57 20 50[ 	]+ldsr[ 	]+.*
0x00000018 ec 3f 40 50[ 	]+stsr[ 	]+.*
0x0000001c ed 47 40 50[ 	]+stsr[ 	]+.*
