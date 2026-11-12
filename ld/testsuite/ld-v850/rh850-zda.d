#source: rh850-zda.s -mv850e3v5
#ld: -Trh850-zda.ld
#objdump: -d
#...
00010000 <.*>:
   10000:	20 56 fe 7f 	movea	32766, r0, r10
   10004:	20 5e 00 80 	movea	-32768, r0, r11
#pass
