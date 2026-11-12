# RH850 FXU instructions without -mfxu
	.text
	addf.s4	wr1, wr2, wr3
	cmovf.w4	wr4, wr1, wr2, wr3
	shflv.w4	1, wr1, wr2, wr3
	ldv.qw	0[r1], wr2
	stv.w	1, wr2, 4[r1]
