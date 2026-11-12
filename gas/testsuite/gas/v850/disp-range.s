# Branch displacements at the limits of their fields.
	.text
	jr	1f
	.skip	0x1ffffa
1:	nop
2:	nop
	.skip	0x1ffffe
	jr	2b
	jarl	3f, r31
	.skip	0x1ffffa
3:	nop
	bne17	4f
	.skip	0xfffa
4:	nop
