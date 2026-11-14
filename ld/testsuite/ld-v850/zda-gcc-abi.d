#source: zda-gcc-abi.s -mv850e3v5 -mgcc-abi
#ld: -m v850 -Tzda-gcc-abi.ld
#objdump: -d -j .text
#...
00010000 <.*>:
   10000:	20 67 65 01 	ld.w	356\[r0\], r12
   10004:	20 6e 64 01 	movea	356, r0, r13
   10008:	80 77 69 01 	ld.bu	360\[r0\], r14
#pass
