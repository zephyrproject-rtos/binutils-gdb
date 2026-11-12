# RH850 FXU system register names
	.text
	stsr	fxsr, r1
	ldsr	r2, fxsr
	stsr	fxst, r3
	stsr	fxinfo, r4
	stsr	fxcfg, r5
	ldsr	r6, fxcfg
	stsr	fxxc, r7
	stsr	fxxp, r8
