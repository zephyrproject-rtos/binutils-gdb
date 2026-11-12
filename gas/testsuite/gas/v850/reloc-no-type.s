# Relocations that the rh850 ABI has no ELF type for.
	.text
	ld.w	zdaoff(_z)[r0], r10
	sld.w	tdaoff(_t)[ep], r10
