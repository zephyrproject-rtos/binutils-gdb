# RH850 G4MH2 instructions
	.text
	ldm.mp	[r1], r2-r5
	ldm.mp	[r20], r0-r31
	stm.mp	r2-r5, [r1]
	stm.mp	r0-r31, [r20]
	ldm.gsr	[r7]
	ldm.gsr	[r31]
	stm.gsr	[r7]
	stm.gsr	[r31]
