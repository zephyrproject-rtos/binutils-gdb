#objdump: -d --prefix-addresses --show-raw-insn
#name: v850 RH850 FXU arith instructions
#as: -mv850e3v5 -mfxu

.*: +file format .*v850.*

Disassembly of section .text:
0x00000000 f0 17 a0 1d[ 	]+absf\.s4[ 	]+.*
0x00000004 f0 8f a0 f5[ 	]+absf\.s4[ 	]+.*
0x00000008 e1 17 a4 1d[ 	]+addf\.s4[ 	]+.*
0x0000000c ff 8f a4 f5[ 	]+addf\.s4[ 	]+.*
0x00000010 e1 17 ae 1d[ 	]+divf\.s4[ 	]+.*
0x00000014 ff 8f ae f5[ 	]+divf\.s4[ 	]+.*
0x00000018 e1 17 aa 1d[ 	]+maxf\.s4[ 	]+.*
0x0000001c ff 8f aa f5[ 	]+maxf\.s4[ 	]+.*
0x00000020 e1 17 ac 1d[ 	]+minf\.s4[ 	]+.*
0x00000024 ff 8f ac f5[ 	]+minf\.s4[ 	]+.*
0x00000028 e1 17 a8 1d[ 	]+mulf\.s4[ 	]+.*
0x0000002c ff 8f a8 f5[ 	]+mulf\.s4[ 	]+.*
0x00000030 f1 17 a0 1d[ 	]+negf\.s4[ 	]+.*
0x00000034 f1 8f a0 f5[ 	]+negf\.s4[ 	]+.*
0x00000038 f3 17 a0 1d[ 	]+recipf\.s4[ 	]+.*
0x0000003c f3 8f a0 f5[ 	]+recipf\.s4[ 	]+.*
0x00000040 f4 17 a0 1d[ 	]+rsqrtf\.s4[ 	]+.*
0x00000044 f4 8f a0 f5[ 	]+rsqrtf\.s4[ 	]+.*
0x00000048 f2 17 a0 1d[ 	]+sqrtf\.s4[ 	]+.*
0x0000004c f2 8f a0 f5[ 	]+sqrtf\.s4[ 	]+.*
0x00000050 e1 17 a6 1d[ 	]+subf\.s4[ 	]+.*
0x00000054 ff 8f a6 f5[ 	]+subf\.s4[ 	]+.*
0x00000058 e1 17 c0 1c[ 	]+fmaf\.s4[ 	]+.*
0x0000005c ff 8f c0 f4[ 	]+fmaf\.s4[ 	]+.*
0x00000060 e1 17 c2 1c[ 	]+fmsf\.s4[ 	]+.*
0x00000064 ff 8f c2 f4[ 	]+fmsf\.s4[ 	]+.*
0x00000068 e1 17 c4 1c[ 	]+fnmaf\.s4[ 	]+.*
0x0000006c ff 8f c4 f4[ 	]+fnmaf\.s4[ 	]+.*
0x00000070 e1 17 c6 1c[ 	]+fnmsf\.s4[ 	]+.*
0x00000074 ff 8f c6 f4[ 	]+fnmsf\.s4[ 	]+.*
