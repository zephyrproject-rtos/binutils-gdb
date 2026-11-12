# .g4mh2 directive
	.text
	.g4mh2
	ldm.mp	[r1], r2-r5
	stsr	pmctrl0, r10
	clip.b	r1, r2
