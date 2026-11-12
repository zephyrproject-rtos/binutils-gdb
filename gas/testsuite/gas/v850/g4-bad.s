# RH850 G4 instructions without -mg4
	.text
	clip.b	r1, r2
	clip.b	r10, r29
	clip.bu	r1, r2
	clip.bu	r10, r29
	clip.h	r1, r2
	clip.h	r10, r29
	clip.hu	r1, r2
	clip.hu	r10, r29
	ldl.bu	[r1], r6
	ldl.bu	[r20], r11
	ldl.hu	[r1], r6
	ldl.hu	[r20], r11
	stc.b	r6, [r1]
	stc.b	r11, [r20]
	stc.h	r6, [r1]
	stc.h	r11, [r20]
	resbank
