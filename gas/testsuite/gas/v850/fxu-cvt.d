#objdump: -d --prefix-addresses --show-raw-insn
#name: v850 RH850 FXU cvt instructions
#as: -mv850e3v5 -mfxu

.*: +file format .*v850.*

Disassembly of section .text:
0x00000000 e5 17 a0 1d[ 	]+ceilf\.suw4[ 	]+.*
0x00000004 e5 8f a0 f5[ 	]+ceilf\.suw4[ 	]+.*
0x00000008 e4 17 a0 1d[ 	]+ceilf\.sw4[ 	]+.*
0x0000000c e4 8f a0 f5[ 	]+ceilf\.sw4[ 	]+.*
0x00000010 ec 17 a0 1d[ 	]+cvtf\.hs4[ 	]+.*
0x00000014 ec 8f a0 f5[ 	]+cvtf\.hs4[ 	]+.*
0x00000018 ed 17 a0 1d[ 	]+cvtf\.sh4[ 	]+.*
0x0000001c ed 8f a0 f5[ 	]+cvtf\.sh4[ 	]+.*
0x00000020 e9 17 a0 1d[ 	]+cvtf\.suw4[ 	]+.*
0x00000024 e9 8f a0 f5[ 	]+cvtf\.suw4[ 	]+.*
0x00000028 e8 17 a0 1d[ 	]+cvtf\.sw4[ 	]+.*
0x0000002c e8 8f a0 f5[ 	]+cvtf\.sw4[ 	]+.*
0x00000030 eb 17 a0 1d[ 	]+cvtf\.uws4[ 	]+.*
0x00000034 eb 8f a0 f5[ 	]+cvtf\.uws4[ 	]+.*
0x00000038 ea 17 a0 1d[ 	]+cvtf\.ws4[ 	]+.*
0x0000003c ea 8f a0 f5[ 	]+cvtf\.ws4[ 	]+.*
0x00000040 e7 17 a0 1d[ 	]+floorf\.suw4[ 	]+.*
0x00000044 e7 8f a0 f5[ 	]+floorf\.suw4[ 	]+.*
0x00000048 e6 17 a0 1d[ 	]+floorf\.sw4[ 	]+.*
0x0000004c e6 8f a0 f5[ 	]+floorf\.sw4[ 	]+.*
0x00000050 e1 17 a0 1d[ 	]+roundf\.suw4[ 	]+.*
0x00000054 e1 8f a0 f5[ 	]+roundf\.suw4[ 	]+.*
0x00000058 e0 17 a0 1d[ 	]+roundf\.sw4[ 	]+.*
0x0000005c e0 8f a0 f5[ 	]+roundf\.sw4[ 	]+.*
0x00000060 e3 17 a0 1d[ 	]+trncf\.suw4[ 	]+.*
0x00000064 e3 8f a0 f5[ 	]+trncf\.suw4[ 	]+.*
0x00000068 e2 17 a0 1d[ 	]+trncf\.sw4[ 	]+.*
0x0000006c e2 8f a0 f5[ 	]+trncf\.sw4[ 	]+.*
0x00000070 e1 17 8c 1d[ 	]+cmpf\.s4[ 	]+.*
0x00000074 ff 8f 9e f5[ 	]+cmpf\.s4[ 	]+.*
0x00000078 a1 07 1d 1e 04 10[ 	]+cmovf\.w4[ 	]+.*
0x0000007e bf 07 1d f6 1d 88[ 	]+cmovf\.w4[ 	]+.*
