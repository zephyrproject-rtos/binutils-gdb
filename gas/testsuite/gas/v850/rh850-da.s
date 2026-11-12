# Data area relocations that the rh850 ABI supports.
	.text
	movea	zdaoff(_z), r0, r10
	ld.w	sdaoff(_s)[gp], r11
	st.w	r11, sdaoff(_s)[gp]
	.data
	.hword	zdaoff(_z)
