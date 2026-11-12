# .fxu directive
	.text
	.fxu
	addf.s4	wr1, wr2, wr3
	ldv.qw	16[r1], wr2
	stsr	fxsr, r1
