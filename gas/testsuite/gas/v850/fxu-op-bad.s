# RH850 FXU bad operands
	.text
	addf.s4	r1, wr2, wr3
	addf.s4	vr1, wr2, wr3
	ldv.qw	0[wr1], wr3
	movv.w4	wr32, wr3
	ldv.qw	8[r1], wr2
	ldv.w	1, 2[r1], wr2
	ldv.dw	1, 4[r1], wr2
	stv.qw	wr1, 4[r1]
	stvz.h4	wr1, 12[r1]
	ldv.qw	32768[r1], wr2
	ldv.qw	sym[r1], wr2
	flpv.s4	4, wr1, wr2
	shflv.w4	4096, wr1, wr2, wr3
	ldv.w	16, 0[r1], wr2
	stv.dw	2, wr1, 0[r1]
	trfsrv.w4	8, wr1
	cmpf.s4	16, wr1, wr2, wr3
	cmpf.s4	neq, wr1, wr2, wr3
