#source: zda-gcc-abi-neg.s -mv850e3v5 -mgcc-abi
#ld: -m v850 -Tzda-gcc-abi-neg.ld
#objdump: -d -j .text
#...
00010000 <.*>:
   10000:	20 7f 01 80 	ld.w	-32768\[r0\], r15
   10004:	80 87 01 80 	ld.bu	-32768\[r0\], r16
#pass
