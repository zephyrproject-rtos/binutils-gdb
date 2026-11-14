#objdump: -d --prefix-addresses --show-raw-insn
#name: v850 branch displacements at the field limits
#as: -mv850e3v5 -mextension

.*: +file format .*v850.*

Disassembly of section .text:
0x0+0000000 9f 07 fe ff[ 	]+jr[ 	]+0x0+1ffffe
#...
0x0+0400000 a0 07 00 00[ 	]+jr[ 	]+0x0+200000
0x0+0400004 9f ff fe ff[ 	]+jarl[ 	]+0x0+600002, lp
#...
0x0+0600004 ea 07 ff ff[ 	]+bne[ 	]+0x0+610002
#pass
