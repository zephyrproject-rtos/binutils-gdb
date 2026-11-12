#as: -mv850e3v5
#objdump: -dr
#name: v850 rh850 ABI data area relocations
#...
00000000 <.text>:
   0:	20 56 00 00 	movea	0, r0, r10
			2: R_V810_HWORD	_z
   4:	24 5f 01 00 	ld.w	0\[gp\], r11
			6: R_V810_GPWLO_1	_s
   8:	64 5f 01 00 	st.w	r11, 0\[gp\]
			a: R_V810_GPWLO_1	_s
#pass
