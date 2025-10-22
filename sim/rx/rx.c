/* rx.c --- opcode semantics for stand-alone RX simulator.

Copyright (C) 2008-2024 Free Software Foundation, Inc.
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

/* This must come before any other includes.  */
#include "defs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "libiberty.h"

#include "opcode/rx.h"
#include "cpu.h"
#include "mem.h"
#include "syscalls.h"
#include "fpu.h"
#include "err.h"
#include "misc.h"
#include "dpu.h"
#include "bfd.h"

#ifdef WITH_PROFILE
static const char * const id_names[] = {
  "RXO_unknown",
  "RXO_mov",	/* d = s (signed) */
  "RXO_movli", /* d = s, LI = 1 */
  "RXO_movco", /* if (LI == 1) {dest=src;src=0;} else { src=1; } LI = 0; */
  "RXO_movbi",	/* d = [s,s2] (signed) */
  "RXO_movbir",	/* [s,s2] = d (signed) */
  "RXO_pushm",	/* s..s2 */
  "RXO_popm",	/* s..s2 */
  "RXO_xchg",	/* s <-> d */
  "RXO_stcc",	/* d = s if cond(s2) */
  "RXO_rtsd",	/* rtsd, 1=imm, 2-0 = reg if reg type */

  /* These are all either d OP= s or, if s2 is set, d = s OP s2.  Note
     that d may be "None".  */
  "RXO_and",
  "RXO_or",
  "RXO_xor",
  "RXO_add",
  "RXO_sub",
  "RXO_mul",
  "RXO_div",
  "RXO_divu",
  "RXO_shll",
  "RXO_shar",
  "RXO_shlr",

  "RXO_adc",	/* d = d + s + carry */
  "RXO_sbb",	/* d = d - s - ~carry */
  "RXO_abs",	/* d = |s| */
  "RXO_max",	/* d = max(d,s) */
  "RXO_min",	/* d = min(d,s) */
  "RXO_emul",	/* d:64 = d:32 * s */
  "RXO_emulu",	/* d:64 = d:32 * s (unsigned) */

  "RXO_rolc",	/* d <<= 1 through carry */
  "RXO_rorc",	/* d >>= 1 through carry*/
  "RXO_rotl",	/* d <<= #s without carry */
  "RXO_rotr",	/* d >>= #s without carry*/
  "RXO_revw",	/* d = revw(s) */
  "RXO_revl",	/* d = revl(s) */
  "RXO_branch",	/* pc = d if cond(s) */
  "RXO_branchrel",/* pc += d if cond(s) */
  "RXO_jsr",	/* pc = d */
  "RXO_jsrrel",	/* pc += d */
  "RXO_rts",
  "RXO_nop",
  "RXO_nop2",
  "RXO_nop3",
  "RXO_nop4",
  "RXO_nop5",
  "RXO_nop6",
  "RXO_nop7",

  "RXO_scmpu",
  "RXO_smovu",
  "RXO_smovb",
  "RXO_suntil",
  "RXO_swhile",
  "RXO_smovf",
  "RXO_sstr",

  "RXO_rmpa",
  "RXO_emula",
  "RXO_emaca",
  "RXO_emsba",
  "RXO_mullh",
  "RXO_mulhi",
  "RXO_mullo",
  "RXO_maclh",
  "RXO_machi",
  "RXO_maclo",
  "RXO_msblh",
  "RXO_msbhi",
  "RXO_msblo",
  "RXO_mvtachi",
  "RXO_mvtaclo",
  "RXO_mvtacgu",
  "RXO_mvfachi",
  "RXO_mvfacmi",
  "RXO_mvfaclo",
  "RXO_mvfacgu",
  "RXO_rdacw",
  "RXO_rdacl",
  "RXO_racw",
  "RXO_racl",

  "RXO_sat",	/* sat(d) */
  "RXO_satr",

  "RXO_fadd",	/* d op= s */
  "RXO_fcmp",
  "RXO_fsub",
  "RXO_ftoi",
  "RXO_fmul",
  "RXO_fdiv",
  "RXO_fsqrt",
  "RXO_round",
  "RXO_itof",
  "RXO_utof",
  "RXO_ftou",

  "RXO_bset",	/* d |= (1<<s) */
  "RXO_bclr",	/* d &= ~(1<<s) */
  "RXO_btst",	/* s & (1<<s2) */
  "RXO_bnot",	/* d ^= (1<<s) */
  "RXO_bmcc",	/* d<s> = cond(s2) */
  "RXO_bfmov",
  "RXO_bfmovz",

  "RXO_clrpsw",	/* flag index in d */
  "RXO_setpsw",	/* flag index in d */
  "RXO_mvtipl",	/* new IPL in s */

  "RXO_rtfi",
  "RXO_rstr",
  "RXO_rte",
  "RXO_rtd",	/* undocumented */
  "RXO_brk",
  "RXO_dbt",	/* undocumented */
  "RXO_int",	/* vector id in s */
  "RXO_stop",
  "RXO_save",
  "RXO_wait",

  "RXO_sccnd",	/* d = cond(s) ? 1 : 0 */

  "RXO_dabs",
  "RXO_dadd",
  "RXO_dcmp",
  "RXO_ddiv",
  "RXO_dmovhi",
  "RXO_dmov",
  "RXO_dmul",
  "RXO_dneg",
  "RXO_dpopm",
  "RXO_dpushm",
  "RXO_dround",
  "RXO_dsqrt",
  "RXO_dsub",
  "RXO_dtof",
  "RXO_dtoi",
  "RXO_dtou",
  "RXO_ftod",
  "RXO_itod",
  "RXO_mvfdc",
  "RXO_mvfdr",
  "RXO_mvtdc",
  "RXO_utod",
};

static const char * const optype_names[] = {
  " -  ",
  "#Imm",	/* #addend */
  " Rn ",	/* Rn */
  "[Rn]",	/* [Rn + addend] */
  "Ps++",	/* [Rn+] */
  "--Pr",	/* [-Rn] */
  " cc ",	/* eq, gtu, etc */
  "Flag",	/* [UIOSZC] */
  "RbRi"	/* [Rb + scale * Ri] */
};

#define N_RXO ARRAY_SIZE (id_names)
#define N_RXT ARRAY_SIZE (optype_names)
#define N_MAP 90

static unsigned long long benchmark_start_cycle;
static unsigned long long benchmark_end_cycle;

static int op_cache[N_RXT][N_RXT][N_RXT];
static int op_cache_rev[N_MAP];
static int op_cache_idx = 0;

static int
op_lookup (int a, int b, int c)
{
  if (op_cache[a][b][c])
    return op_cache[a][b][c];
  op_cache_idx ++;
  if (op_cache_idx >= N_MAP)
    {
      printf("op_cache_idx exceeds %d\n", N_MAP);
      exit(1);
    }
  op_cache[a][b][c] = op_cache_idx;
  op_cache_rev[op_cache_idx] = (a<<8) | (b<<4) | c;
  return op_cache_idx;
}

static char *
op_cache_string (int map)
{
  static int ci;
  static char cb[5][20];
  int a, b, c;

  map = op_cache_rev[map];
  a = (map >> 8) & 15;
  b = (map >> 4) & 15;
  c = (map >> 0) & 15;
  ci = (ci + 1) % 5;
  sprintf(cb[ci], "%s %s %s", optype_names[a], optype_names[b], optype_names[c]);
  return cb[ci];
}

static unsigned long long cycles_per_id[N_RXO][N_MAP];
static unsigned long long times_per_id[N_RXO][N_MAP];
static unsigned long long memory_stalls;
static unsigned long long register_stalls;
static unsigned long long branch_stalls;
static unsigned long long branch_alignment_stalls;
static unsigned long long fast_returns;

static unsigned long times_per_pair[N_RXO][N_MAP][N_RXO][N_MAP];
static int prev_opcode_id = RXO_unknown;
static int po0;

#define STATS(x) x

#else
#define STATS(x)
#endif /* WITH_PROFILE */


#ifdef CYCLE_ACCURATE

static int new_rt = -1;

/* Number of cycles to add if an insn spans an 8-byte boundary.  */
static int branch_alignment_penalty = 0;

#endif

static int running_benchmark = 1;

#define tprintf if (trace && running_benchmark) printf

jmp_buf decode_jmp_buf;
unsigned int rx_cycles = 0;

#ifdef CYCLE_ACCURATE
/* If nonzero, memory was read at some point and cycle latency might
   take effect.  */
static int memory_source = 0;
/* If nonzero, memory was written and extra cycles might be
   needed.  */
static int memory_dest = 0;

static void
cycles (int throughput)
{
  tprintf("%d cycles\n", throughput);
  regs.cycle_count += throughput;
}

/* Number of execution (E) cycles the op uses.  For memory sources, we
   include the load micro-op stall as two extra E cycles.  */
#define E(c) cycles (memory_source ? c + 2 : c)
#define E1 cycles (1)
#define E2 cycles (2)
#define EBIT cycles (memory_source ? 2 : 1)

/* Check to see if a read latency must be applied for a given register.  */
#define RL(r) \
  if (regs.rt == r )							\
    {									\
      tprintf("register %d load stall\n", r);				\
      regs.cycle_count ++;						\
      STATS(register_stalls ++);					\
      regs.rt = -1;							\
    }

#define RLD(r)					\
  if (memory_source)				\
    {						\
      tprintf ("Rt now %d\n", r);		\
      new_rt = r;				\
    }

static int
lsb_count (unsigned long v, int is_signed)
{
  int i, lsb;
  if (is_signed && (v & 0x80000000U))
    v = (unsigned long)(long)(-v);
  for (i=31; i>=0; i--)
    if (v & (1 << i))
      {
	/* v is 0..31, we want 1=1-2, 2=3-4, 3=5-6, etc. */
	lsb = (i + 2) / 2;
	return lsb;
      }
  return 0;
}

static int
divu_cycles(unsigned long num, unsigned long den)
{
  int nb = lsb_count (num, 0);
  int db = lsb_count (den, 0);
  int rv;

  if (nb < db)
    rv = 2;
  else
    rv = 3 + nb - db;
  E (rv);
  return rv;
}

static int
div_cycles(long num, long den)
{
  int nb = lsb_count ((unsigned long)num, 1);
  int db = lsb_count ((unsigned long)den, 1);
  int rv;

  if (nb < db)
    rv = 3;
  else
    rv = 5 + nb - db;
  E (rv);
  return rv;
}

#else /* !CYCLE_ACCURATE */

#define cycles(t)
#define E(c)
#define E1
#define E2
#define EBIT
#define RL(r)
#define RLD(r)

#define divu_cycles(n,d)
#define div_cycles(n,d)

#endif /* else CYCLE_ACCURATE */

static const int size2bytes[] = {
  4, 1, 1, 1, 2, 2, 2, 3, 4
};

#define rx_abort() _rx_abort(__FILE__, __LINE__)
static void ATTRIBUTE_NORETURN
_rx_abort (const char *file, int line)
{
  if (strrchr (file, '/'))
    file = strrchr (file, '/') + 1;
  fprintf(stderr, "abort at %s:%d\n", file, line);
  abort();
}

static unsigned char *get_byte_base;
static RX_Opcode_Decoded **decode_cache_base;
static SI get_byte_page;

void
reset_decoder (SI tpc)
{
  get_byte_base = 0;
  decode_cache_base = 0;
  get_byte_page = (~tpc) & NONPAGE_MASK;
}

static inline void
maybe_get_mem_page (SI tpc)
{
  if (((tpc ^ get_byte_page) & NONPAGE_MASK) || enable_counting)
    {
      get_byte_page = tpc & NONPAGE_MASK;
      get_byte_base = rx_mem_ptr (get_byte_page, MPA_READING) - get_byte_page;
      decode_cache_base = rx_mem_decode_cache (get_byte_page) - get_byte_page;
    }
}

/* This gets called a *lot* so optimize it.  */
static int
rx_get_byte (RX_Data *rx_data)
{
  SI tpc = rx_data->addr;

  /* See load.c for an explanation of this.  */
  if (rx_big_endian)
    tpc ^= 3;

  maybe_get_mem_page (tpc);

  rx_data->addr ++;
  return get_byte_base [tpc];
}

static long long *
get_op_72 (const RX_Opcode_Decoded *rd, int i)
{
	const RX_Opcode_Operand *o = rd->op + i;

	switch (o->type)
	{
	case RX_Operand_Register:	/* An */
		return get_reg72 (o->reg);
	default:
		abort ();
	}
}

static long long
get_op_64 (const RX_Opcode_Decoded *rd, int i)
{
	const RX_Opcode_Operand *o = rd->op + i;
	long long result;
	int addr;

	switch (o->type)
	{
	case RX_Operand_Register:	/* An */
		return get_reg72 (o->reg)[0];
	case RX_Operand_Zero_Indirect:
	case RX_Operand_Indirect:
		addr = get_reg (o->reg) + o->addend;
		result = mem_get_si (addr) & 0xFFFFFFFF;
		result |= (long long)mem_get_si (addr + 4) << 32;
		return result;
	default:
		abort ();
	}
}

static long long
get_op_double (const RX_Opcode_Decoded *rd, int i)
{
  const RX_Opcode_Operand *o = rd->op + i;

  switch (o->type)
  {
    case RX_Operand_DR_Register: /* DRn */
	case RX_Operand_DRH_Register:
	case RX_Operand_DRL_Register:
      return get_reg_double (o->reg);
	//TODO: this should not be here
    case RX_Operand_Immediate:  /* #addend */
      return o->addend;
    default:
      abort ();
  }
}

static int
get_op (const RX_Opcode_Decoded *rd, int i)
{
  const RX_Opcode_Operand *o = rd->op + i;
  int addr, rv = 0;

  switch (o->type)
    {
    case RX_Operand_None:
      rx_abort ();

    case RX_Operand_Immediate:	/* #addend */
      return o->addend;

    case RX_Operand_Register:	/* Rn */
      RL (o->reg);
      rv = get_reg (o->reg);
      break;

    case RX_Operand_Predec:	/* [-Rn] */
      put_reg (o->reg, get_reg (o->reg) - size2bytes[o->size]);
      ATTRIBUTE_FALLTHROUGH;
    case RX_Operand_Postinc:	/* [Rn+] */
    case RX_Operand_Zero_Indirect:	/* [Rn + 0] */
    case RX_Operand_Indirect:	/* [Rn + addend] */
    case RX_Operand_TwoReg:	/* [Rn + scale * R2] */
#ifdef CYCLE_ACCURATE
      RL (o->reg);
      if (o->type == RX_Operand_TwoReg)
	RL (rd->op[2].reg);
      regs.rt = -1;
      if (regs.m2m == M2M_BOTH)
	{
	  tprintf("src memory stall\n");
#ifdef WITH_PROFILE
	  memory_stalls ++;
#endif
	  regs.cycle_count ++;
	  regs.m2m = 0;
	}

      memory_source = 1;
#endif

      if (o->type == RX_Operand_TwoReg)
	addr = get_reg (o->reg) * size2bytes[rd->size] + get_reg (rd->op[2].reg);
      else
	addr = get_reg (o->reg) + o->addend;

      switch (o->size)
	{
	default:
	case RX_AnySize:
	  rx_abort ();

	case RX_Byte: /* undefined extension */
	case RX_UByte:
	case RX_SByte:
	  rv = mem_get_qi (addr);
	  break;

	case RX_Word: /* undefined extension */
	case RX_UWord:
	case RX_SWord:
	  rv = mem_get_hi (addr);
	  break;

	case RX_3Byte:
	  rv = mem_get_psi (addr);
	  break;

	case RX_Long:
	  rv = mem_get_si (addr);
	  break;
	}

      if (o->type == RX_Operand_Postinc)
	put_reg (o->reg, get_reg (o->reg) + size2bytes[o->size]);

      break;

    case RX_Operand_Condition:	/* eq, gtu, etc */
      return condition_true (o->reg);

    case RX_Operand_Flag:	/* [UIOSZC] */
      return (regs.r_psw & (1 << o->reg)) ? 1 : 0;

	case RX_Operand_DFPU_Condition:
	  return o->reg;
    }

  /* if we've gotten here, we need to clip/extend the value according
     to the size.  */
  switch (o->size)
    {
    default:
    case RX_AnySize:
      rx_abort ();

    case RX_Byte: /* undefined extension */
      rv |= 0xdeadbe00; /* keep them honest */
      break;

    case RX_UByte:
      rv &= 0xff;
      break;

    case RX_SByte:
      rv = sign_ext (rv, 8);
      break;

    case RX_Word: /* undefined extension */
      rv |= 0xdead0000; /* keep them honest */
      break;

    case RX_UWord:
      rv &=  0xffff;
      break;

    case RX_SWord:
      rv = sign_ext (rv, 16);
      break;

    case RX_3Byte:
      rv &= 0xffffff;
      break;

    case RX_Long:
      break;
    }
  return rv;
}

static void
put_op_72 (const RX_Opcode_Decoded *rd, int i, long long *v)
{
	const RX_Opcode_Operand *o = rd->op + i;

	switch (o->type)
	{
	case RX_Operand_Register:	/* An */
		put_reg72 (o->reg, (unsigned long long *)v);
		break;
	default:
		abort ();
	}
}

static void
put_op_64 (const RX_Opcode_Decoded *rd, int i, long long v)
{
	const RX_Opcode_Operand *o = rd->op + i;
	DI v72[2] = {(unsigned long long)v, 0};
	int addr;

	switch (o->type)
	{
	case RX_Operand_Register:	/* An */
		put_reg72(o->reg, v72);
		break;
	case RX_Operand_Zero_Indirect:
	case RX_Operand_Indirect:
		addr = get_reg (o->reg) + o->addend;
		mem_put_si (addr, v & 0xFFFFFFFF);
		mem_put_si (addr + 4, (v >> 32) & 0xFFFFFFFF);
		break;
	default:
		abort ();
	}
}

static void
put_op_double (const RX_Opcode_Decoded *rd, int i, long long v)
{
  const RX_Opcode_Operand *o = rd->op + i;

  switch (o->type)
  {
  case RX_Operand_DR_Register: /* DRn */
  case RX_Operand_DRH_Register:
  case RX_Operand_DRL_Register:
    put_reg_double (o->reg, (unsigned long long)v);
    break;
  default:
    abort ();
  }
}

static void
put_op (const RX_Opcode_Decoded *rd, int i, int v)
{
  const RX_Opcode_Operand *o = rd->op + i;
  int addr;

  switch (o->size)
    {
    default:
    case RX_AnySize:
      if (o->type != RX_Operand_Register)
	rx_abort ();
      break;

    case RX_Byte: /* undefined extension */
      v |= 0xdeadbe00; /* keep them honest */
      break;

    case RX_UByte:
      v &= 0xff;
      break;

    case RX_SByte:
      v = sign_ext (v, 8);
      break;

    case RX_Word: /* undefined extension */
      v |= 0xdead0000; /* keep them honest */
      break;

    case RX_UWord:
      v &=  0xffff;
      break;

    case RX_SWord:
      v = sign_ext (v, 16);
      break;

    case RX_3Byte:
      v &= 0xffffff;
      break;

    case RX_Long:
      break;
    }

  switch (o->type)
    {
    case RX_Operand_None:
      /* Opcodes like TST and CMP use this.  */
      break;

    case RX_Operand_Immediate:	/* #addend */
    case RX_Operand_Condition:	/* eq, gtu, etc */
      rx_abort ();

    case RX_Operand_Register:	/* Rn */
      put_reg (o->reg, v);
      RLD (o->reg);
      break;

    case RX_Operand_Predec:	/* [-Rn] */
      put_reg (o->reg, get_reg (o->reg) - size2bytes[o->size]);
      ATTRIBUTE_FALLTHROUGH;
    case RX_Operand_Postinc:	/* [Rn+] */
    case RX_Operand_Zero_Indirect:	/* [Rn + 0] */
    case RX_Operand_Indirect:	/* [Rn + addend] */
    case RX_Operand_TwoReg:	/* [Rn + scale * R2] */

#ifdef CYCLE_ACCURATE
      if (regs.m2m == M2M_BOTH)
	{
	  tprintf("dst memory stall\n");
	  regs.cycle_count ++;
#ifdef WITH_PROFILE
	  memory_stalls ++;
#endif
	  regs.m2m = 0;
	}
      memory_dest = 1;
#endif

      if (o->type == RX_Operand_TwoReg)
	addr = get_reg (o->reg) * size2bytes[rd->size] + get_reg (rd->op[2].reg);
      else
	addr = get_reg (o->reg) + o->addend;

      switch (o->size)
	{
	default:
	case RX_AnySize:
	  rx_abort ();

	case RX_Byte: /* undefined extension */
	case RX_UByte:
	case RX_SByte:
	  mem_put_qi (addr, v);
	  break;

	case RX_Word: /* undefined extension */
	case RX_UWord:
	case RX_SWord:
	  mem_put_hi (addr, v);
	  break;

	case RX_3Byte:
	  mem_put_psi (addr, v);
	  break;

	case RX_Long:
	  mem_put_si (addr, v);
	  break;
	}

      if (o->type == RX_Operand_Postinc)
	put_reg (o->reg, get_reg (o->reg) + size2bytes[o->size]);

      break;

    case RX_Operand_Flag:	/* [UIOSZC] */
      if (v)
	regs.r_psw |= (1 << o->reg);
      else
	regs.r_psw &= ~(1 << o->reg);
      break;
    }
}

#define PD(x) put_op (opcode, 0, x)
#define PS(x) put_op (opcode, 1, x)
#define PS2(x) put_op (opcode, 2, x)
#define GD() get_op (opcode, 0)
#define GS() get_op (opcode, 1)
#define GS2() get_op (opcode, 2)
#define GS3() get_op (opcode, 3)
#define GS4() get_op (opcode, 4)
#define DSZ() size2bytes[opcode->op[0].size]
#define SSZ() size2bytes[opcode->op[0].size]
#define S2SZ() size2bytes[opcode->op[0].size]

#define GD_72() get_op_72 (opcode, 0)
#define GD_DB() get_op_double (opcode, 0)
#define GD_64() get_op_64 (opcode, 0)
#define GS_72() get_op_72 (opcode, 1)
#define GS_64() get_op_64 (opcode, 1)
#define GS_DB() get_op_double (opcode, 1)
#define GS2_DB() get_op_double (opcode, 2)
#define PD_72(x) put_op_72 (opcode, 0, x)
#define PD_64(x) put_op_64 (opcode, 0, x)
#define PD_DB(x) put_op_double (opcode, 0, x)

/* "Universal" sources.  */
#define US1() ((opcode->op[2].type == RX_Operand_None) ? GD() : GS())
#define US2() ((opcode->op[2].type == RX_Operand_None) ? GS() : GS2())

#define CONTEXT 92

static void
push_context(int src)
{
  int v;
  int offset = 0;
  //TODO: change to error
  src = (src >= 255)? 255 : src;

  tprintf("Saving to context memory location: %d\n", src);
  /* push regs from r1 to r15 to context memory */
  for (v = 1; v <= 15; v++)
  {
    tprintf("save:%d %d\n", src, offset);
	mem_put_context_si (src, offset, get_reg(v));
	offset += 4;
  }

  /* push usp and fpsw to context memory */
  mem_put_context_si (src, offset, get_reg(usp));
  offset += 4;
  mem_put_context_si (src, offset, get_reg(fpsw));
  offset += 4;

  /* push acc0 and acc1 to context memory */
  mem_put_context_acc (src, offset, get_reg72(acc0));
  offset += 12;
  mem_put_context_acc (src, offset, get_reg72(acc1));
}

static void
push(int val)
{
  int rsp = get_reg (sp);
  rsp -= 4;
  put_reg (sp, rsp);
  mem_put_si (rsp, val);
}

static void
push_double(unsigned long long val)
{
  push (val >> 32);
  push (val & 0xFFFFFFFF);
}

/* Just like the above, but tag the memory as "pushed pc" so if anyone
   tries to write to it, it will cause an error.  */
static void
pushpc(int val)
{
  int rsp = get_reg (sp);
  rsp -= 4;
  put_reg (sp, rsp);
  mem_put_si (rsp, val);
  mem_set_content_range (rsp, rsp+3, MC_PUSHED_PC);
}

static void
pop_context(int src)
{
  int v;
  int offset = 0;
  int content;
  long long * contentACC;
  //TODO: change to error
  src = (src > 255)? 255 : src;

  tprintf("Restoring from context memory location: %d\n", src);
  /* pop regs from r1 to r15 to context memory */
  for (v = 1; v <= 15; v++)
  {
    content = mem_get_context_si (src, offset);
    offset += 4;
    put_reg (v, content);
  }

  /* pop usp and fpsw to context memory */
  content = mem_get_context_si (src, offset);
  offset += 4;
  put_reg (usp, content);

  content = mem_get_context_si (src, offset);
  offset += 4;
  put_reg (fpsw, content);

  /* pop acc0 and acc1 to context memory */
  contentACC = mem_get_context_acc (src, offset);
  offset += 4;
  put_reg72 (acc0, contentACC);

  contentACC = mem_get_context_acc (src, offset);
  put_reg72 (acc1, contentACC);
}

static int
pop (void)
{
  int rv;
  int rsp = get_reg (sp);
  rv = mem_get_si (rsp);
  rsp += 4;
  put_reg (sp, rsp);
  return rv;
}

static unsigned long long
pop_double()
{
  return (pop() & 0x00000000ffffffff) | ((unsigned long long)pop() << 32);
}

static int
poppc (void)
{
  int rv;
  int rsp = get_reg (sp);
  if (mem_get_content_type (rsp) != MC_PUSHED_PC)
    execution_error (SIM_ERR_CORRUPT_STACK, rsp);
  rv = mem_get_si (rsp);
  mem_set_content_range (rsp, rsp+3, MC_UNINIT);
  rsp += 4;
  put_reg (sp, rsp);
  return rv;
}

#define MATH_OP(vop,c)				\
{ \
  umb = US2(); \
  uma = US1(); \
  ll = (unsigned long long) uma vop (unsigned long long) umb vop c; \
  tprintf ("0x%x " #vop " 0x%x " #vop " 0x%x = 0x%llx\n", uma, umb, c, ll); \
  ma = sign_ext (uma, DSZ() * 8);					\
  mb = sign_ext (umb, DSZ() * 8);					\
  sll = (long long) ma vop (long long) mb vop c; \
  tprintf ("%d " #vop " %d " #vop " %d = %lld\n", ma, mb, c, sll); \
  set_oszc (sll, DSZ(), (long long) ll > ((1 vop 1) ? (long long) b2mask[DSZ()] : (long long) -1)); \
  PD (sll); \
  E (1);    \
}

#define LOGIC_OP(vop) \
{ \
  mb = US2(); \
  ma = US1(); \
  v = ma vop mb; \
  tprintf("0x%x " #vop " 0x%x = 0x%x\n", ma, mb, v); \
  set_sz (v, DSZ()); \
  PD(v); \
  E (1); \
}

#define SHIFT_OP(val, type, count, OP, carry_mask)	\
{ \
  int i, c=0; \
  count = US2() & 31; \
  val = (type)US1();				\
  tprintf("%lld " #OP " %d\n", val, count); \
  for (i = 0; i < count; i ++) \
    { \
      c = val & carry_mask; \
      val OP 1; \
    } \
  set_oszc (val, 4, c); \
  PD (val); \
}

typedef union {
  int i;
  float f;
} FloatInt;

ATTRIBUTE_UNUSED
static inline int
float2int (float f)
{
  FloatInt fi;
  fi.f = f;
  return fi.i;
}

static inline float
int2float (int i)
{
  FloatInt fi;
  fi.i = i;
  return fi.f;
}

static int
fop_fadd (fp_t s1, fp_t s2, fp_t *d)
{
  *d = rxfp_add (s1, s2);
  return 1;
}

static int
fop_fmul (fp_t s1, fp_t s2, fp_t *d)
{
  *d = rxfp_mul (s1, s2);
  return 1;
}

static int
fop_fdiv (fp_t s1, fp_t s2, fp_t *d)
{
  *d = rxfp_div (s1, s2);
  return 1;
}

static int
fop_fsub (fp_t s1, fp_t s2, fp_t *d)
{
  *d = rxfp_sub (s1, s2);
  return 1;
}

static int
dop_dmul (dp_t s1, dp_t s2, dp_t *d)
{
  // Must call the rxdp function first
  // so flags have a chance to be raised
  dp_t temp = rxdp_mul(s1, s2);

  //If undeflow flag is raised, handle things accordingly
  //Else do the regular thing
  if(regs.r_dpsw & DPSWBITS_DFU) {
    // If DDN is 1, return 0
    // Else, don't change the dest value, and raise unimplm processing
    if(regs.r_dpsw & DPSWBITS_DDN) {
      *d = 0;
    } else {
      regs.r_dpsw |= DPSWBITS_DCE;
      return 0;
    }
  } else {
    *d = temp;
  }

  return 1;
}

static int
dop_ddiv (dp_t s1, dp_t s2, dp_t *d)
{
  // Must call the rxdp function first
  // so flags have a chance to be raised
  dp_t temp = rxdp_div(s1, s2);

  //If undeflow flag is raised, handle things accordingly
  //Else do the regular thing
  if(regs.r_dpsw & DPSWBITS_DFU) {
    // If DDN is 1, return 0
    // Else, don't change the dest value, and raise unimplm processing
    if(regs.r_dpsw & DPSWBITS_DDN) {
      *d = 0;
    } else {
      regs.r_dpsw |= DPSWBITS_DCE;
      tprintf("[SIM] DDN no dest write\n", *d);
      return 0;
    }
  } else {
    *d = temp;
  }

  return 1;
}

static int
dop_dsub (dp_t s1, dp_t s2, dp_t *d)
{
  // Must call the rxdp function first
  // so flags have a chance to be raised
  dp_t temp = rxdp_sub(s1, s2);

  //If undeflow flag is raised, handle things accordingly
  //Else do the regular thing
  if(regs.r_dpsw & DPSWBITS_DFU) {
    // If DDN is 1, return 0
    // Else, don't change the dest value, and raise unimplm processing
    if(regs.r_dpsw & DPSWBITS_DDN) {
      *d = 0;
    } else {
      regs.r_dpsw |= DPSWBITS_DCE;
      return 0;
    }
  } else {
    *d = temp;
  }

  return 1;
}

static int
dop_dadd (dp_t s1, dp_t s2, dp_t *d)
{
  // Must call the rxdp function first
  // so flags have a chance to be raised
  dp_t temp = rxdp_add(s1, s2);
  //If undeflow flag is raised, handle things accordingly
  //Else do the regular thing
  if(regs.r_dpsw & DPSWBITS_DFU) {
    // If DDN is 1, return 0
    // Else, don't change the dest value, and raise unimplm processing
    if(regs.r_dpsw & DPSWBITS_DDN) {
      *d = 0;
    } else {
      regs.r_dpsw |= DPSWBITS_DCE;
      return 0;
    }
  } else {
    *d = temp;
  }
  return 1;
}

#define FPPENDING() (regs.r_fpsw & (FPSWBITS_CE | (FPSWBITS_FMASK & (regs.r_fpsw << FPSW_EFSH))))
#define FPCLEAR() regs.r_fpsw &= FPSWBITS_CLEAR
#define FPCHECK() \
  if (FPPENDING()) \
    return do_fp_exception (opcode_pc)

#define FLOAT_OP(func) \
{ \
  int do_store;   \
  fp_t fa, fb, fc; \
  FPCLEAR(); \
  fb = GS (); \
  fa = GD (); \
  do_store = fop_##func (fa, fb, &fc); \
  tprintf("%g " #func " %g = %g %08x\n", int2float(fa), int2float(fb), int2float(fc), fc); \
  FPCHECK(); \
  if (do_store) \
    PD (fc);	\
  mb = 0; \
  if ((fc & 0x80000000UL) != 0) \
    mb |= FLAGBIT_S; \
  if ((fc & 0x7fffffffUL) == 0)			\
    mb |= FLAGBIT_Z; \
  set_flags (FLAGBIT_S | FLAGBIT_Z, mb); \
}

#define FLOAT_OP2(func) \
{ \
  int do_store;   \
  fp_t fa, fb, fc; \
  FPCLEAR(); \
  fb = GS (); \
  fa = GS2  (); \
  do_store = fop_##func (fa, fb, &fc); \
  tprintf("%g " #func " %g = %g %08x\n", int2float(fa), int2float(fb), int2float(fc), fc); \
  FPCHECK(); \
  if (do_store) \
    PD (fc);	\
  mb = 0; \
  if ((fc & 0x80000000UL) != 0) \
    mb |= FLAGBIT_S; \
  if ((fc & 0x7fffffffUL) == 0)			\
    mb |= FLAGBIT_Z; \
  set_flags (FLAGBIT_S | FLAGBIT_Z, mb); \
}

#define DOUBLE_OP2(func) \
{ \
  int do_store;  \
  dp_t da, db, dc;   \
  db = GS2_DB ();  \
  da = GS_DB ();  \
  do_store = dop_##func (da, db, &dc);  \
  if (do_store)  \
    PD_DB (dc); \
}

#define carry (FLAG_C ? 1 : 0)

static struct {
  unsigned long vaddr;
  const char *str;
  int signal;
} exception_info[] = {
  { 0xFFFFFFD0UL, "priviledged opcode", SIGILL },
  { 0xFFFFFFD4UL, "access violation", SIGSEGV },
  { 0xFFFFFFDCUL, "undefined opcode", SIGILL },
  { 0xFFFFFFE4UL, "floating point", SIGFPE }
};
#define EX_PRIVILEDGED	0
#define EX_ACCESS	1
#define EX_UNDEFINED	2
#define EX_FLOATING	3
#define EXCEPTION(n)  \
  return generate_exception (n, opcode_pc)

#define PRIVILEDGED() \
  if (FLAG_PM) \
    EXCEPTION (EX_PRIVILEDGED)

static int
generate_exception (unsigned long type, SI opcode_pc)
{
  SI old_psw, old_pc, new_pc;

  new_pc = mem_get_si (exception_info[type].vaddr);
  /* 0x00020000 is the value used to initialise the known
     exception vectors (see rx.ld), but it is a reserved
     area of memory so do not try to access it, and if the
     value has not been changed by the program then the
     vector has not been installed.  */
  if (new_pc == 0 || new_pc == 0x00020000)
    {
      if (rx_in_gdb)
	return RX_MAKE_STOPPED (exception_info[type].signal);

      fprintf(stderr, "Unhandled %s exception at pc = %#lx\n",
	      exception_info[type].str, (unsigned long) opcode_pc);
      if (type == EX_FLOATING)
	{
	  int mask = FPPENDING ();
	  fprintf (stderr, "Pending FP exceptions:");
	  if (mask & FPSWBITS_FV)
	    fprintf(stderr, " Invalid");
	  if (mask & FPSWBITS_FO)
	    fprintf(stderr, " Overflow");
	  if (mask & FPSWBITS_FZ)
	    fprintf(stderr, " Division-by-zero");
	  if (mask & FPSWBITS_FU)
	    fprintf(stderr, " Underflow");
	  if (mask & FPSWBITS_FX)
	    fprintf(stderr, " Inexact");
	  if (mask & FPSWBITS_CE)
	    fprintf(stderr, " Unimplemented");
	  fprintf(stderr, "\n");
	}
      return RX_MAKE_EXITED (1);
    }

  tprintf ("Triggering %s exception\n", exception_info[type].str);

  old_psw = regs.r_psw;
  regs.r_psw &= ~ (FLAGBIT_I | FLAGBIT_U | FLAGBIT_PM);
  old_pc = opcode_pc;
  regs.r_pc = new_pc;
  pushpc (old_psw);
  pushpc (old_pc);
  return RX_MAKE_STEPPED ();
}

void
generate_access_exception (void)
{
  int rv;

  rv = generate_exception (EX_ACCESS, regs.r_pc);
  if (RX_EXITED (rv))
    longjmp (decode_jmp_buf, rv);
}

static int
do_fp_exception (unsigned long opcode_pc)
{
  while (FPPENDING())
    EXCEPTION (EX_FLOATING);
  return RX_MAKE_STEPPED ();
}

static int
op_is_memory (const RX_Opcode_Decoded *rd, int i)
{
  switch (rd->op[i].type)
    {
    case RX_Operand_Predec:
    case RX_Operand_Postinc:
    case RX_Operand_Indirect:
      return 1;
    default:
      return 0;
    }
}
#define OM(i) op_is_memory (opcode, i)

#define DO_RETURN(x) { longjmp (decode_jmp_buf, x); }

int
decode_opcode (void)
{
  unsigned int uma=0, umb=0;
  int ma=0, mb=0;
  int opcode_size, v;
  unsigned long tmp1, tmp2;
  signed long long ltmp1, ltmp2, ltmp3;
  unsigned long long ll, dins, da, db;
  long long sll;
  long long *sll72;
  unsigned long opcode_pc;
  RX_Data rx_data;
  const RX_Opcode_Decoded *opcode;
#ifdef WITH_PROFILE
  unsigned long long prev_cycle_count;
#endif
#ifdef CYCLE_ACCURATE
  unsigned int tx;
#endif

#ifdef WITH_PROFILE
  prev_cycle_count = regs.cycle_count;
#endif

#ifdef CYCLE_ACCURATE
  memory_source = 0;
  memory_dest = 0;
#endif

  rx_cycles ++;

  maybe_get_mem_page (regs.r_pc);

  opcode_pc = regs.r_pc;

  /* Note that we don't word-swap this point, there's no point.  */
  if (decode_cache_base[opcode_pc] == NULL)
    {
      RX_Opcode_Decoded *opcode_w;
      rx_data.addr = opcode_pc;
      opcode_w = decode_cache_base[opcode_pc] = calloc (1, sizeof (RX_Opcode_Decoded));
      opcode_size = rx_decode_opcode (opcode_pc, opcode_w,
				      rx_get_byte, &rx_data, bfd_mach_rx_v3_dfpu);
      opcode = opcode_w;
    }
  else
    {
      opcode = decode_cache_base[opcode_pc];
      opcode_size = opcode->n_bytes;
    }

#ifdef CYCLE_ACCURATE
  if (branch_alignment_penalty)
    {
      if ((regs.r_pc ^ (regs.r_pc + opcode_size - 1)) & ~7)
	{
	  tprintf("1 cycle branch alignment penalty\n");
	  cycles (branch_alignment_penalty);
#ifdef WITH_PROFILE
	  branch_alignment_stalls ++;
#endif
	}
      branch_alignment_penalty = 0;
    }
#endif

  regs.r_pc += opcode_size;

  rx_flagmask = opcode->flags_s;
  rx_flagand = ~(int)opcode->flags_0;
  rx_flagor = opcode->flags_1;

  switch (opcode->id)
    {
    case RXO_abs:
      sll = GS ();
      tprintf("|%lld| = ", sll);
      if (sll < 0)
	sll = -sll;
      tprintf("%lld\n", sll);
      PD (sll);
      set_osz (sll, 4);
      E (1);
      break;

    case RXO_adc:
      MATH_OP (+,carry);
      break;

    case RXO_add:
      MATH_OP (+,0);
      break;

    case RXO_and:
      LOGIC_OP (&);
      break;

    case RXO_bclr:
      ma = GD ();
      mb = GS ();
      if (opcode->op[0].type == RX_Operand_Register)
	mb &= 0x1f;
      else
	mb &= 0x07;
      ma &= ~(1 << mb);
      PD (ma);
      EBIT;
      break;

    case RXO_bmcc:
      ma = GD ();
      mb = GS ();
      if (opcode->op[0].type == RX_Operand_Register)
	mb &= 0x1f;
      else
	mb &= 0x07;
      if (GS2 ())
	ma |= (1 << mb);
      else
	ma &= ~(1 << mb);
      PD (ma);
      EBIT;
      break;

    case RXO_bnot:
      ma = GD ();
      mb = GS ();
      if (opcode->op[0].type == RX_Operand_Register)
	mb &= 0x1f;
      else
	mb &= 0x07;
      ma ^= (1 << mb);
      PD (ma);
      EBIT;
      break;

    case RXO_branch:
      if (opcode->op[1].type == RX_Operand_None || GS())
	{
#ifdef CYCLE_ACCURATE
	  SI old_pc = regs.r_pc;
	  int delta;
#endif
	  regs.r_pc = GD();
#ifdef CYCLE_ACCURATE
	  delta = regs.r_pc - old_pc;
	  if (delta >= 0 && delta < 16
	      && opcode_size > 1)
	    {
	      tprintf("near forward branch bonus\n");
	      cycles (2);
	    }
	  else
	    {
	      cycles (3);
	      branch_alignment_penalty = 1;
	    }
#ifdef WITH_PROFILE
	  branch_stalls ++;
#endif
#endif
	}
#ifdef CYCLE_ACCURATE
      else
	cycles (1);
#endif
      break;

    case RXO_branchrel:
      if (opcode->op[1].type == RX_Operand_None || GS())
	{
	  int delta = GD();
	  regs.r_pc = opcode_pc + delta;
#ifdef CYCLE_ACCURATE
	  /* Note: specs say 3, chip says 2.  */
	  if (delta >= 0 && delta < 16
	      && opcode_size > 1)
	    {
	      tprintf("near forward branch bonus\n");
	      cycles (2);
	    }
	  else
	    {
	      cycles (3);
	      branch_alignment_penalty = 1;
	    }
#ifdef WITH_PROFILE
	  branch_stalls ++;
#endif
#endif
	}
#ifdef CYCLE_ACCURATE
      else
	cycles (1);
#endif
      break;

    case RXO_brk:
      {
	int old_psw = regs.r_psw;
	if (rx_in_gdb)
	  DO_RETURN (RX_MAKE_HIT_BREAK ());
	if (regs.r_intb == 0)
	  {
	    tprintf("BREAK hit, no vector table.\n");
	    DO_RETURN (RX_MAKE_EXITED(1));
	  }
	regs.r_psw &= ~(FLAGBIT_I | FLAGBIT_U | FLAGBIT_PM);
	pushpc (old_psw);
	pushpc (regs.r_pc);
	regs.r_pc = mem_get_si (regs.r_intb);
	cycles(6);
      }
      break;

    case RXO_bset:
      ma = GD ();
      mb = GS ();
      if (opcode->op[0].type == RX_Operand_Register)
	mb &= 0x1f;
      else
	mb &= 0x07;
      ma |= (1 << mb);
      PD (ma);
      EBIT;
      break;

    case RXO_btst:
      ma = GS ();
      mb = GS2 ();
      if (opcode->op[1].type == RX_Operand_Register)
	mb &= 0x1f;
      else
	mb &= 0x07;
      umb = ma & (1 << mb);
      set_zc (! umb, umb);
      EBIT;
      break;

    case RXO_clrpsw:
      v = 1 << opcode->op[0].reg;
      if (FLAG_PM
	  && (v == FLAGBIT_I
	      || v == FLAGBIT_U))
      {
        cycles (1);
	    break;
      }
      regs.r_psw &= ~v;
      cycles (1);
      break;

    case RXO_div: /* d = d / s */
      ma = GS();
      mb = GD();
      tprintf("%d / %d = ", mb, ma);
      if (ma == 0 || (ma == -1 && (unsigned int) mb == 0x80000000))
	{
	  tprintf("#NAN\n");
	  set_flags (FLAGBIT_O, FLAGBIT_O);
	  cycles (3);
	}
      else
	{
	  v = mb/ma;
	  tprintf("%d\n", v);
	  set_flags (FLAGBIT_O, 0);
	  PD (v);
	  div_cycles (mb, ma);
	}
      break;

    case RXO_divu: /* d = d / s */
      uma = GS();
      umb = GD();
      tprintf("%u / %u = ", umb, uma);
      if (uma == 0)
	{
	  tprintf("#NAN\n");
	  set_flags (FLAGBIT_O, FLAGBIT_O);
	  cycles (2);
	}
      else
	{
	  v = umb / uma;
	  tprintf("%u\n", v);
	  set_flags (FLAGBIT_O, 0);
	  PD (v);
	  divu_cycles (umb, uma);
	}
      break;

    case RXO_emul:
      ma = GD ();
      mb = GS ();
      sll = (long long)ma * (long long)mb;
      tprintf("%d * %d = %lld\n", ma, mb, sll);
      put_reg (opcode->op[0].reg, sll);
      put_reg (opcode->op[0].reg + 1, sll >> 32);
      E2;
      break;

    case RXO_emulu:
      uma = GD ();
      umb = GS ();
      ll = (long long)uma * (long long)umb;
      tprintf("%#x * %#x = %#llx\n", uma, umb, ll);
      put_reg (opcode->op[0].reg, ll);
      put_reg (opcode->op[0].reg + 1, ll >> 32);
      E2;
      break;

    case RXO_fadd:
      if(opcode->op[2].type != RX_Operand_None)
      {
    	  FLOAT_OP2 (fadd);
          E (2);
      }
      else
      {
    	  FLOAT_OP (fadd);
          E (4);
      }
      break;

    case RXO_fcmp:
      ma = GD();
      mb = GS();
      FPCLEAR ();
      rxfp_cmp (ma, mb);
      FPCHECK ();
      E (1);
      break;

    case RXO_fdiv:
      FLOAT_OP (fdiv);
      E (16);
      break;

    case RXO_fmul:
      if(opcode->op[2].type != RX_Operand_None)
      {
    	  FLOAT_OP2 (fmul);
    	  E (2);
      }
      else
      {
    	  FLOAT_OP (fmul);
    	  E (3);
      }
      break;

    case RXO_rtfi:
      PRIVILEDGED ();
      regs.r_psw = regs.r_bpsw;
      regs.r_pc = regs.r_bpc;
      put_reg(libit, 0);
#ifdef CYCLE_ACCURATE
      regs.fast_return = 0;
      cycles(3);
#endif
      break;

    case RXO_rstr:
      pop_context (GS());
	  cycles (1);//TODO: update
      break;

    case RXO_save:
      push_context (GS());
	  cycles (1);//TODO: update
      break;

    case RXO_fsub:
      if(opcode->op[2].type != RX_Operand_None)
      {
    	  FLOAT_OP2 (fsub);
    	  E (2);
      }
      else
      {
    	  FLOAT_OP (fsub);
    	  E (4);
      }
      break;

    case RXO_ftoi:
      ma = GS ();
      FPCLEAR ();
      mb = rxfp_ftoi (ma, FPRM_ZERO);
      FPCHECK ();
      PD (mb);
      tprintf("(int) %g = %d\n", int2float(ma), mb);
      set_sz (mb, 4);
      E (2);
      break;

    case RXO_int:
      v = GS ();
      if (v == 255)
	{
	  int rc = rx_syscall (regs.r[5]);
	  if (! RX_STEPPED (rc))
	    DO_RETURN (rc);
	}
      else
	{
	  int old_psw = regs.r_psw;
	  regs.r_psw &= ~(FLAGBIT_I | FLAGBIT_U | FLAGBIT_PM);
	  pushpc (old_psw);
	  pushpc (regs.r_pc);
	  regs.r_pc = mem_get_si (regs.r_intb + 4 * v);
	}
      cycles (6);
      break;

    case RXO_itof:
      ma = GS ();
      FPCLEAR ();
      mb = rxfp_itof (ma, regs.r_fpsw);
      FPCHECK ();
      tprintf("(float) %d = %x\n", ma, mb);
      PD (mb);
      set_sz (ma, 4);
      E (2);
      break;

    case RXO_utof:
      uma = GS ();
      FPCLEAR ();
      umb = rxfp_utof (uma, regs.r_fpsw);
      FPCHECK ();
      tprintf("(float) %d = %x\n", uma, umb);
      PD (umb);
      set_sz (uma, 4);
      if (opcode->op[1].type == RX_Operand_Register)
      {
    	  E (2);
      }
      else
      {
    	  E (2);
      }
      break;

    case RXO_ftou:
      uma = GS ();
      FPCLEAR ();
      umb = rxfp_ftou (uma);
      FPCHECK ();
      tprintf("(unsigned) %x = %d\n", uma, umb);
      PD (umb);
      set_sz (uma, 4);
      if (opcode->op[1].type == RX_Operand_Register)
      {
    	  E (2);
      }
      else
      {
    	  E (2);
      }
      break;

    case RXO_fsqrt:
      uma = GS ();
      FPCLEAR ();
      umb = rxfp_fsqrt (uma);
      FPCHECK ();
      PD (umb);
      set_sz (uma, 4);
      if (opcode->op[1].type == RX_Operand_Register)
      {
    	  E (16);
      }
      else
      {
    	  E (18);
      }
      break;

    case RXO_jsr:
    case RXO_jsrrel:
      {
#ifdef CYCLE_ACCURATE
	int delta;
	regs.m2m = 0;
#endif
	v = GD ();
#ifdef CYCLE_ACCURATE
	regs.link_register = regs.r_pc;
#endif
	pushpc (get_reg (pc));
	if (opcode->id == RXO_jsrrel)
	  v += regs.r_pc;
#ifdef CYCLE_ACCURATE
	delta = v - regs.r_pc;
#endif
	put_reg (pc, v);
#ifdef CYCLE_ACCURATE
	/* Note: docs say 3, chip says 2 */
	if (delta >= 0 && delta < 16)
	  {
	    tprintf ("near forward jsr bonus\n");
	    cycles (2);
	  }
	else
	  {
	    branch_alignment_penalty = 1;
	    cycles (3);
	  }
	regs.fast_return = 1;
#endif
      }
      break;

    case RXO_maclh:
      ma = (short) GS();
      mb = (short) (GS2() >> 16);
      sll72 = GD_72();
      sll = sll72[0];
      ltmp1 = (((long long) ma) * ((long long) mb)) << 16;
      ltmp2 = (ltmp1 & 0x8000000000000000LL) ? 0xFFFFFFFFFFFFFFFFLL : 0x0LL;
      sll72[0] += ltmp1;
      ltmp3 = ((unsigned long long)sll72[0] < (unsigned long long)sll);
      sll72[1] += ltmp2 + ltmp3;
      PD_72(sll72);
      E1;
      break;

    case RXO_machi:
      ma = (short) (GS() >> 16);
      mb = (short) (GS2() >> 16);
      sll72 = GD_72();
      sll = sll72[0];
      ltmp1 = (((long long) ma) * ((long long) mb)) << 16;
      ltmp2 = (ltmp1 & 0x8000000000000000LL) ? 0xFFFFFFFFFFFFFFFFLL : 0x0LL;
      sll72[0] += ltmp1;
      ltmp3 = ((unsigned long long)sll72[0] < (unsigned long long)sll);
      sll72[1] += ltmp2 + ltmp3;
      PD_72(sll72);
      E1;
      break;

    case RXO_maclo:
      ma = (short) GS();
      mb = (short) GS2();
      sll72 = GD_72();
      sll = sll72[0];
      ltmp1 = (long long) (ma * mb) << 16;
      ltmp2 = (ltmp1 & 0x8000000000000000LL) ? 0xFFFFFFFFFFFFFFFFLL : 0x0LL;
      sll72[0] += ltmp1;
      ltmp3 = ((unsigned long long)sll72[0] < (unsigned long long)sll);
      sll72[1] += ltmp2 + ltmp3;
      PD_72(sll72);
      E1;
      break;

    case RXO_msblh:
      ma = (short) GS();
      mb = (short) (GS2() >> 16);
      sll72 = GD_72();
      sll = sll72[0];
      ltmp1 = (((long long) ma) * ((long long) mb)) << 16;
      ltmp2 = (ltmp1 & 0x8000000000000000LL) ? 0xFFFFFFFFFFFFFFFFLL : 0x0LL;
      sll72[0] -= ltmp1;
      ltmp3 = ((unsigned long long)sll72[0] > (unsigned long long)sll);
      sll72[1] -= ltmp2 + ltmp3;
      PD_72(sll72);
      E1;
      break;

    case RXO_msbhi:
      ma = (short) (GS() >> 16);
      mb = (short) (GS2() >> 16);
      sll72 = GD_72();
      sll = sll72[0];
      ltmp1 = (((long long) ma) * ((long long) mb)) << 16;
      ltmp2 = (ltmp1 & 0x8000000000000000LL) ? 0xFFFFFFFFFFFFFFFFLL : 0x0LL;
      sll72[0] -= ltmp1;
      ltmp3 = ((unsigned long long)sll72[0] > (unsigned long long)sll);
      sll72[1] -= ltmp2 + ltmp3;
      PD_72(sll72);
      E1;
      break;

    case RXO_msblo:
      ma = (short) GS();
      mb = (short) GS2();
      sll72 = GD_72();
      sll = sll72[0];
      ltmp1 = (((long long) ma) * ((long long) mb)) << 16;
      ltmp2 = (ltmp1 & 0x8000000000000000LL) >> 63;
      ltmp3 = ((unsigned long long)sll72[0] > (unsigned long long)sll);
      sll72[0] -= ltmp1;
      sll72[1] -= ltmp2 + ltmp3;
      PD_72(sll72);
      E1;
      break;

    case RXO_max:
      mb = GS();
      ma = GD();
      if (ma > mb)
	PD (ma);
      else
	PD (mb);
      E (1);
      break;

    case RXO_min:
      mb = GS();
      ma = GD();
      if (ma < mb)
	PD (ma);
      else
	PD (mb);
      E (1);
      break;

    case RXO_mov:
		/* TODO: we need to update this!!! */
      if (opcode->op[1].type == RX_Operand_Register
		&& opcode->op[1].reg == fpsw /* FPSW */)
	  {
		  v =  regs.r_fpsw;
	  }
	  else
	  {
		  v = GS ();
	  }

      if (opcode->op[1].type == RX_Operand_Register
	  && opcode->op[1].reg == pc /* PC */)
	{
	  /* Special case.  We want the address of the insn, not the
	     address of the next insn.  */
	  v = opcode_pc;
	}

      if (opcode->op[0].type == RX_Operand_Register
	  && opcode->op[0].reg == psw /* PSW */)
	{
	  /* Special case, LDC and POPC can't ever modify PM.  */
	  int pm = regs.r_psw & FLAGBIT_PM;
	  v &= ~ FLAGBIT_PM;
	  v |= pm;
	  if (pm)
	    {
	      v &= ~ (FLAGBIT_I | FLAGBIT_U | FLAGBITS_IPL);
	      v |= pm;
	    }
	}
      if (FLAG_PM)
	{
	  /* various things can't be changed in user mode.  */
	  if (opcode->op[0].type == RX_Operand_Register)
	    if (opcode->op[0].reg == psw)
	      {
		v &= ~ (FLAGBIT_I | FLAGBIT_U | FLAGBITS_IPL);
		v |= regs.r_psw & (FLAGBIT_I | FLAGBIT_U | FLAGBITS_IPL);
	      }
	  if (opcode->op[0].reg == isp /* ISP */
	      || opcode->op[0].reg == bpsw /* BPSW */
	      || opcode->op[0].reg == intb /* INTB */
	      || opcode->op[0].reg == fintv /* VCT */)
	    /* These are ignored.  */
	    break;
	}
      if (OM(0) && OM(1))
	cycles (2);
      else
	cycles (1);

      PD (v);

#ifdef CYCLE_ACCURATE
      if ((opcode->op[0].type == RX_Operand_Predec
	   && opcode->op[1].type == RX_Operand_Register)
	  || (opcode->op[0].type == RX_Operand_Postinc
	      && opcode->op[1].type == RX_Operand_Register))
	{
	  /* Special case: push reg doesn't cause a memory stall.  */
	  memory_dest = 0;
	  tprintf("push special case\n");
	}
#endif

      set_sz (v, DSZ());
      break;

    case  RXO_movli:
      v = GS();
      PD(v);
      put_reg(libit, 1);
      cycles (1);
      break;

    case  RXO_movco:
      if(get_reg(libit) == 1)
      {
    	v = GS();
    	PD(v);
    	PS(0);
      }
      else
      {
    	PS(1);
      }
      put_reg(libit, 0);
      cycles (1);
      break;

    case RXO_movbi:
      PD (GS ());
      cycles (1);
      break;

    case RXO_movbir:
      PS (GD ());
      cycles (1);
      break;

    case RXO_bfmov:
      // GS2 = slsb; GS3 = dlsb; GS4 = width; GS = src; GD = dest
	  tmp1 = (0xFFFFFFFF >> (32 - GS4())) << GS3();
      tprintf("bfmov\n");
      tprintf("slsb: %d\n", GS2());
      tprintf("dlsb: %d\n", GS3());
      tprintf("width: %d\n", GS4());
	  tprintf("tmp1: 0x%X\n", tmp1);
	  tmp2 = (GS() >> GS2()) << GS3();
	  tprintf("tmp2: 0x%X\n", tmp2);
	  PD((tmp2 & tmp1) | (GD() & (~tmp1)));
	  cycles (1);//TODO: update
	  break;

    case RXO_bfmovz:
      // GS2 = slsb; GS3 = dlsb; GS4 = width; GS = src; GD = dest
      tmp1 = (0xFFFFFFFF >> (32 - GS4())) << GS3();
      tprintf("bfmovz\n");
      tprintf("slsb: %d\n", GS2());
      tprintf("dlsb: %d\n", GS3());
      tprintf("width: %d\n", GS4());
      tprintf("src: %d\n", GS());
      tprintf("dest: %d\n", GD());
      tmp2 = (GS() >> GS2()) << GS3();
      PD((tmp2 & tmp1));
	  cycles (1);//TODO: update
	  break;

    case RXO_mul:
      v = US2 ();
      ll = (unsigned long long) US1() * (unsigned long long) v;
      PD(ll);
      E (1);
      break;

    case RXO_emula:
      ma = GS();
      mb = GS2();
      sll = (long long) ma * (long long) mb;
      PD_64(sll);
      E1;
      break;

    case RXO_emaca:
      ma = GS();
      mb = GS2();
      sll72 = GD_72();
      sll = sll72[0];
      ltmp1 = (((long long) ma) * ((long long) mb));
      ltmp2 = (ltmp1 & 0x8000000000000000LL) ? 0xFFFFFFFFFFFFFFFFLL : 0x0LL;
      sll72[0] += ltmp1;
      if((ltmp1) > 0)
      {
        ltmp3 = ((unsigned long long)sll72[0] < (unsigned long long)sll);
      }
      else
      {
        ltmp3 = ((unsigned long long)sll72[0] > (unsigned long long)sll);
      }
      sll72[1] += ltmp2 + ltmp3;
      PD_72(sll72);
      E1;
      break;

    case RXO_emsba:
      ma = GS();
      mb = GS2();
      sll72 = GD_72();
      sll = sll72[0];
      ltmp1 = (((long long) ma) * ((long long) mb));
      ltmp2 = (ltmp1 & 0x8000000000000000LL) ? 0xFFFFFFFFFFFFFFFFLL : 0x0LL;
      sll72[0] -= ltmp1;
      if((ltmp1) > 0)
      {
        ltmp3 = ((unsigned long long)sll72[0] < (unsigned long long)sll);
      }
      else
      {
        ltmp3 = ((unsigned long long)sll72[0] > (unsigned long long)sll);
      }
      sll72[1] += ltmp2 + ltmp3;
      PD_72(sll72);
      E1;
      break;

    case RXO_mullh:
      ma = (short) GS();
      mb = (short) (GS2() >> 16);
      sll = ((long long)ma) * ((long long)mb);
      sll <<= 16;
      PD_64(sll);
      E1;
      break;

    case RXO_mulhi:
      ma = (short) (GS() >> 16);
      mb = (short) (GS2() >> 16);
      sll = ((long long)ma) * ((long long)mb);
      sll <<= 16;
      PD_64(sll);
      E1;
      break;

    case RXO_mullo:
      ma = (short) GS();
      mb = (short) GS2();
      sll = ((long long)ma) * ((long long)mb);
      sll <<= 16;
      PD_64(sll);
      E1;
      break;

    case RXO_mvfachi:
      v = GS2();
      sll = GS_64();
      v = (sll << v) >> 32;
      PD (v);
      E1;
      break;

    case RXO_mvfaclo:
      v = GS2();
      sll = GS_64();
      v = sll << v;
      PD (v);
      E1;
      break;

    case RXO_mvfacmi:
      v = GS2();
      sll = GS_64();
      v = (sll << v) >> 16;
      PD (v);
      E1;
      break;

    case RXO_mvfacgu:
      v = GS2();
      sll72 = GS_72();
      v = sll72[1] << v;
      PD (v);
      E1;
      break;

    case RXO_mvtachi:
      sll = (GD_64() & 0xFFFFFFFF) | ((unsigned long long)GS() << 32);
      PD_64(sll);
      E1;
      break;

    case RXO_mvtaclo:
      sll = (GD_64() & 0xFFFFFFFF00000000) | GS();
      PD_64(sll);
      E1;
      break;

    case RXO_mvtacgu:
      sll72 = GD_72();
      sll72[1] = GS() & 0xFF;
      PD_72(sll72);
      E1;
      break;

    case RXO_mvtipl:
      regs.r_psw &= ~ FLAGBITS_IPL;
      regs.r_psw |= (GS () << FLAGSHIFT_IPL) & FLAGBITS_IPL;
      E1;
      break;

    case RXO_nop:
    case RXO_nop2:
    case RXO_nop3:
    case RXO_nop4:
    case RXO_nop5:
    case RXO_nop6:
    case RXO_nop7:
      E1;
      break;

    case RXO_or:
      LOGIC_OP (|);
      break;

    case RXO_popm:
      /* POPM cannot pop R0 (sp).  */
      if (opcode->op[1].reg == 0 || opcode->op[2].reg == 0)
	EXCEPTION (EX_UNDEFINED);
      if (opcode->op[1].reg >= opcode->op[2].reg)
	{
	  regs.r_pc = opcode_pc;
	  DO_RETURN (RX_MAKE_STOPPED (SIGILL));
	}
      for (v = opcode->op[1].reg; v <= opcode->op[2].reg; v++)
	{
	  cycles (1);
	  RLD (v);
	  put_reg (v, pop ());
	}
      break;

    case RXO_pushm:
      /* PUSHM cannot push R0 (sp).  */
      if (opcode->op[1].reg == 0 || opcode->op[2].reg == 0)
	EXCEPTION (EX_UNDEFINED);
      if (opcode->op[1].reg >= opcode->op[2].reg)
	{
	  regs.r_pc = opcode_pc;
	  return RX_MAKE_STOPPED (SIGILL);
	}
      for (v = opcode->op[2].reg; v >= opcode->op[1].reg; v--)
	{
	  RL (v);
	  push (get_reg (v));
	}
      cycles (opcode->op[2].reg - opcode->op[1].reg + 1);
      break;

    case RXO_rdacw:
      sll72 = GD_72();
      v = GS();
      /* tmp = (signed 72bit) Adest << src; */
      sll72[0] <<= v;
      sll72[1] <<= v;
      /* if (tmp > (signed 72bit) 000_0000_7FFF_0000_0000h) */
      if ((unsigned long long)sll72[0] > 0x7FFF00000000ULL)
      {
    	  /* Adest = 00_0000_7FFF_0000_0000h; */
    	  sll72[0] = 0x7FFF00000000LL;
    	  sll72[1] = 0x0;
      }
      /* else if (tmp72 < (signed 72bit) FF_FFFF_8000_0000_0000h) */
      else if((sll72[1] & 0x80) &&
    		  (((unsigned long long)sll72[1] < 0xFFFFFFFFFFFFFFFFULL) ||
    		  (((unsigned long long)sll72[1] == 0xFFFFFFFFFFFFFFFFULL)
    		  && ((unsigned long long)sll72[0] < 0xFFFF800000000000ULL))))
      {
    	  /* Adest = FF_FFFF_8000_0000_0000h;*/
    	  sll72[0] = 0xFFFF800000000000LL;
    	  sll72[1] = 0xFFLL;
      }
      else
      {
      	  /* Adest = tmp & FF_FFFF_FFFF_0000_0000h; */
       	  sll72[0] &= 0xFFFFFFFF00000000LL;
    	  sll72[1] &= 0xFFLL;
      }
      PD_72(sll72);
      E1;
      break;

    case RXO_rdacl:
      sll72 = GD_72();
      v = GS();
      /* tmp = (signed 72bit) Adest << src; */
      sll72[0] <<= v;
      sll72[1] <<= v;
      /* if (tmp > (signed 72bit) 00_7FFF_FFFF_0000_0000h)  */
      if ((unsigned long long)sll72[0] > 0x7FFFFFFF00000000ULL)
      {
    	  /* Adest = 00_7FFF_FFFF_0000_0000h;  */
       	  sll72[0] = 0x7FFFFFFF00000000LL;
       	  sll72[1] = 0x0;
      }
      /* else if (tmp < (signed 72bit) FF_8000_0000_0000_0000h) */
      else if((sll72[1] & 0x80) &&
      	  	  (((unsigned long long)sll72[1] < 0xFFFFFFFFFFFFFFFFULL) ||
          		  (((unsigned long long)sll72[1] == 0xFFFFFFFFFFFFFFFFULL)
          		  && ((unsigned long long)sll72[0] < 0x8000000000000000ULL))))
      {
       	  /* Adest = FF_8000_0000_0000_0000h; */
       	  sll72[0] = 0x8000000000000000LL;
       	  sll72[1] = 0xFFLL;
         }
      else
      {
      	  /* Adest = tmp & FF_FFFF_FFFF_0000_0000h; */
       	  sll72[0] &= 0xFFFFFFFF00000000LL;
       	  sll72[1] &= 0xFFLL;
      }
      PD_72(sll72);
      E1;
      break;

    case RXO_racw:
      sll72 = GD_72();
      v = GS();
      /* tmp = (signed 72bit) Adest << src; */
      sll72[0] <<= v;
      sll72[1] <<= v;
      /* tmp73 = (signed 73bit) tmp + 000_0000_0000_8000_0000h; */
      sll72[0] += 0x80000000LL;
      /*if overflow */
      if((unsigned long long)sll72[0] < 0x80000000ULL)
      {
    	  sll72[1]++;
      }
      /* if (tmp > (signed 72bit) 000_0000_7FFF_0000_0000h) */
      if ((unsigned long long)sll72[0] > 0x7FFF00000000ULL)
      {
    	  /* Adest = 00_0000_7FFF_0000_0000h; */
          sll72[0] = 0x7FFF00000000LL;
          sll72[1] = 0x0;
      }
      /* else if (tmp72 < (signed 72bit) FF_FFFF_8000_0000_0000h) */
      else if((sll72[1] & 0x80) &&
       		  (((unsigned long long)sll72[1] < 0xFFFFFFFFFFFFFFFFULL) ||
       		  (((unsigned long long)sll72[1] == 0xFFFFFFFFFFFFFFFFULL)
       		  && ((unsigned long long)sll72[0] < 0xFFFF800000000000ULL))))
      {
       	  /* Adest = FF_FFFF_8000_0000_0000h;*/
       	  sll72[0] = 0xFFFF800000000000LL;
       	  sll72[1] = 0xFFLL;
      }
      else
      {
       	  /* Adest = tmp & FF_FFFF_FFFF_0000_0000h; */
       	  sll72[0] &= 0xFFFFFFFF00000000LL;
      	  sll72[1] &= 0xFFLL;
      }
      PD_72(sll72);
      E1;
      break;

    case RXO_racl:
      sll72 = GD_72();
      v = GS();
      /* tmp = (signed 72bit) Adest << src; */
      sll72[0] <<= v;
      sll72[1] <<= v;
      /* tmp73 = (signed 73bit) tmp + 000_0000_0000_8000_0000h; */
      sll72[0] += 0x80000000LL;
      /*if overflow */
      if((unsigned long long)sll72[0] < 0x80000000ULL)
      {
       	  sll72[1]++;
      }
      /* if (tmp > (signed 72bit) 00_7FFF_FFFF_0000_0000h)  */
      if ((unsigned long long)sll72[0] > 0x7FFFFFFF00000000ULL)
      {
      	  /* Adest = 00_7FFF_FFFF_0000_0000h;  */
       	  sll72[0] = 0x7FFFFFFF00000000LL;
       	  sll72[1] = 0x0;
      }
      /* else if (tmp < (signed 72bit) FF_8000_0000_0000_0000h) */
      else if((sll72[1] & 0x80) &&
         	  	  (((unsigned long long)sll72[1] < 0xFFFFFFFFFFFFFFFFULL) ||
           		  (((unsigned long long)sll72[1] == 0xFFFFFFFFFFFFFFFFULL)
           		  && ((unsigned long long)sll72[0] < 0x8000000000000000ULL))))
      {
       	  /* Adest = FF_FFFF_8000_0000_0000h;*/
       	  sll72[0] = 0xFFFF800000000000LL;
       	  sll72[1] = 0xFFLL;
      }
      else
      {
       	  /* Adest = tmp & FF_FFFF_FFFF_0000_0000h; */
      	  sll72[0] &= 0xFFFFFFFF00000000LL;
      	  sll72[1] &= 0xFFLL;
      }
      PD_72(sll72);
      E1;
      break;

    case RXO_rte:
      PRIVILEDGED ();
      regs.r_pc = poppc ();
      regs.r_psw = poppc ();
      if (FLAG_PM)
	regs.r_psw |= FLAGBIT_U;
      put_reg(libit, 0);
#ifdef CYCLE_ACCURATE
      regs.fast_return = 0;
      cycles (6);
#endif
      break;

    case RXO_revl:
      uma = GS ();
      umb = (((uma >> 24) & 0xff)
	     | ((uma >> 8) & 0xff00)
	     | ((uma << 8) & 0xff0000)
	     | ((uma << 24) & 0xff000000UL));
      PD (umb);
      E1;
      break;

    case RXO_revw:
      uma = GS ();
      umb = (((uma >> 8) & 0x00ff00ff)
	     | ((uma << 8) & 0xff00ff00UL));
      PD (umb);
      E1;
      break;

    case RXO_rmpa:
      RL(4);
      RL(5);
#ifdef CYCLE_ACCURATE
      tx = regs.r[3];
#endif

      while (regs.r[3] != 0)
	{
	  long long tmp;

	  switch (opcode->size)
	    {
	    case RX_Long:
	      ma = mem_get_si (regs.r[1]);
	      mb = mem_get_si (regs.r[2]);
	      regs.r[1] += 4;
	      regs.r[2] += 4;
	      break;
	    case RX_Word:
	      ma = sign_ext (mem_get_hi (regs.r[1]), 16);
	      mb = sign_ext (mem_get_hi (regs.r[2]), 16);
	      regs.r[1] += 2;
	      regs.r[2] += 2;
	      break;
	    case RX_Byte:
	      ma = sign_ext (mem_get_qi (regs.r[1]), 8);
	      mb = sign_ext (mem_get_qi (regs.r[2]), 8);
	      regs.r[1] += 1;
	      regs.r[2] += 1;
	      break;
	    default:
	      abort ();
	    }
	  /* We do the multiply as a signed value.  */
	  sll = (long long)ma * (long long)mb;
	  tprintf("        %016llx = %d * %d\n", sll, ma, mb);
	  /* but we do the sum as unsigned, while sign extending the operands.  */
	  tmp = regs.r[4] + (sll & 0xffffffffUL);
	  regs.r[4] = tmp & 0xffffffffUL;
	  tmp >>= 32;
	  sll >>= 32;
	  tmp += regs.r[5] + (sll & 0xffffffffUL);
	  regs.r[5] = tmp & 0xffffffffUL;
	  tmp >>= 32;
	  sll >>= 32;
	  tmp += regs.r[6] + (sll & 0xffffffffUL);
	  regs.r[6] = tmp & 0xffffffffUL;
	  tprintf("%08lx\033[36m%08lx\033[0m%08lx\n",
		  (unsigned long) regs.r[6],
		  (unsigned long) regs.r[5],
		  (unsigned long) regs.r[4]);

	  regs.r[3] --;
	}
      if (regs.r[6] & 0x00008000)
	regs.r[6] |= 0xffff0000UL;
      else
	regs.r[6] &= 0x0000ffff;
      ma = (regs.r[6] & 0x80000000UL) ? FLAGBIT_S : 0;
      if (regs.r[6] != 0 && regs.r[6] != 0xffffffffUL)
	set_flags (FLAGBIT_O|FLAGBIT_S, ma | FLAGBIT_O);
      else
	set_flags (FLAGBIT_O|FLAGBIT_S, ma);
#ifdef CYCLE_ACCURATE
      switch (opcode->size)
	{
	case RX_Long:
	  cycles (6 + 4 * tx);
	  break;
	case RX_Word:
	  cycles (6 + 5 * (tx / 2) + 4 * (tx % 2));
	  break;
	case RX_Byte:
	  cycles (6 + 7 * (tx / 4) + 4 * (tx % 4));
	  break;
	default:
	  abort ();
	}
#endif
      break;

    case RXO_rolc:
      v = GD ();
      ma = v & 0x80000000UL;
      v <<= 1;
      v |= carry;
      set_szc (v, 4, ma);
      PD (v);
      E1;
      break;

    case RXO_rorc:
      uma = GD ();
      mb = uma & 1;
      uma >>= 1;
      uma |= (carry ? 0x80000000UL : 0);
      set_szc (uma, 4, mb);
      PD (uma);
      E1;
      break;

    case RXO_rotl:
      mb = GS ();
      uma = GD ();
      if (mb)
	{
	  uma = (uma << mb) | (uma >> (32-mb));
	  mb = uma & 1;
	}
      set_szc (uma, 4, mb);
      PD (uma);
      E1;
      break;

    case RXO_rotr:
      mb = GS ();
      uma = GD ();
      if (mb)
	{
	  uma = (uma >> mb) | (uma << (32-mb));
	  mb = uma & 0x80000000;
	}
      set_szc (uma, 4, mb);
      PD (uma);
      E1;
      break;

    case RXO_round:
      ma = GS ();
      FPCLEAR ();
      mb = rxfp_ftoi (ma, regs.r_fpsw);
      FPCHECK ();
      PD (mb);
      tprintf("(int) %g = %d\n", int2float(ma), mb);
      set_sz (mb, 4);
      E (2);
      break;

    case RXO_rts:
      {
#ifdef CYCLE_ACCURATE
	int cyc = 5;
#endif
	regs.r_pc = poppc ();
#ifdef CYCLE_ACCURATE
	/* Note: specs say 5, chip says 3.  */
	if (regs.fast_return && regs.link_register == regs.r_pc)
	  {
#ifdef WITH_PROFILE
	    fast_returns ++;
#endif
	    tprintf("fast return bonus\n");
	    cyc -= 2;
	  }
	cycles (cyc);
	regs.fast_return = 0;
	branch_alignment_penalty = 1;
#endif
      }
      break;

    case RXO_rtsd:
      if (opcode->op[2].type == RX_Operand_Register)
	{
	  int i;
	  /* RTSD cannot pop R0 (sp).  */
	  put_reg (0, get_reg (0) + GS() - (opcode->op[0].reg-opcode->op[2].reg+1)*4);
	  if (opcode->op[2].reg == 0)
	    EXCEPTION (EX_UNDEFINED);
#ifdef CYCLE_ACCURATE
	  tx = opcode->op[0].reg - opcode->op[2].reg + 1;
#endif
	  for (i = opcode->op[2].reg; i <= opcode->op[0].reg; i ++)
	    {
	      RLD (i);
	      put_reg (i, pop ());
	    }
	}
      else
	{
#ifdef CYCLE_ACCURATE
	  tx = 0;
#endif
	  put_reg (0, get_reg (0) + GS());
	}
      put_reg (pc, poppc());
#ifdef CYCLE_ACCURATE
      if (regs.fast_return && regs.link_register == regs.r_pc)
	{
	  tprintf("fast return bonus\n");
#ifdef WITH_PROFILE
	  fast_returns ++;
#endif
	  cycles (tx < 3 ? 3 : tx + 1);
	}
      else
	{
	  cycles (tx < 5 ? 5 : tx + 1);
	}
      regs.fast_return = 0;
      branch_alignment_penalty = 1;
#endif
      break;

    case RXO_sat:
      if (FLAG_O && FLAG_S)
	PD (0x7fffffffUL);
      else if (FLAG_O && ! FLAG_S)
	PD (0x80000000UL);
      E1;
      break;

    case RXO_satr:
      if (FLAG_O && ! FLAG_S)
	{
	  put_reg (6, 0x0);
	  put_reg (5, 0x7fffffff);
	  put_reg (4, 0xffffffff);
	}
      else if (FLAG_O && FLAG_S)
	{
	  put_reg (6, 0xffffffff);
	  put_reg (5, 0x80000000);
	  put_reg (4, 0x0);
	}
      E1;
      break;

    case RXO_sbb:
      MATH_OP (-, ! carry);
      break;

    case RXO_sccnd:
      if (GS())
	PD (1);
      else
	PD (0);
      E1;
      break;

    case RXO_scmpu:
#ifdef CYCLE_ACCURATE
      tx = regs.r[3];
#endif
      while (regs.r[3] != 0)
	{
	  uma = mem_get_qi (regs.r[1] ++);
	  umb = mem_get_qi (regs.r[2] ++);
	  regs.r[3] --;
	  if (uma != umb || uma == 0)
	    break;
	}
      if (uma == umb)
	set_zc (1, 1);
      else
	set_zc (0, ((int)uma - (int)umb) >= 0);
      cycles (2 + 4 * (tx / 4) + 4 * (tx % 4));
      break;

    case RXO_setpsw:
      v = 1 << opcode->op[0].reg;
      if (FLAG_PM
	  && (v == FLAGBIT_I
	      || v == FLAGBIT_U))
      {
        cycles(1);
        break;
      }
      regs.r_psw |= v;
      cycles (1);
      break;

    case RXO_smovb:
      RL (3);
#ifdef CYCLE_ACCURATE
      tx = regs.r[3];
#endif
      while (regs.r[3])
	{
	  uma = mem_get_qi (regs.r[2] --);
	  mem_put_qi (regs.r[1]--, uma);
	  regs.r[3] --;
	}
#ifdef CYCLE_ACCURATE
      if (tx > 3)
	cycles (6 + 3 * (tx / 4) + 3 * (tx % 4));
      else
	cycles (2 + 3 * (tx % 4));
#endif
      break;

    case RXO_smovf:
      RL (3);
#ifdef CYCLE_ACCURATE
      tx = regs.r[3];
#endif
      while (regs.r[3])
	{
	  uma = mem_get_qi (regs.r[2] ++);
	  mem_put_qi (regs.r[1]++, uma);
	  regs.r[3] --;
	}
      cycles (2 + 3 * (int)(tx / 4) + 3 * (tx % 4));
      break;

    case RXO_smovu:
#ifdef CYCLE_ACCURATE
      tx = regs.r[3];
#endif
      while (regs.r[3] != 0)
	{
	  uma = mem_get_qi (regs.r[2] ++);
	  mem_put_qi (regs.r[1]++, uma);
	  regs.r[3] --;
	  if (uma == 0)
	    break;
	}
      cycles (2 + 3 * (int)(tx / 4) + 3 * (tx % 4));
      break;

    case RXO_shar: /* d = ma >> mb */
      SHIFT_OP (sll, int, mb, >>=, 1);
      E (1);
      break;

    case RXO_shll: /* d = ma << mb */
      SHIFT_OP (ll, int, mb, <<=, 0x80000000UL);
      E (1);
      break;

    case RXO_shlr: /* d = ma >> mb */
      SHIFT_OP (ll, unsigned int, mb, >>=, 1);
      E (1);
      break;

    case RXO_sstr:
      RL (3);
#ifdef CYCLE_ACCURATE
      tx = regs.r[3];
#endif
      switch (opcode->size)
	{
	case RX_Long:
	  while (regs.r[3] != 0)
	    {
	      mem_put_si (regs.r[1], regs.r[2]);
	      regs.r[1] += 4;
	      regs.r[3] --;
	    }
	  cycles (2 + tx);
	  break;
	case RX_Word:
	  while (regs.r[3] != 0)
	    {
	      mem_put_hi (regs.r[1], regs.r[2]);
	      regs.r[1] += 2;
	      regs.r[3] --;
	    }
	  cycles (2 + (int)(tx / 2) + tx % 2);
	  break;
	case RX_Byte:
	  while (regs.r[3] != 0)
	    {
	      mem_put_qi (regs.r[1], regs.r[2]);
	      regs.r[1] ++;
	      regs.r[3] --;
	    }
	  cycles (2 + (int)(tx / 4) + tx % 4);
	  break;
	default:
	  abort ();
	}
      break;

    case RXO_stcc:
      if (GS2())
	PD (GS ());
      E1;
      break;

    case RXO_stop:
      PRIVILEDGED ();
      regs.r_psw |= FLAGBIT_I;
      DO_RETURN (RX_MAKE_STOPPED(0));

    case RXO_sub:
      MATH_OP (-, 0);
      break;

    case RXO_suntil:
      RL(3);
#ifdef CYCLE_ACCURATE
      tx = 0;
#endif
      if (regs.r[3] == 0)
	{
	  cycles (3);
	  break;
	}
      switch (opcode->size)
	{
	case RX_Long:
	  uma = get_reg (2);
	  while (regs.r[3] != 0)
	    {
	      regs.r[3] --;
	      umb = mem_get_si (get_reg (1));
	      regs.r[1] += 4;
#ifdef CYCLE_ACCURATE
	      tx ++;
#endif
	      if (umb == uma)
		break;
	    }
#ifdef CYCLE_ACCURATE
	  cycles (3 + 3 * tx);
#endif
	  break;
	case RX_Word:
	  uma = get_reg (2) & 0xffff;
	  while (regs.r[3] != 0)
	    {
	      regs.r[3] --;
	      umb = mem_get_hi (get_reg (1));
	      regs.r[1] += 2;
#ifdef CYCLE_ACCURATE
	      tx ++;
#endif
	      if (umb == uma)
		break;
	    }
#ifdef CYCLE_ACCURATE
	  cycles (3 + 3 * (tx / 2) + 3 * (tx % 2));
#endif
	  break;
	case RX_Byte:
	  uma = get_reg (2) & 0xff;
	  while (regs.r[3] != 0)
	    {
	      regs.r[3] --;
	      umb = mem_get_qi (regs.r[1]);
	      regs.r[1] += 1;
#ifdef CYCLE_ACCURATE
	      tx ++;
#endif
	      if (umb == uma)
		break;
	    }
#ifdef CYCLE_ACCURATE
	  cycles (3 + 3 * (tx / 4) + 3 * (tx % 4));
#endif
	  break;
	default:
	  abort();
	}
      if (uma == umb)
	set_zc (1, 1);
      else
	set_zc (0, ((int)uma - (int)umb) >= 0);
      break;

    case RXO_swhile:
      RL(3);
#ifdef CYCLE_ACCURATE
      tx = 0;
#endif
      if (regs.r[3] == 0)
	break;
      switch (opcode->size)
	{
	case RX_Long:
	  uma = get_reg (2);
	  while (regs.r[3] != 0)
	    {
	      regs.r[3] --;
	      umb = mem_get_si (get_reg (1));
	      regs.r[1] += 4;
#ifdef CYCLE_ACCURATE
	      tx ++;
#endif
	      if (umb != uma)
		break;
	    }
#ifdef CYCLE_ACCURATE
	  cycles (3 + 3 * tx);
#endif
	  break;
	case RX_Word:
	  uma = get_reg (2) & 0xffff;
	  while (regs.r[3] != 0)
	    {
	      regs.r[3] --;
	      umb = mem_get_hi (get_reg (1));
	      regs.r[1] += 2;
#ifdef CYCLE_ACCURATE
	      tx ++;
#endif
	      if (umb != uma)
		break;
	    }
#ifdef CYCLE_ACCURATE
	  cycles (3 + 3 * (tx / 2) + 3 * (tx % 2));
#endif
	  break;
	case RX_Byte:
	  uma = get_reg (2) & 0xff;
	  while (regs.r[3] != 0)
	    {
	      regs.r[3] --;
	      umb = mem_get_qi (regs.r[1]);
	      regs.r[1] += 1;
#ifdef CYCLE_ACCURATE
	      tx ++;
#endif
	      if (umb != uma)
		break;
	    }
#ifdef CYCLE_ACCURATE
	  cycles (3 + 3 * (tx / 4) + 3 * (tx % 4));
#endif
	  break;
	default:
	  abort();
	}
      if (uma == umb)
	set_zc (1, 1);
      else
	set_zc (0, ((int)uma - (int)umb) >= 0);
      break;

    case RXO_wait:
      PRIVILEDGED ();
      regs.r_psw |= FLAGBIT_I;
      DO_RETURN (RX_MAKE_STOPPED(0));

    case RXO_xchg:
#ifdef CYCLE_ACCURATE
      regs.m2m = 0;
#endif
      v = GS (); /* This is the memory operand, if any.  */
      PS (GD ()); /* and this may change the address register.  */
      PD (v);
      E2;
#ifdef CYCLE_ACCURATE
      /* all M cycles happen during xchg's cycles.  */
      memory_dest = 0;
      memory_source = 0;
#endif
      break;

    case RXO_xor:
      if(opcode->op[2].type != RX_Operand_None)
      {
        //tprintf ("XOR\n");
        mb = GS();
        ma = GS2();
        v = ma ^ mb;
		set_sz (v, DSZ());
        PD(v);
      }
      else
      {
        LOGIC_OP (^);
      }
	  cycles (1);//TODO: update
      break;

	//TODO: ERIX
  case RXO_dabs:
    tprintf("DABS\n");
    dins = GS_DB ();
    PD_DB (rxdp_abs (dins));
    //set_osz (dins, 4);
    E (1);
    break;
  case RXO_dadd:
    DOUBLE_OP2(dadd);
	E (1);
	  break;
	case RXO_dcmp:
    da = GD_DB();
    db = GS2_DB();
    put_reg (dcmr, rxdp_cmp (da, db, GS()));
    E (1);
	  break;
	case RXO_ddiv:
    DOUBLE_OP2(ddiv);
	E (1);
	  break;
	//TODO: improve this we can reduce the number of instructions
  case RXO_dmov_1:
    tprintf("DMOV.D\n");
    dins = 0;
    dins |= (long long)GS() << 32;
    PD_DB(dins);
    E (1);
	break;
  case RXO_dmov_2:
    tprintf("DMOV.L\n");
    dins = GD_DB() & 0x00000000ffffffff;
    dins |= (long long)GS() << 32;
    PD_DB(dins);
    E (1);
	break;
  case RXO_dmov_3:
    tprintf("DMOV.L\n");
    dins = GD_DB() & 0xffffffff00000000;
    dins |= (long long)GS() & 0x00000000ffffffff;
    PD_DB(dins);
    E (1);
	break;
  case RXO_dmov_4:
    tprintf("DMOV.L\n");
    dins = GS_DB() >> 32;
    PD((long)dins);
    E (1);
	break;
  case RXO_dmov_5:
    tprintf("DMOV.L\n");
    dins = GS_DB() & 0x00000000ffffffff;
    PD((long)dins);
    E (1);
	break;
  case RXO_dmov_6:
    tprintf("DMOV.L\n");
    dins = GS_DB();
    PD_DB(dins);
    E (1);
	break;
  case RXO_dmov_7:
  case RXO_dmov_8:
  case RXO_dmov_9:
    tprintf("DMOV.D\n");
    dins = GS_DB();
    PD_64(dins);
    E (1);
	break;
  case RXO_dmov_10:
  case RXO_dmov_11:
  case RXO_dmov_12:
    tprintf("DMOV.D\n");
    dins = GS_64();
    PD_DB(dins);
    E (1);
	break;
  case RXO_dmov_13:
    tprintf("DMOV.D\n");
    dins = 0;
    dins |= (long long)GS_DB() << 32;
    PD_DB(dins);
    E (1);
    break;
  case RXO_dmov_14:
    tprintf("DMOV.L\n");
    dins = GD_DB() & 0x00000000ffffffff;
    dins |= (long long)GS_DB() << 32;
    PD_DB(dins);
    E (1);
    break;
  case RXO_dmov_15:
    tprintf("DMOV.L\n");
    dins = GD_DB() & 0xffffffff00000000;
    dins |= (long long)GS_DB() & 0x00000000ffffffff;
    PD_DB(dins);
    E (1);
    break;
	case RXO_dmul:
    DOUBLE_OP2 (dmul);
	E (1);
	  break;
	case RXO_dneg:
    tprintf("DNED\n");
    dins = GS_DB ();
    PD_DB (rxdp_neg (dins));
	E (1);
	  break;
	case RXO_dpopm:
    if (opcode->op[1].reg > opcode->op[0].reg)
    {
      regs.r_pc = opcode_pc;
      DO_RETURN (RX_MAKE_STOPPED (SIGILL));
    }
    for (v = opcode->op[1].reg; v <= opcode->op[0].reg; v++)
    {
      put_reg_double (v, pop_double ());
    }
	E (1);
	  break;
  case RXO_dpopm_2:
    if (opcode->op[1].reg > opcode->op[0].reg)
    {
      regs.r_pc = opcode_pc;
      DO_RETURN (RX_MAKE_STOPPED (SIGILL));
    }
    for (v = opcode->op[1].reg; v <= opcode->op[0].reg; v++)
    {
      put_reg (v + dpsw, pop());
    }
	E (1);
	  break;
  case RXO_dpushm:
    if (opcode->op[1].reg > opcode->op[0].reg)
    {
      regs.r_pc = opcode_pc;
      return RX_MAKE_STOPPED (SIGILL);
    }
    for (v = opcode->op[0].reg; v >= opcode->op[1].reg; v--)
    {
      push_double (get_reg_double (v));
    }
	E (1);
	  break;
  case RXO_dpushm_2:
    if (opcode->op[1].reg > opcode->op[0].reg)
    {
      regs.r_pc = opcode_pc;
      return RX_MAKE_STOPPED (SIGILL);
    }
    for (v = opcode->op[0].reg; v >= opcode->op[1].reg; v--)
    {
      push (get_reg (v + dpsw));
    }
	E (1);
	  break;
	case RXO_dround:
    dins = GS_DB ();
    PD_DB (rxdp_round (dins, regs.r_dpsw & DPSWBITS_DRM));
	E (1);
	  break;
	case RXO_dsqrt:
    dins = GS_DB ();
    PD_DB (rxdp_dsqrt (dins));
	E (1);
	  break;
	case RXO_dsub:
    DOUBLE_OP2(dsub);
	E (1);
	  break;
	case RXO_dtof:
    dins = GS_DB();
    PD_DB (rxdp_dtof (dins,  regs.r_dpsw & DPSWBITS_DRM));
	E (1);
	  break;
	case RXO_dtoi:
    dins = GS_DB();
    PD_DB  (rxdp_dtoi (dins, regs.r_dpsw & DPSWBITS_DRM));
	E (1);
	  break;
	case RXO_dtou:
    dins = GS_DB();
    PD_DB (rxdp_dtou (dins, regs.r_dpsw & DPSWBITS_DRM));
	E (1);
	  break;
	case RXO_ftod:
    uma = GS ();
    PD_DB (rxdp_ftod (uma));
	E (1);
	  break;
	case RXO_itod:
    uma = GS ();
    PD_DB (rxdp_itod (uma));
	E (1);
	  break;
  case RXO_mvfdc:
	PD(get_reg(opcode->op[1].reg + dpsw));
	E (1);
	  break;
	case RXO_mvfdr:
	/* write Z flag */
	regs.r_psw &= ~FLAGBIT_Z;
	regs.r_psw |= (get_reg(dcmr) & 1) << 1;
	E (1);
	  break;
	case RXO_mvtdc:
    put_reg(opcode->op[0].reg + dpsw, GS ());
	E (1);
	  break;
	case RXO_utod:
    uma = GS ();
    PD_DB (rxdp_utod (uma));
	E (1);
	  break;
    default:
      EXCEPTION (EX_UNDEFINED);
    }

#ifdef CYCLE_ACCURATE
  regs.m2m = 0;
  if (memory_source)
    regs.m2m |= M2M_SRC;
  if (memory_dest)
    regs.m2m |= M2M_DST;

  regs.rt = new_rt;
  new_rt = -1;
#endif

#ifdef WITH_PROFILE
  if (prev_cycle_count == regs.cycle_count)
    {
      printf("Cycle count not updated! id %s\n", id_names[opcode->id]);
      abort ();
    }
#endif

#ifdef WITH_PROFILE
  if (running_benchmark)
    {
      int omap = op_lookup (opcode->op[0].type, opcode->op[1].type, opcode->op[2].type);


      cycles_per_id[opcode->id][omap] += regs.cycle_count - prev_cycle_count;
      times_per_id[opcode->id][omap] ++;

      times_per_pair[prev_opcode_id][po0][opcode->id][omap] ++;

      prev_opcode_id = opcode->id;
      po0 = omap;
    }
#endif

  return RX_MAKE_STEPPED ();
}

#ifdef WITH_PROFILE
void
reset_pipeline_stats (void)
{
  memset (cycles_per_id, 0, sizeof(cycles_per_id));
  memset (times_per_id, 0, sizeof(times_per_id));
  memory_stalls = 0;
  register_stalls = 0;
  branch_stalls = 0;
  branch_alignment_stalls = 0;
  fast_returns = 0;
  memset (times_per_pair, 0, sizeof(times_per_pair));
  running_benchmark = 1;

  benchmark_start_cycle = regs.cycle_count;
}

void
halt_pipeline_stats (void)
{
  running_benchmark = 0;
  benchmark_end_cycle = regs.cycle_count;
}
#endif

void
pipeline_stats (void)
{
#ifdef WITH_PROFILE
  int i, o1;
  int p, p1;
#endif

#ifdef CYCLE_ACCURATE
  if (verbose == 1)
    {
      printf ("cycles: %llu\n", regs.cycle_count);
      return;
    }

  printf ("cycles: %13s\n", comma (regs.cycle_count));
#endif

#ifdef WITH_PROFILE
  if (benchmark_start_cycle)
    printf ("bmark:  %13s\n", comma (benchmark_end_cycle - benchmark_start_cycle));

  printf("\n");
  for (i = 0; i < N_RXO; i++)
    for (o1 = 0; o1 < N_MAP; o1 ++)
      if (times_per_id[i][o1])
	printf("%13s %13s %7.2f  %s %s\n",
	       comma (cycles_per_id[i][o1]),
	       comma (times_per_id[i][o1]),
	       (double)cycles_per_id[i][o1] / times_per_id[i][o1],
	       op_cache_string(o1),
	       id_names[i]+4);

  printf("\n");
  for (p = 0; p < N_RXO; p ++)
    for (p1 = 0; p1 < N_MAP; p1 ++)
      for (i = 0; i < N_RXO; i ++)
	for (o1 = 0; o1 < N_MAP; o1 ++)
	  if (times_per_pair[p][p1][i][o1])
	    {
	      printf("%13s   %s %-9s  ->  %s %s\n",
		     comma (times_per_pair[p][p1][i][o1]),
		     op_cache_string(p1),
		     id_names[p]+4,
		     op_cache_string(o1),
		     id_names[i]+4);
	    }

  printf("\n");
  printf("%13s memory stalls\n", comma (memory_stalls));
  printf("%13s register stalls\n", comma (register_stalls));
  printf("%13s branches taken (non-return)\n", comma (branch_stalls));
  printf("%13s branch alignment stalls\n", comma (branch_alignment_stalls));
  printf("%13s fast returns\n", comma (fast_returns));
#endif
}
