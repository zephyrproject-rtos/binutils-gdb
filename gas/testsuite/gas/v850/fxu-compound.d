#objdump: -d --prefix-addresses --show-raw-insn
#name: v850 RH850 FXU compound instructions
#as: -mv850e3v5 -mfxu

.*: +file format .*v850.*

Disassembly of section .text:
0x00000000 e1 17 d0 1d[ 	]+addsubf\.s4[ 	]+.*
0x00000004 ff 8f d0 f5[ 	]+addsubf\.s4[ 	]+.*
0x00000008 e1 17 d8 1d[ 	]+addsubnf\.s4[ 	]+.*
0x0000000c ff 8f d8 f5[ 	]+addsubnf\.s4[ 	]+.*
0x00000010 e1 17 d2 1d[ 	]+subaddf\.s4[ 	]+.*
0x00000014 ff 8f d2 f5[ 	]+subaddf\.s4[ 	]+.*
0x00000018 e1 17 da 1d[ 	]+subaddnf\.s4[ 	]+.*
0x0000001c ff 8f da f5[ 	]+subaddnf\.s4[ 	]+.*
0x00000020 e1 17 c4 1d[ 	]+addxf\.s4[ 	]+.*
0x00000024 ff 8f c4 f5[ 	]+addxf\.s4[ 	]+.*
0x00000028 e1 17 c8 1d[ 	]+mulxf\.s4[ 	]+.*
0x0000002c ff 8f c8 f5[ 	]+mulxf\.s4[ 	]+.*
0x00000030 e1 17 c6 1d[ 	]+subxf\.s4[ 	]+.*
0x00000034 ff 8f c6 f5[ 	]+subxf\.s4[ 	]+.*
0x00000038 e1 17 dc 1d[ 	]+addsubnxf\.s4[ 	]+.*
0x0000003c ff 8f dc f5[ 	]+addsubnxf\.s4[ 	]+.*
0x00000040 e1 17 d4 1d[ 	]+addsubxf\.s4[ 	]+.*
0x00000044 ff 8f d4 f5[ 	]+addsubxf\.s4[ 	]+.*
0x00000048 e1 17 de 1d[ 	]+subaddnxf\.s4[ 	]+.*
0x0000004c ff 8f de f5[ 	]+subaddnxf\.s4[ 	]+.*
0x00000050 e1 17 d6 1d[ 	]+subaddxf\.s4[ 	]+.*
0x00000054 ff 8f d6 f5[ 	]+subaddxf\.s4[ 	]+.*
0x00000058 e1 17 b4 1d[ 	]+addrf\.s4[ 	]+.*
0x0000005c ff 8f b4 f5[ 	]+addrf\.s4[ 	]+.*
0x00000060 e1 17 ba 1d[ 	]+maxrf\.s4[ 	]+.*
0x00000064 ff 8f ba f5[ 	]+maxrf\.s4[ 	]+.*
0x00000068 e1 17 bc 1d[ 	]+minrf\.s4[ 	]+.*
0x0000006c ff 8f bc f5[ 	]+minrf\.s4[ 	]+.*
0x00000070 e1 17 b8 1d[ 	]+mulrf\.s4[ 	]+.*
0x00000074 ff 8f b8 f5[ 	]+mulrf\.s4[ 	]+.*
0x00000078 e1 17 b6 1d[ 	]+subrf\.s4[ 	]+.*
0x0000007c ff 8f b6 f5[ 	]+subrf\.s4[ 	]+.*
