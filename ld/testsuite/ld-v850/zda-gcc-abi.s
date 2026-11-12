	.text
	ld.w	zdaoff(zw)[r0], r12
	movea	zdaoff(zw), r0, r13
	ld.bu	zdaoff(zb)[r0], r14
	.section .zdata, "aw"
	.word	0
zw:	.word	3
zb:	.byte	1
