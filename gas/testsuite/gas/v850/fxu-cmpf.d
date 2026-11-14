#objdump: -d --prefix-addresses --show-raw-insn
#name: v850 RH850 FXU comparison conditions
#as: -mv850e3v5 -mfxu

.*: +file format .*v850.*

Disassembly of section .text:
0x00000000 e1 17 80 1d[ 	]+cmpf\.s4[ 	]+f, wr1, wr2, wr3
0x00000004 e1 17 82 1d[ 	]+cmpf\.s4[ 	]+un, wr1, wr2, wr3
0x00000008 e1 17 84 1d[ 	]+cmpf\.s4[ 	]+eq, wr1, wr2, wr3
0x0000000c e1 17 86 1d[ 	]+cmpf\.s4[ 	]+ueq, wr1, wr2, wr3
0x00000010 e1 17 88 1d[ 	]+cmpf\.s4[ 	]+olt, wr1, wr2, wr3
0x00000014 e1 17 8a 1d[ 	]+cmpf\.s4[ 	]+ult, wr1, wr2, wr3
0x00000018 e1 17 8c 1d[ 	]+cmpf\.s4[ 	]+ole, wr1, wr2, wr3
0x0000001c e1 17 8e 1d[ 	]+cmpf\.s4[ 	]+ule, wr1, wr2, wr3
0x00000020 e1 17 90 1d[ 	]+cmpf\.s4[ 	]+sf, wr1, wr2, wr3
0x00000024 e1 17 92 1d[ 	]+cmpf\.s4[ 	]+ngle, wr1, wr2, wr3
0x00000028 e1 17 94 1d[ 	]+cmpf\.s4[ 	]+seq, wr1, wr2, wr3
0x0000002c e1 17 96 1d[ 	]+cmpf\.s4[ 	]+ngl, wr1, wr2, wr3
0x00000030 e1 17 98 1d[ 	]+cmpf\.s4[ 	]+lt, wr1, wr2, wr3
0x00000034 e1 17 9a 1d[ 	]+cmpf\.s4[ 	]+nge, wr1, wr2, wr3
0x00000038 e1 17 9c 1d[ 	]+cmpf\.s4[ 	]+le, wr1, wr2, wr3
0x0000003c e1 17 9e 1d[ 	]+cmpf\.s4[ 	]+ngt, wr1, wr2, wr3
