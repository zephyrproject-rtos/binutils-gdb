# Data area relocations that the rh850 ABI has no relocation for.
	.text
	ld.w	zdaoff(_z)[r0], r10
	st.w	r10, zdaoff(_z)[r0]
	ld.h	zdaoff(_z)[r0], r10
	ld.bu	zdaoff(_z)[r0], r10
	movea	sdaoff(_s), gp, r10
	ld.bu	sdaoff(_s)[gp], r10
	movea	tdaoff(_t), ep, r10
	sld.w	tdaoff(_t)[ep], r10
	sst.w	r10, tdaoff(_t)[ep]
	sld.h	tdaoff(_t)[ep], r10
	sld.b	tdaoff(_t)[ep], r10
	sld.hu	tdaoff(_t)[ep], r10
	sld.bu	tdaoff(_t)[ep], r10
	.data
	.hword	sdaoff(_s)
	.hword	tdaoff(_t)
	.hword	ctoff(_c)
