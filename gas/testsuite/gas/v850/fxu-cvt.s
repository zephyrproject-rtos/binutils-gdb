# RH850 FXU instructions: cvt
	.text
	ceilf.suw4	wr2, wr3
	ceilf.suw4	wr17, wr30
	ceilf.sw4	wr2, wr3
	ceilf.sw4	wr17, wr30
	cvtf.hs4	wr2, wr3
	cvtf.hs4	wr17, wr30
	cvtf.sh4	wr2, wr3
	cvtf.sh4	wr17, wr30
	cvtf.suw4	wr2, wr3
	cvtf.suw4	wr17, wr30
	cvtf.sw4	wr2, wr3
	cvtf.sw4	wr17, wr30
	cvtf.uws4	wr2, wr3
	cvtf.uws4	wr17, wr30
	cvtf.ws4	wr2, wr3
	cvtf.ws4	wr17, wr30
	floorf.suw4	wr2, wr3
	floorf.suw4	wr17, wr30
	floorf.sw4	wr2, wr3
	floorf.sw4	wr17, wr30
	roundf.suw4	wr2, wr3
	roundf.suw4	wr17, wr30
	roundf.sw4	wr2, wr3
	roundf.sw4	wr17, wr30
	trncf.suw4	wr2, wr3
	trncf.suw4	wr17, wr30
	trncf.sw4	wr2, wr3
	trncf.sw4	wr17, wr30
	cmpf.s4	6, wr1, wr2, wr3
	cmpf.s4	15, wr31, wr17, wr30
	cmovf.w4	wr4, wr1, wr2, wr3
	cmovf.w4	wr29, wr31, wr17, wr30
