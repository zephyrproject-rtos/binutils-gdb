# RH850 FXU instructions: mem
	.text
	ldv.dw	2, 0[r5], wr3
	ldv.dw	3, -16[r20], wr30
	ldv.qw	0[r5], wr3
	ldv.qw	-16[r20], wr30
	ldv.w	9, 0[r5], wr3
	ldv.w	15, -16[r20], wr30
	ldvz.h4	0[r5], wr3
	ldvz.h4	-16[r20], wr30
	stv.dw	1, wr3, 0[r5]
	stv.dw	0, wr30, -16[r20]
	stv.qw	wr3, 0[r5]
	stv.qw	wr30, -16[r20]
	stv.w	2, wr3, 0[r5]
	stv.w	3, wr30, -16[r20]
	stvz.h4	wr3, 0[r5]
	stvz.h4	wr30, -16[r20]
