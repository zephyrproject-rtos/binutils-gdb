# Loop relaxation: loop disp16, add + bne disp17, add + be + jr disp22.
	.text
1:	nop
	.skip	0xfffc
	loop	r1, 1b
	loop	r2, 2f
	.skip	0x100
2:	nop
3:	nop
	.skip	0x1ffffa
	loop	r4, 3b
