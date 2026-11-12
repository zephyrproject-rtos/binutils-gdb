# Loop relaxation: a target beyond the reach of the 22-bit jump.
	.text
1:	nop
	.skip	0x1ffffc
	loop	r4, 1b
