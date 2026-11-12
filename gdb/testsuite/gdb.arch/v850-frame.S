/* This testcase is part of GDB, the GNU debugger.

   Copyright 2026 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* Two functions without a frame pointer and without CFI, so that GDB
   unwinds them with the prologue analyzer.  OUTER sets r29 to zero
   before it calls INNER.  */

	.text
	.globl	_outer
	.type	_outer, @function
_outer:
	addi	-8, sp, sp
	st.w	lp, 4[sp]
	st.w	r29, 0[sp]
	mov	r0, r29
	jarl	_inner, lp
	ld.w	0[sp], r29
	ld.w	4[sp], lp
	addi	8, sp, sp
	jmp	[lp]
	.size	_outer, . - _outer

	.globl	_inner
	.type	_inner, @function
_inner:
	addi	-8, sp, sp
	st.w	lp, 4[sp]
	nop
	ld.w	4[sp], lp
	addi	8, sp, sp
	jmp	[lp]
	.size	_inner, . - _inner
