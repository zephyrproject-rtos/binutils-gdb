/* dpu.h --- DPU emulator for stand-alone RX simulator.

Copyright (C) 2018- Free Software Foundation, Inc.
Contributed by CyberThor Studios Ltd.

This file is part of the GNU simulators.

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

typedef unsigned long long dp_t;
typedef unsigned int fp_t;

extern dp_t rxdp_add (dp_t fa, dp_t fb);
extern dp_t rxdp_sub (dp_t fa, dp_t fb);
extern unsigned int rxdp_cmp (dp_t fa, dp_t fb, unsigned int cond);
extern dp_t rxdp_div (dp_t fa, dp_t fb);
extern dp_t rxdp_mul (dp_t fa, dp_t fb);
extern dp_t rxdp_abs (dp_t fa);
extern dp_t rxdp_neg (dp_t fa);
extern dp_t rxdp_dsqrt (dp_t fa);
extern fp_t rxdp_dtof (dp_t fa,  int round_mode);
extern int rxdp_dtoi (dp_t fa, int round_mode);
extern unsigned int rxdp_dtou (dp_t fa, int round_mode);
extern dp_t rxdp_ftod (fp_t fa);
extern dp_t rxdp_itod (int fa);
extern dp_t rxdp_utod (unsigned int fa);
extern dp_t rxdp_round (dp_t fa, int round_mode);
