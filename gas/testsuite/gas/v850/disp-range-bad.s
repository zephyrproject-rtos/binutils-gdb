# Branch displacements one step beyond their fields.
	.text
	jr	1f
	.skip	0x1ffffc
1:	nop
2:	nop
	.skip	0x200000
	jr	2b
	jarl	3f, r31
	.skip	0x1ffffc
3:	nop
	bne17	4f
	.skip	0xfffc
4:	nop
	br	5f
	.skip	0x200004
5:	nop
	bne	6f
	.skip	0x200004
6:	nop
