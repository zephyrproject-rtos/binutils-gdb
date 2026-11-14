# RH850 FXU instructions: move
	.text
	movv.w4	wr2, wr3
	movv.w4	wr17, wr30
	flpv.s4	2, wr2, wr3
	flpv.s4	3, wr17, wr30
	shflv.w4	2643, wr1, wr2, wr3
	shflv.w4	4095, wr31, wr17, wr30
	trfsrv.w4	5, wr2
	trfsrv.w4	0, wr17
