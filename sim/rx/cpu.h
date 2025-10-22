/* cpu.h --- declarations for the RX core.

Copyright (C) 2005-2024 Free Software Foundation, Inc.
Contributed by Red Hat, Inc.

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

#include <stdint.h>
#include <setjmp.h>

extern int verbose;
extern int trace;
extern int enable_counting;

typedef uint8_t QI;
typedef uint16_t HI;
typedef uint32_t SI;
typedef uint64_t DI;

extern int rx_in_gdb;
extern int rx_big_endian;

typedef struct
{
  SI r[16];
  DI dr[16];

  SI r_psw;
  SI r_pc;
  SI r_usp;
  SI r_fpsw;
  SI r__reserved_cr_4;
  SI r__reserved_cr_5;
  SI r__reserved_cr_6;
  SI r__reserved_cr_7;

  SI r_bpsw;
  SI r_bpc;
  SI r_isp;
  SI r_fintv;
  SI r_intb;
  SI r_extb;
  SI r__reserved_cr_14;
  SI r__reserved_cr_15;

  SI r__reserved_cr_16;
  SI r__reserved_cr_17;
  SI r__reserved_cr_18;
  SI r__reserved_cr_19;
  SI r__reserved_cr_20;
  SI r__reserved_cr_21;
  SI r__reserved_cr_22;
  SI r__reserved_cr_23;

  SI r__reserved_cr_24;
  SI r__reserved_cr_25;
  SI r__reserved_cr_26;
  SI r__reserved_cr_27;
  SI r__reserved_cr_28;
  SI r__reserved_cr_29;
  SI r__reserved_cr_30;
  SI r__reserved_cr_31;

  SI r_temp;

  /*DI r_acc;*/
  DI r_acc0[2];
  DI r_acc1[2];

  SI r_dpsw;
  SI r_dcmr;
  SI r_decnt;
  SI r_depc;

  SI bit_li : 1;

#ifdef CYCLE_ACCURATE
  /* If set, RTS/RTSD take 2 fewer cycles.  */
  char fast_return;
  SI link_register;

  unsigned long long cycle_count;
  /* Bits saying what kind of memory operands the previous insn had.  */
  int m2m;
  /* Target register for load. */
  int rt;
#endif
} regs_type;

#define M2M_SRC		0x01
#define M2M_DST		0x02
#define M2M_BOTH	0x03

#define sp	0
#define dr0 16
#define psw 32
#define pc  33
#define usp 34
#define fpsw  35

#define bpsw  40
#define bpc 41
#define isp 42
#define fintv 43
#define intb  44
#define extb  45

#define r_temp_idx 64
#define acc0  65
#define acc1  66
#define acc0hi  67
#define acc0mi  68
#define acc0lo  69
#define acc1hi  70
#define acc1mi  71
#define acc1lo  72

#define dpsw 73
#define dcmr 74
#define decnt 75
#define depc 76

#define libit 78

extern regs_type regs;

#define FLAGBIT_C	0x00000001
#define FLAGBIT_Z	0x00000002
#define FLAGBIT_S	0x00000004
#define FLAGBIT_O	0x00000008
#define FLAGBIT_I	0x00010000
#define FLAGBIT_U	0x00020000
#define FLAGBIT_PM	0x00100000
#define FLAGBITS_IPL	0x0f000000
#define FLAGSHIFT_IPL	24

#define FPSWBITS_RM	0x00000003
#define FPSWBITS_CV	0x00000004 /* invalid operation */
#define FPSWBITS_CO	0x00000008 /* overflow */
#define FPSWBITS_CZ	0x00000010 /* divide-by-zero */
#define FPSWBITS_CU	0x00000020 /* underflow */
#define FPSWBITS_CX	0x00000040 /* inexact */
#define FPSWBITS_CE	0x00000080 /* unimplemented processing */
#define FPSWBITS_CMASK	0x000000fc /* all the above */
#define FPSWBITS_DN	0x00000100
#define FPSW_CESH	8
#define FPSWBITS_EV	0x00000400
#define FPSWBITS_EO	0x00000800
#define FPSWBITS_EZ	0x00001000
#define FPSWBITS_EU	0x00002000
#define FPSWBITS_EX	0x00004000
#define FPSW_EFSH	16
#define FPSW_CFSH	24
#define FPSWBITS_FV	0x04000000
#define FPSWBITS_FO	0x08000000
#define FPSWBITS_FZ	0x10000000
#define FPSWBITS_FU	0x20000000
#define FPSWBITS_FX	0x40000000
#define FPSWBITS_FSUM	0x80000000
#define FPSWBITS_FMASK  0x3c000000
#define FPSWBITS_CLEAR	0xffffff03 /* masked at start of any FP opcode */

#define FPRM_NEAREST	0
#define FPRM_ZERO	1
#define FPRM_PINF	2
#define FPRM_NINF	3

/*
 * Double-Precision Floating-Point Status Word (DPSW)
 */
#define DPSWBITS_DRM 0x00000003 /* Double-precision floating-point rounding-mode setting bits */
#define DPSWBITS_DCV 0x00000004 /* Invalid operation cause flag */
#define DPSWBITS_DCO 0x00000008 /* Overlow cause flag */
#define DPSWBITS_DCZ 0x00000010 /* Division-by-zero cause flag */
#define DPSWBITS_DCU 0x00000020 /* Underflow cause flag */
#define DPSWBITS_DCX 0x00000040 /* Inexact cause flag */
#define DPSWBITS_DCE 0x00000080 /* Unimplemented processing flag */
#define DPSWBITS_CMASK  0x000000fc /* all the above */
#define DPSWBITS_DDN 0x00000100 /* 0 flush bit of denormalized number */
#define DPSWBITS_B9  0x00000200 /* Reserved */
#define DPSWBITS_DEV 0x00000400 /* Invalid operation exception enable bit */
#define DPSWBITS_DEO 0x00000800 /* Overflow exception enable bit */
#define DPSWBITS_DEZ 0x00001000 /* Division-by-zero exception enable bit */
#define DPSWBITS_DEU 0x00002000 /* Underflow exception enable bit */
#define DPSWBITS_DEX 0x00004000 /* Inexact exception enable bit */
#define DPSWBITS_B15 0x00008000 /* Reserved */
#define DPSWBITS_B16 0x00010000 /* Reserved */
#define DPSWBITS_B17 0x00020000 /* Reserved */
#define DPSWBITS_B18 0x00040000 /* Reserved */
#define DPSWBITS_B19 0x00080000 /* Reserved */
#define DPSWBITS_B20 0x00100000 /* Reserved */
#define DPSWBITS_B21 0x00200000 /* Reserved */
#define DPSWBITS_B22 0x00400000 /* Reserved */
#define DPSWBITS_B23 0x00800000 /* Reserved */
#define DPSWBITS_B24 0x01000000 /* Reserved */
#define DPSWBITS_B25 0x02000000 /* Reserved */
#define DPSWBITS_DFV 0x04000000 /* Invalid operation flag */
#define DPSWBITS_DFO 0x08000000 /* Overflow flag */
#define DPSWBITS_DFZ 0x10000000 /* Division-by-zero flag */
#define DPSWBITS_DFU 0x20000000 /* Underflow flag */
#define DPSWBITS_DFX 0x40000000 /* Inexact flag */
#define DPSWBITS_DFS 0x80000000 /* Double-precision floating point error summary flag */

#define DPSWBITS_FMASK  0x3c000000 /*DFU, DFZ, DFO, DFV*/
#define DPSWBITS_CLEAR  0xffffff03

#define DPSW_CESH 8
#define DPSW_EFSH 16
#define DPSW_CFSH 24

#define DPRM_NEAREST  0
#define DPRM_ZERO 1
#define DPRM_PINF 2
#define DPRM_NINF 3

extern char *reg_names[];

extern int rx_flagmask;
extern int rx_flagand;
extern int rx_flagor;

extern unsigned int b2mask[];
extern unsigned int b2signbit[];
extern int b2maxsigned[];
extern int b2minsigned[];

void init_regs (void);
void stack_heap_stats (void);
void set_pointer_width (int bytes);
unsigned int get_reg (int id);
unsigned long long get_reg_double (int id);
unsigned long long * get_reg72 (int id);
void put_reg (int id, unsigned int value);
void put_reg72 (int id, unsigned long long * value);
void put_reg_double (int id, unsigned long long value);

void set_flags (int mask, int newbits);
void set_oszc (long long value, int bytes, int c);
void set_szc (long long value, int bytes, int c);
void set_osz (long long value, int bytes);
void set_sz (long long value, int bytes);
void set_zc (int z, int c);
void set_c (int c);

const char *bits (int v, int b);

int condition_true (int cond_id);

#define FLAG(f) ((regs.r_psw & f) ? 1 : 0)
#define FLAG_C	FLAG(FLAGBIT_C)
#define FLAG_D	FLAG(FLAGBIT_D)
#define FLAG_Z	FLAG(FLAGBIT_Z)
#define FLAG_S	FLAG(FLAGBIT_S)
#define FLAG_B	FLAG(FLAGBIT_B)
#define FLAG_O	FLAG(FLAGBIT_O)
#define FLAG_I	FLAG(FLAGBIT_I)
#define FLAG_U	FLAG(FLAGBIT_U)
#define FLAG_PM	FLAG(FLAGBIT_PM)

/* Instruction step return codes.
   Suppose one of the decode_* functions below returns a value R:
   - If RX_STEPPED (R), then the single-step completed normally.
   - If RX_HIT_BREAK (R), then the program hit a breakpoint.
   - If RX_EXITED (R), then the program has done an 'exit' system
     call, and the exit code is RX_EXIT_STATUS (R).
   - If RX_STOPPED (R), then a signal (number RX_STOP_SIG (R)) was
     generated.

   For building step return codes:
   - RX_MAKE_STEPPED is the return code for finishing a normal step.
   - RX_MAKE_HIT_BREAK is the return code for hitting a breakpoint.
   - RX_MAKE_EXITED (C) is the return code for exiting with status C.
   - RX_MAKE_STOPPED (S) is the return code for stopping on signal S.  */
#define RX_MAKE_STEPPED()   (1)
#define RX_MAKE_HIT_BREAK() (2)
#define RX_MAKE_EXITED(c)   (((int) (c) << 8) + 3)
#define RX_MAKE_STOPPED(s)  (((int) (s) << 8) + 4)

#define RX_STEPPED(r)       ((r) == RX_MAKE_STEPPED ())
#define RX_HIT_BREAK(r)     ((r) == RX_MAKE_HIT_BREAK ())
#define RX_EXITED(r)        (((r) & 0xff) == 3)
#define RX_EXIT_STATUS(r)   ((r) >> 8)
#define RX_STOPPED(r)       (((r) & 0xff) == 4)
#define RX_STOP_SIG(r)      ((r) >> 8)

/* The step result for the current step.  Global to allow
   communication between the stepping function and the system
   calls.  */
extern int step_result;

extern unsigned int rx_cycles;

/* Used to detect heap/stack collisions.  */
extern unsigned int heaptop;
extern unsigned int heapbottom;

extern int decode_opcode (void);
extern void reset_decoder (SI tpc);
extern void reset_pipeline_stats (void);
extern void halt_pipeline_stats (void);
extern void pipeline_stats (void);

extern void trace_register_changes ();
extern void generate_access_exception (void);
extern jmp_buf decode_jmp_buf;
