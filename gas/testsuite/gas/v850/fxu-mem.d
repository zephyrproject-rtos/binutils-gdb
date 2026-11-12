#objdump: -d --prefix-addresses --show-raw-insn
#name: v850 RH850 FXU mem instructions
#as: -mv850e3v5 -mfxu

.*: +file format .*v850.*

Disassembly of section .text:
0x00000000 a5 07 5d 1b 00 00[ 	]+ldv\.dw[ 	]+.*
0x00000006 b4 07 7d f3 f0 ff[ 	]+ldv\.dw[ 	]+.*
0x0000000c a5 07 9d 1a 00 00[ 	]+ldv\.qw[ 	]+.*
0x00000012 b4 07 9d f2 f0 ff[ 	]+ldv\.qw[ 	]+.*
0x00000018 a5 07 3d 19 00 00[ 	]+ldv\.w[ 	]+.*
0x0000001e b4 07 fd f1 f0 ff[ 	]+ldv\.w[ 	]+.*
0x00000024 a5 07 dd 1b 00 00[ 	]+ldvz\.h4[ 	]+.*
0x0000002a b4 07 dd f3 f0 ff[ 	]+ldvz\.h4[ 	]+.*
0x00000030 a5 07 bd 1b 00 00[ 	]+stv\.dw[ 	]+.*
0x00000036 b4 07 9d f3 f0 ff[ 	]+stv\.dw[ 	]+.*
0x0000003c a5 07 bd 1a 00 00[ 	]+stv\.qw[ 	]+.*
0x00000042 b4 07 bd f2 f0 ff[ 	]+stv\.qw[ 	]+.*
0x00000048 a5 07 5d 1a 00 00[ 	]+stv\.w[ 	]+.*
0x0000004e b4 07 7d f2 f0 ff[ 	]+stv\.w[ 	]+.*
0x00000054 a5 07 fd 1b 00 00[ 	]+stvz\.h4[ 	]+.*
0x0000005a b4 07 fd f3 f0 ff[ 	]+stvz\.h4[ 	]+.*
