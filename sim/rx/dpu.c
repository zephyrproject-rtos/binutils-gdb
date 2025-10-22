/* dpu.c --- DPU emulator for stand-alone RX simulator.

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

#include "config.h"
#include <stdio.h>

#include "cpu.h"
#include "dpu.h"

/* DPU encodings are as follows:

   S EXPONENT    MANTISSA
   1 12345678901 1234567890123456789012345678901234567890123456789012

   0 00000000000 0000000000000000000000000000000000000000000000000000 +0
   1 00000000000 0000000000000000000000000000000000000000000000000000	-0

   X 00000000000 0000000000000000000000000000000000000000000000000001	Denormals
   X 00000000000 1111111111111111111111111111111111111111111111111111
 
   X 00000000001 XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX	Normals
   X 11111111110 XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

   0 11111111111 0000000000000000000000000000000000000000000000000000	+Inf
   1 11111111111 0000000000000000000000000000000000000000000000000000	-Inf

   X 11111111111 0XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX	SNaN (X != 0)
   X 11111111111 1XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX	QNaN (X != 0)

*/

//#define trace 1
#define tprintf if (trace) printf

/* Some magic numbers.  */
#define PLUS_MAX   0x7FEFFFFFFFFFFFFFULL
#define MINUS_MAX  0xFFEFFFFFFFFFFFFFULL
#define PLUS_INF   0x7FF0000000000000ULL
#define MINUS_INF  0xFFF0000000000000ULL
#define PLUS_ZERO  0x0000000000000000ULL
#define MINUS_ZERO 0x8000000000000000ULL

#define DP_RAISE(e) dp_raise(DPSWBITS_DC##e)
static void
dp_raise (int mask)
{
  regs.r_dpsw |= mask;
  if (mask != DPSWBITS_DCE)
    {
      if (regs.r_dpsw & (mask << DPSW_CESH))
	regs.r_dpsw |= (mask << DPSW_CFSH);
      if (regs.r_dpsw & DPSWBITS_FMASK)
	regs.r_dpsw |= DPSWBITS_DFS;
      else
	regs.r_dpsw &= ~DPSWBITS_DFS;
    }
}

/* We classify all numbers as one of these.  They correspond to the
   rows/colums in the exception tables.  */
typedef enum {
  DP_NORMAL,
  DP_PZERO,
  DP_NZERO,
  DP_PINFINITY,
  DP_NINFINITY,
  DP_DENORMAL,
  DP_QNAN,
  DP_SNAN
} DP_Type;

#if defined DEBUG0
static const char *fpt_names[] = {
  "Normal", "+0", "-0", "+Inf", "-Inf", "Denormal", "QNaN", "SNaN"
};
#endif

#define EXP_BIAS  1023
#define EXP_ZERO -1023
#define EXP_INF   1024

#define MANT_BIAS 0x0010000000000000ULL

typedef enum {
  FP_NORMAL,
  FP_PZERO,
  FP_NZERO,
  FP_PINFINITY,
  FP_NINFINITY,
  FP_DENORMAL,
  FP_QNAN,
  FP_SNAN
} FP_Type;

typedef struct {
  int exp;
  unsigned int mant; /* 24 bits */
  char type;
  char sign;
  fp_t orig_value;
} FP_Parts;

static void
fp_explode (fp_t f, FP_Parts *p)
{
  int exp, mant, sign;

  exp = ((f & 0x7f800000UL) >> 23);
  mant = f & 0x007fffffUL;
  sign = f & 0x80000000UL;
  /*printf("explode: %08x %x %2x %6x\n", f, sign, exp, mant);*/

  p->sign = sign ? -1 : 1;
  p->exp = exp - EXP_BIAS;
  p->orig_value = f;
  p->mant = mant | 0x00800000UL;

  if (p->exp == EXP_ZERO)
    {
      if (regs.r_fpsw & FPSWBITS_DN)
  mant = 0;
      if (mant)
  p->type = FP_DENORMAL;
      else
  {
    p->mant = 0;
    p->type = sign ? FP_NZERO : FP_PZERO;
  }
    }
  else if (p->exp == EXP_INF)
    {
      if (mant == 0)
  p->type = sign ? FP_NINFINITY : FP_PINFINITY;
      else if (mant & 0x00400000UL)
  p->type = FP_QNAN;
      else
  p->type = FP_SNAN;
    }
  else
    p->type = FP_NORMAL;
}

typedef struct {
  int exp;
  unsigned long long mant; /* 53 bits */
  char type;
  char sign;
  dp_t orig_value;
} DP_Parts;

static void
dp_explode (dp_t f, DP_Parts *p)
{
  unsigned long long mant;
  unsigned int exp, sign;
  //printf ("Erix\n");
  exp = ((f & 0x7FF0000000000000ULL) >> 52);
  mant = f & 0x000FFFFFFFFFFFFFULL;
  sign = (f >> 63) & 0x1;

  tprintf("explode: %016llx %d %x %016llx\n", f, sign, exp, mant);
  //printf("explode: %e %d %x %016llx\n", f, sign, exp, mant);


  p->sign = sign ? -1 : 1;
  p->exp = exp - EXP_BIAS;
  p->orig_value = f;
  p->mant = mant | MANT_BIAS;

  if (p->exp == EXP_ZERO)
  {
    //if (regs.r_dpsw & FPSWBITS_DN)
    //  mant = 0;
    if (mant && !(regs.r_dpsw & DPSWBITS_DDN))
    {
      p->type = DP_DENORMAL;
    }
    else
    {
      p->mant = 0;
      p->type = sign ? DP_NZERO : DP_PZERO;
    }
  }
  else if (p->exp == EXP_INF)
  {
    if (mant == 0)
      p->type = sign ? DP_NINFINITY : DP_PINFINITY;
    else if (mant & 0x0008000000000000ULL)
      p->type = DP_QNAN;
    else
      p->type = DP_SNAN;
    }
  else
    p->type = DP_NORMAL;
}

static dp_t
dp_implode (DP_Parts *p)
{
  unsigned long long mant;
  unsigned int exp;

  exp = p->exp + EXP_BIAS;
  mant = p->mant;

  if (p->type == DP_NORMAL)
  {
      while (mant && exp > 0
          && mant < 0x0010000000000000ULL)
      {
        mant <<= 1;
        exp --;
      }
      while (mant > 0x001FFFFFFFFFFFFFULL)
      {
        mant >>= 1;
        exp ++;
      }
      if (exp < 0)
      {
        /* underflow */
        exp = 0;
        mant = 0;
        dp_raise(DPSWBITS_DFU);
      }
      if (exp >= 2047)
      {
        /* overflow */
        exp = 2047;
        mant = 0;
        DP_RAISE (O);
      }
    }

  mant &= 0x000FFFFFFFFFFFFFULL;
  exp &= 0x7ff;
  mant |= (unsigned long long)exp << 52;
  if (p->sign < 0)
    mant |= 0x8000000000000000ULL;

  tprintf("implode: exp 0x%x mant 0x%016llx\n", exp, mant);
  return mant;
}

typedef union {
  unsigned long long ll;
  double d;
} U_d_ll;

typedef enum {
  eNR,		/* Use the normal result.  */
  ePZ, eNZ,	/* +- zero */
  eSZ,		/* signed zero - XOR signs of ops together.  */
  eRZ,		/* +- zero depending on rounding mode.  */
  ePI, eNI,	/* +- Infinity */
  eSI,		/* signed infinity - XOR signs of ops together.  */
  eQN, eSN,	/* Quiet/Signalling NANs */
  eIn,		/* Invalid.  */
  eUn,		/* Unimplemented.  */
  eDZ,		/* Divide-by-zero.  */
  eLT,		/* less than */
  eGT,		/* greater than */
  eEQ,		/* equal to */
} DP_ExceptionCases;
#if defined DEBUG0
static const char *fpt_names[] = {
  "Normal", "+0", "-0", "+Inf", "-Inf", "Denormal", "QNaN", "SNaN"
};
static const char *ex_names[] = {
  "NR", "PZ", "NZ", "SZ", "RZ", "PI", "NI", "SI", "QN", "SN", "IN", "Un", "DZ", "LT", "GT", "EQ"
};
#endif

/* Similar to check_exceptions in fpu.c, but for doubles,
   this checks for all exceptional cases (not all DP exceptions) and
   returns TRUE if it is providing the result in *c.  If it returns
   FALSE, the caller should do the "normal" operation.  */
int
check_dp_exceptions (DP_Parts *a, DP_Parts *b, dp_t *c,
		  DP_ExceptionCases ex_tab[5][5], 
		  DP_ExceptionCases *case_ret)
{
  DP_ExceptionCases dpec;
  if (a->type == DP_SNAN
      || b->type == DP_SNAN)
    dpec = eIn;
  else if (a->type == DP_QNAN
	   || b->type == DP_QNAN)
    dpec = eQN;
  else if (a->type == DP_DENORMAL
	   || b->type == DP_DENORMAL)
    dpec = eUn;
  else
    dpec = ex_tab[(int)(b->type)][(int)(a->type)];

  /*printf("%s %s -> %s\n", fpt_names[(int)(a->type)], fpt_names[(int)(b->type)], ex_names[(int)(dpec)]);*/

  if (case_ret)
    *case_ret = dpec;

  switch (dpec)
    {
    case eNR:	/* Use the normal result.  */
      return 0;

    case ePZ:	/* + zero */
      *c = 0x00000000;
      return 1;

    case eNZ:	/* - zero */
      *c = 0x8000000000000000;
      return 1;

    case eSZ:	/* signed zero */
      *c = (a->sign == b->sign) ? PLUS_ZERO : MINUS_ZERO;
      return 1;

    case eRZ:	/* +- zero depending on rounding mode.  */
      if ((regs.r_dpsw & DPSWBITS_DRM) == DPRM_NINF)
	*c = 0x8000000000000000;
      else
	*c = 0x00000000;
      return 1;

    case ePI:	/* + Infinity */
      *c = 0x7ff0000000000000;
      return 1;

    case eNI:	/* - Infinity */
      *c = 0xfff0000000000000;
      return 1;

    case eSI:	/* sign Infinity */
      *c = (a->sign == b->sign) ? PLUS_INF : MINUS_INF;
      return 1;

    case eQN:	/* Quiet NANs */
      if(a->type == DP_QNAN && b->type == DP_QNAN)
        *c = b->orig_value;
      else if (a->type == DP_QNAN)
	*c = a->orig_value;
      else
	*c = b->orig_value;
      return 1;

    case eSN:	/* Signalling NANs */
      if (a->type == DP_SNAN)
	*c = a->orig_value;
      else
	*c = b->orig_value;
      DP_RAISE (V);
      return 1;

    case eIn:	/* Invalid.  */
      DP_RAISE (V);
      if(a->type == DP_SNAN && b->type == DP_SNAN)
  *c = b->orig_value | 0x00008000000000000;
      else if (a->type == DP_SNAN)
	*c = a->orig_value | 0x00008000000000000;
      else if  (b->type == DP_SNAN)
	*c = b->orig_value | 0x00008000000000000;
      else
	*c = 0x7FFFFFFFFFFFFFFF; /*Neither source operand is an NaN and an invalid operation is generated)*/
      return 1;

    case eUn:	/* Unimplemented.  */
      DP_RAISE (E);
      return 1;

    case eDZ:	/* Division-by-zero.  */
      *c = (a->sign == b->sign) ? PLUS_INF : MINUS_INF;
      DP_RAISE (Z);
      return 1;

    default:
      return 0;
    }
}

#define CHECK_DP_EXCEPTIONS(DPPa, DPPb, dpc, ex_tab) \
  if (check_dp_exceptions (&DPPa, &DPPb, &dpc, ex_tab, 0))	\
    return dpc;

static DP_ExceptionCases ex_add_tab[5][5] = {
  /* N   +0   -0   +In  -In */
  { eNR, eNR, eNR, ePI, eNI }, /* Normal */
  { eNR, ePZ, eRZ, ePI, eNI }, /* +0   */
  { eNR, eRZ, eNZ, ePI, eNI }, /* -0   */
  { ePI, ePI, ePI, ePI, eIn }, /* +Inf */
  { eNI, eNI, eNI, eIn, eNI }, /* -Inf */
};

dp_t
rxdp_add (dp_t fa, dp_t fb)
{
  DP_Parts a, b, c;
  dp_t rv;
  U_d_ll da, db;
  double res;

  da.ll = fa;
  db.ll = fb;

  dp_explode (fa, &a);
  dp_explode (fb, &b);

  if(check_dp_exceptions(&a, &b, &rv, ex_add_tab, NULL)) {
    //tprintf("DADD src or src2 edge case\n");
    tprintf("%lf + %lf = %lf\n", da.d, db.d, rv);
    return rv;
  } else {
    res = da.d + db.d;
    tprintf("%lf + %lf = %lf\n", da.d, db.d, res);
    da.d = res;
    rv = da.ll;
  }

  dp_explode(rv, &c);
  if(c.type == DP_DENORMAL) {
    regs.r_dpsw |= DPSWBITS_DFU;
  }

  //rv = dp_implode (&c);
  return rv;
}

static DP_ExceptionCases ex_sub_tab[5][5] = {
  /* N   +0   -0   +In  -In */
  { eNR, eNR, eNR, eNI, ePI }, /* Normal */
  { eNR, eRZ, ePZ, eNI, ePI }, /* +0   */
  { eNR, eNZ, eRZ, eNI, ePI }, /* -0   */
  { ePI, ePI, ePI, eIn, ePI }, /* +Inf */
  { eNI, eNI, eNI, eNI, eIn }, /* -Inf */
};

dp_t
rxdp_sub (dp_t fa, dp_t fb)
{
  dp_t rv;
  U_d_ll da, db;
  DP_Parts a, b, c;
  double res;

  da.ll = fa;
  db.ll = fb;

  dp_explode(fa, &a);
  dp_explode(fb, &b);

  if(check_dp_exceptions(&a, &b, &rv, ex_sub_tab, NULL)) {
    //tprintf("DSUB src edge case\n");
    return rv;
  } else {
    res = db.d - da.d;
    tprintf("%f - %f = %f\n", db.d, da.d, res);
    da.d = res;
    rv = da.ll;
  }

  dp_explode(rv, &c);
  if(c.type == DP_DENORMAL) {
    regs.r_dpsw |= DPSWBITS_DFU;
  }
    
  return rv;
}

static DP_ExceptionCases ex_cmp_tab[5][5] = {
  /* N   +0   -0   +In  -In */
  { eNR, eNR, eNR, eLT, eGT }, /* Normal */
  { eNR, eEQ, eEQ, eLT, eGT }, /* +0   */
  { eNR, eEQ, eEQ, eLT, eGT }, /* -0   */
  { eGT, eGT, eGT, eEQ, eGT }, /* +Inf */
  { eLT, eLT, eLT, eLT, eEQ }, /* -Inf */
};

static int isNaN(unsigned long long value)
{
	if(((value >= 0x7FF0000000000001) && (value < 0x7FF7FFFFFFFFFFFF)) ||
		((value >= 0xFFF0000000000001) && (value < 0xFFF7FFFFFFFFFFFF)) ||
		((value >= 0x7FF8000000000000) && (value < 0x7FFFFFFFFFFFFFFF)) ||
		((value >= 0xFFF8000000000000) && (value < 0xFFFFFFFFFFFFFFFF)))
	{
		return 1;
	}
	return 0;
}

void abort();

unsigned int
rxdp_cmp (dp_t fa, dp_t fb, unsigned int cond)
{
	U_d_ll da, db;
	const char *condition_name;
	unsigned int res;
	
	da.ll = fa;
	db.ll = fb;
	
	switch(cond)
	{
		case 1:
			res = isNaN(db.ll) || isNaN(da.ll);
			condition_name = "<>";
			break;
		case 2:
			res = (db.d == da.d);
			condition_name = "==";
			break;
		case 4:
			res = (db.d < da.d);
			condition_name = "<";
			break;
		case 6:
			res = (db.d <= da.d);
			condition_name = "<=";
			break;
		default:
			abort();
	}

	tprintf("%lf %s %lf => RES=%d\n", db.d, condition_name, da.d, res);
	
	return res & 0x1;
}

static DP_ExceptionCases ex_div_tab[5][5] = {
  /* N   +0   -0   +In  -In */
  { eNR, eDZ, eDZ, eSZ, eSZ }, /* Normal */
  { eSZ, eIn, eIn, ePZ, eNZ }, /* +0   */
  { eSZ, eIn, eIn, eNZ, ePZ }, /* -0   */
  { eSI, ePI, eNI, eIn, eIn }, /* +Inf */
  { eSI, eNI, ePI, eIn, eIn }, /* -Inf */
};

dp_t
rxdp_div (dp_t fa, dp_t fb)
{
  DP_Parts a, b, c;
  dp_t rv;
  U_d_ll da, db;
  U_d_ll res;

  da.ll = fa;
  db.ll = fb;

  dp_explode (fa, &a);
  dp_explode (fb, &b);
  
  if(check_dp_exceptions (&a, &b, &rv, ex_div_tab, NULL)) {
    //tprintf("DDIV src or src2 edge case\n");
    return rv;
  }

  res.d = db.d / da.d;
  rv = res.ll;

  dp_explode(rv, &c);
  if(c.type == DP_DENORMAL) {
    regs.r_dpsw |= DPSWBITS_DFU;
  }

  tprintf("%e / %e = %llx\n", db.d, da.d, rv);

  return rv;
}

static DP_ExceptionCases ex_mul_tab[5][5] = {
  /* N   +0   -0   +In  -In */
  { eNR, eNR, eNR, eSI, eSI }, /* Normal */
  { eNR, ePZ, eNZ, eIn, eIn }, /* +0   */
  { eNR, eNZ, ePZ, eIn, eIn }, /* -0   */
  { eSI, eIn, eIn, ePI, eNI }, /* +Inf */
  { eSI, eIn, eIn, eNI, ePI }, /* -Inf */
};

dp_t
rxdp_mul (dp_t fa, dp_t fb)
{
  dp_t rv;
  U_d_ll da, db, res;
  DP_Parts a, b, c;

  da.ll = fa;
  db.ll = fb;

  dp_explode(fa, &a);
  dp_explode(fb, &b);

  if(check_dp_exceptions(&a, &b, &rv, ex_mul_tab, NULL)) {
    //tprintf("DMUL src edge case\n");
    return rv;
  } else {
    res.d = da.d * db.d;
    rv = res.ll;
  }

  dp_explode(rv, &c);
  if(c.type == DP_DENORMAL) {
    regs.r_dpsw |= DPSWBITS_DFU;
  }

  tprintf("%f x %f = %f\n", da.d, db.d, res);
  
  return rv;
}

dp_t
rxdp_abs (dp_t fa)
{
  DP_Parts a, b;
  dp_t rv;
  U_d_ll orig, u;

  orig.ll = fa;

  dp_explode (fa, &a);
  a.sign = 0;
  rv = dp_implode (&a);

  u.ll = rv;
  tprintf("|%lf| = ", orig.d);
  tprintf("%lf\n", u.d);
  return rv;
}

dp_t
rxdp_neg (dp_t fa)
{
  DP_Parts a, b;
  dp_t rv;
  U_d_ll orig, u;

  orig.ll = fa;

  dp_explode (fa, &a);
  a.sign = -a.sign;
  rv = dp_implode (&a);

  u.ll = rv;
  tprintf("%lf => ", orig.d);
  tprintf("%lf\n", u.d);
  return rv;
}

/* we don't include math because of FP_DENORMAL and others */
double  sqrt(double x);

dp_t
rxdp_dsqrt (dp_t fa)
{
  DP_Parts a;
  dp_t rv;
  U_d_ll da, db;
  double res;

  da.ll = fa;

  int sign, exp, mant, qnan;
  dp_explode (fa, &a);
 
  //tprintf("exp, mantisa, sign, type: %d 0x%016llx %d %d\n",a.exp, a.mant, a.sign, a.type);
  switch (a.type)
  {
    case DP_NORMAL:
      break;
    case DP_PZERO:
      return 0;
    case DP_NZERO:
      return 0x8000000000000000;
    case DP_DENORMAL:
      DP_RAISE (E);
      return 0;
    case DP_QNAN:
      return fa;
    case DP_SNAN:
      DP_RAISE (V);
      return fa | 0x00008000000000000;
  }

  if (a.sign < 0)
  {
    DP_RAISE (V);
    return 0x7FFFFFFFFFFFFFFF;
  }

  db.d = res = sqrt (da.d);
  tprintf("sqrt(%lf) = %016llx %016llx\n", da.d, res, db.ll);

  rv = db.ll;

  return rv;
}

fp_t
rxdp_dtof (dp_t fa, int round_mode)
{

  DP_Parts a;
   dp_t rv;

  int sign, exp, mant, qnan;
  dp_explode (fa, &a);
 
  //tprintf("exp, mantisa, sign, type: %d 0x%016llx %d %d\n",a.exp, a.mant, a.sign, a.type);
  //tprintf("explode: %016llx %d %x %016llx\n", f, sign, exp, mant);
  switch (a.type)
  {
    case DP_NORMAL:
      break;
    case DP_PZERO:
      return 0;
    case DP_NZERO:
      return 0x80000000UL;
    case DP_DENORMAL:
      DP_RAISE (E);
      return 0;
    case DP_QNAN:
      qnan = 0x7F800000 | ((a.mant >> 29)  & 0xFFFFFF);
      return a.sign < 0? (qnan|0x00800000UL) : qnan;
    case DP_SNAN:
      DP_RAISE (V);
      qnan = 0x7FC00000 | ((a.mant >> 29)  & 0xFFFFFF);
      return a.sign < 0? (qnan|0x00800000UL) : qnan;
  }
  
  if (a.exp >= 128)
    {
      if(a.type == DP_PINFINITY)
        return 0x7f800000;
      else
        if (a.type == DP_NINFINITY)
          return 0xff800000;
        else
        {
          DP_RAISE (V);
          return sign = a.sign < 0? 0xff800000 : 0x7f800000;
        }
    }
  if (a.exp <= -127)
    {
      DP_RAISE (E);
      DP_RAISE (U);
      return 0;
    }
 
  a.exp -= 1024;
  sign = a.sign < 0? -1 : 1;
  exp = a.exp;
  mant = (fa >> (52-23)) & 0x007fffffUL;
  mant |= 0x00800000UL;
//TO DO 
  //inexact exception occurs whn result is rounded
  //overflow exception occurs when exponent after rounding is 128
  if (fa & 0x1ffffffffULL)
  {
    switch (round_mode & 3)
    {
      case DPRM_NEAREST:
        if (fa & 0x10000000ULL)
          mant ++;
        break;
      case DPRM_ZERO:
        break;
      case DPRM_PINF:
        if (!sign)
          mant ++;
        break;
      case DPRM_NINF:
        if (sign)
          mant ++;
        break;
    }
  }

  exp += 127;
  while (mant
       && exp > 0
       && mant < 0x00800000UL)
  {
    mant <<= 1;
    exp --;
  }

  while (mant > 0x00ffffffUL)
  {
    mant >>= 1;
    exp ++;
  }

  mant &= 0x007fffffUL;
  exp &= 0xff;
  mant |= exp << 23;

  if (sign < 0)
    mant |= 0x80000000UL;

  return mant;
}

int
rxdp_dtoi (dp_t fa, int round_mode)
{

  DP_Parts a;
   dp_t rv;

  int sign;
  int whole_bits, frac_bits;
 

  dp_explode (fa, &a);
 
  switch (a.type)
  {
    case DP_NORMAL:
      break;
    case DP_PZERO:
    case DP_NZERO:
      return 0;
    case DP_PINFINITY:
      DP_RAISE (V);
      return 0x7fffffffL;
    case DP_NINFINITY:
      DP_RAISE (V);
      return 0x80000000L;
    case DP_DENORMAL:
      DP_RAISE (E);
      return 0;
    case DP_QNAN:
    case DP_SNAN:
      DP_RAISE (V);
      return a.sign < 0? 0x80000000U : 0x7fffffff;
  }
 

  if (a.exp >= 31)
  {
    DP_RAISE (V);
    return a.sign < 0? 0x80000000U : 0x7fffffff;
  }
 

  a.exp -= 52;
 

  if (a.exp <= -54)
  {
    /* Less than 0.49999 */
    frac_bits = a.mant;
    whole_bits = 0;
  }
  else
    if (a.exp < 0)
    {
      frac_bits = a.mant << (64 + a.exp);
      whole_bits = a.mant >> (-a.exp);
    }
    else
    {
      frac_bits = 0;
      whole_bits = a.mant << a.exp;
    }

  if (frac_bits)
  {
    switch (round_mode & 3)
    {
    case DPRM_NEAREST:
      if (frac_bits & 0x10000000ULL)
       whole_bits ++;
      break;
    case DPRM_ZERO:
      break;
    case DPRM_PINF:
      if (!sign)
        whole_bits ++;
      break;
    case DPRM_NINF:
      if (sign)
        whole_bits ++;
      break;
    }
  }

  rv = a.sign < 0? -whole_bits : whole_bits;
  
  return rv;
 }

unsigned int
rxdp_dtou (dp_t fa, int round_mode)
{
  DP_Parts a;
  dp_t rv;

  int sign;
  int whole_bits, frac_bits;
 

  dp_explode (fa, &a);
 
  switch (a.type)
  {
    case DP_NORMAL:
      break;
    case DP_PZERO:
    case DP_NZERO:
      return 0;
    case DP_PINFINITY:
      DP_RAISE (V);
      return 0xffffffffL;
    case DP_NINFINITY:
      DP_RAISE (V);
      return 0;
    case DP_DENORMAL:
      DP_RAISE (E);
      return 0;
    case DP_QNAN:
    case DP_SNAN:
      DP_RAISE (V);
      return a.sign < 0? 0x00000000U : 0xffffffff;
  }
 

  if (a.exp >= 31)
  {
    DP_RAISE (V);
    return a.sign < 0? 0x00000000U : 0xffffffff;
  }
 
  a.exp -= 52;
 
  if (a.exp <= -54)
  {
    /* Less than 0.49999 */
    frac_bits = a.mant;
    whole_bits = 0;
  }
  else 
    if (a.exp < 0)
    {
      frac_bits = a.mant << (64 + a.exp);
      whole_bits = a.mant >> (-a.exp);
    }
    else
    {
      frac_bits = 0;
      whole_bits = a.mant << a.exp;
    }

  if (frac_bits)
  {
    switch (round_mode & 3)
    {
    case DPRM_NEAREST:
      if (frac_bits & 0x10000000ULL)
        whole_bits ++;
      break;
    case DPRM_ZERO:
      break;
    case DPRM_PINF:
      if (!sign)
        whole_bits ++;
      break;
    case DPRM_NINF:
      if (sign)
        whole_bits ++;
      break;
    }
  }

  rv = a.sign < 0? 0x00000000U : whole_bits;
  
  return rv;
 }

dp_t
rxdp_ftod (fp_t fa)
{
  U_d_ll da;
  double res;
  unsigned long long qnan;

  FP_Parts a;
  fp_explode (fa, &a);

  switch (a.type)
  {
    case FP_NORMAL:
      break;
    case FP_PZERO:
      return 0;
    case FP_NZERO:
      return 0x8000000000000000;
    case FP_DENORMAL:
      DP_RAISE (E);
      return 0;
    case FP_NINFINITY:
      return 0xFFF0000000000000;
    case FP_PINFINITY:
      return 0x7FF0000000000000;
    case FP_QNAN:
      qnan = a.mant << 29;
      return a.sign<0? qnan & 0xFFFFFFFFFFFFFFFF : 0x7FFFFFFFFFFFFFFF;
    case FP_SNAN:
      DP_RAISE (V);
      qnan = a.mant << 29;
      return a.sign<0? qnan & 0xFFFFFFFFFFFFFFFF : 0x7FFFFFFFFFFFFFFF;
  }

  res = (double)(*(float *)&fa);

  tprintf("(double) %f = %lf\n", (*(float *)&fa), res);
  
  da.d = res;

  return da.ll;
}

dp_t
rxdp_itod (int fa)
{
  //DP_Parts a, b, c;
  dp_t rv;
  U_d_ll da;
  double res;

  res = (double)fa;

  tprintf("(double) %d = %lf\n", fa, res);

  da.d = res;

  return da.ll;
}

dp_t
rxdp_utod (unsigned int fa)
{
  //DP_Parts a, b, c;
  dp_t rv;
  U_d_ll da;
  double res;

  da.ll = fa;

  res = (double)fa;

  tprintf("(double) %u = %lf\n", fa, res);

  da.d = res;

  return da.ll;
}

double  round(double x);
double  trunc(double x);
double  ceil(double x);
double  floor(double x);

dp_t
rxdp_round (dp_t fa, int round_mode)
{
  dp_t rv;
  U_d_ll da, db;
  DP_Parts a;
  double res;

  da.ll = fa;

  dp_explode (fa, &a);
 
  switch (a.type)
  {
    case DP_NORMAL:
      break;
    case DP_PZERO:
    case DP_NZERO:
      return 0;
    case DP_PINFINITY:
      DP_RAISE (V);
      return 0x7fffffff;
    case DP_NINFINITY:
      DP_RAISE (V);
      return 0x80000000;
    case DP_DENORMAL:
      DP_RAISE (E);
      return 0;
    case DP_QNAN:
    case DP_SNAN:
      DP_RAISE (V);
      return a.sign < 0? 0x80000000U : 0x7fffffff;
  }
 

  if (a.exp >= 31)
  {
    DP_RAISE (V);
    return a.sign < 0? 0x80000000U : 0x7FFFFFFF;
  }

  switch (round_mode & 3)
  {
    case DPRM_NEAREST:
      db.d = res = round (da.d);
      break;
    case DPRM_ZERO:
      db.d = res = trunc (da.d);
      break;
    case DPRM_PINF:
      db.d = res = ceil (da.d);
      break;
    case DPRM_NINF:
      db.d = res = floor (da.d);
      break;
  }


  rv = rxdp_dtoi(db.ll,round_mode);
  
  return rv;
}
