# RH850 FXU instructions: arith
	.text
	absf.s4	wr2, wr3
	absf.s4	wr17, wr30
	addf.s4	wr1, wr2, wr3
	addf.s4	wr31, wr17, wr30
	divf.s4	wr1, wr2, wr3
	divf.s4	wr31, wr17, wr30
	maxf.s4	wr1, wr2, wr3
	maxf.s4	wr31, wr17, wr30
	minf.s4	wr1, wr2, wr3
	minf.s4	wr31, wr17, wr30
	mulf.s4	wr1, wr2, wr3
	mulf.s4	wr31, wr17, wr30
	negf.s4	wr2, wr3
	negf.s4	wr17, wr30
	recipf.s4	wr2, wr3
	recipf.s4	wr17, wr30
	rsqrtf.s4	wr2, wr3
	rsqrtf.s4	wr17, wr30
	sqrtf.s4	wr2, wr3
	sqrtf.s4	wr17, wr30
	subf.s4	wr1, wr2, wr3
	subf.s4	wr31, wr17, wr30
	fmaf.s4	wr1, wr2, wr3
	fmaf.s4	wr31, wr17, wr30
	fmsf.s4	wr1, wr2, wr3
	fmsf.s4	wr31, wr17, wr30
	fnmaf.s4	wr1, wr2, wr3
	fnmaf.s4	wr31, wr17, wr30
	fnmsf.s4	wr1, wr2, wr3
	fnmsf.s4	wr31, wr17, wr30
