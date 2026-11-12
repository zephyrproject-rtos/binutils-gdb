#objdump: -d --prefix-addresses --show-raw-insn
#name: v850 RH850 FXU move instructions
#as: -mv850e3v5 -mfxu

.*: +file format .*v850.*

Disassembly of section .text:
0x00000000 fe 17 a0 1d[ 	]+movv\.w4[ 	]+.*
0x00000004 fe 8f a0 f5[ 	]+movv\.w4[ 	]+.*
0x00000008 fa 17 a0 1d[ 	]+flpv\.s4[ 	]+.*
0x0000000c fb 8f a0 f5[ 	]+flpv\.s4[ 	]+.*
0x00000010 a1 07 bd 1e 53 12[ 	]+shflv\.w4[ 	]+.*
0x00000016 bf 07 bd f6 ff 8f[ 	]+shflv\.w4[ 	]+.*
0x0000001c ff 17 a0 2d[ 	]+trfsrv\.w4[ 	]+.*
0x00000020 ff 8f a0 05[ 	]+trfsrv\.w4[ 	]+.*
