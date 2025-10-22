/* mem.c --- memory for RX simulator.

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

/* This must come before any other includes.  */
#include "defs.h"

/* This slows down the simulator and we get some false negatives from
   gcc, like when it uses a long-sized hole to hold a byte-sized
   variable, knowing that it doesn't care about the other bits.  But,
   if you need to track down a read-from-unitialized bug, set this to
   1.  */
#define RDCHECK 0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "opcode/rx.h"
#include "mem.h"
#include "cpu.h"
#include "syscalls.h"
#include "misc.h"
#include "err.h"

#define L1_BITS  (10)
#define L2_BITS  (10)
#define OFF_BITS PAGE_BITS

#define L1_LEN  (1 << L1_BITS)
#define L2_LEN  (1 << L2_BITS)
#define OFF_LEN (1 << OFF_BITS)

/* context memory: 256 locations containing: r1-r15, USP, FPSW, ACC0, ACC1  */
static unsigned char context_memory[256][4*15+4+4+2*12];

static unsigned char **pt[L1_LEN];
static unsigned char **ptr[L1_LEN];
static RX_Opcode_Decoded ***ptdc[L1_LEN];

/* [ get=0/put=1 ][ byte size ] */
static unsigned int mem_counters[2][13];

#define COUNT(isput,bytes)                                      \
  if (verbose && enable_counting) mem_counters[isput][bytes]++

void
init_mem (void)
{
  int i, j;

  for (i = 0; i < L1_LEN; i++)
    if (pt[i])
      {
	for (j = 0; j < L2_LEN; j++)
	  if (pt[i][j])
	    free (pt[i][j]);
	free (pt[i]);
      }
  memset (pt, 0, sizeof (pt));
  memset (ptr, 0, sizeof (ptr));
  memset (mem_counters, 0, sizeof (mem_counters));
}

unsigned char
rx_context_mem_read_byte (unsigned long location, unsigned long offset)
{
	if (trace)
	{
		printf (" context_memory(%d)[%d] => 0x%02X\n", location, offset, context_memory[location][offset]);
	}
	return context_memory[location][offset];
}

unsigned char
rx_context_mem_write_byte (unsigned long location, unsigned long offset, unsigned char value)
{
	if (trace)
	{
		printf (" context_memory(%d)[%d] <= 0x%02X\n", location, offset, value);
	}
	context_memory[location][offset] = value;
}

unsigned char *
rx_mem_ptr (unsigned long address, enum mem_ptr_action action)
{
  int pt1 = (address >> (L2_BITS + OFF_BITS)) & ((1 << L1_BITS) - 1);
  int pt2 = (address >> OFF_BITS) & ((1 << L2_BITS) - 1);
  int pto = address & ((1 << OFF_BITS) - 1);

  if (address == 0)
    execution_error (SIM_ERR_NULL_POINTER_DEREFERENCE, 0);

  if (pt[pt1] == 0)
    {
      pt[pt1] = (unsigned char **) calloc (L2_LEN, sizeof (char **));
      ptr[pt1] = (unsigned char **) calloc (L2_LEN, sizeof (char **));
      ptdc[pt1] = (RX_Opcode_Decoded ***) calloc (L2_LEN, sizeof (RX_Opcode_Decoded ***));
    }
  if (pt[pt1][pt2] == 0)
    {
      if (action == MPA_READING)
	execution_error (SIM_ERR_READ_UNWRITTEN_PAGES, address);

      pt[pt1][pt2] = (unsigned char *) calloc (OFF_LEN, 1);
      ptr[pt1][pt2] = (unsigned char *) calloc (OFF_LEN, 1);
      ptdc[pt1][pt2] = (RX_Opcode_Decoded **) calloc (OFF_LEN, sizeof(RX_Opcode_Decoded *));
    }
  else if (action == MPA_READING
	   && ptr[pt1][pt2][pto] == MC_UNINIT)
    execution_error (SIM_ERR_READ_UNWRITTEN_BYTES, address);

  if (action == MPA_WRITING)
    {
      int pto_dc;
      if (ptr[pt1][pt2][pto] == MC_PUSHED_PC)
	execution_error (SIM_ERR_CORRUPT_STACK, address);
      ptr[pt1][pt2][pto] = MC_DATA;

      /* The instruction decoder doesn't store it's decoded instructions
         at word swapped addresses.  Therefore, when clearing the decode
	 cache, we have to account for that here.  */
      pto_dc = pto ^ (rx_big_endian ? 3 : 0);
      if (ptdc[pt1][pt2][pto_dc])
	{
	  free (ptdc[pt1][pt2][pto_dc]);
	  ptdc[pt1][pt2][pto_dc] = NULL;
	}
    }

  if (action == MPA_CONTENT_TYPE)
    return (unsigned char *) (ptr[pt1][pt2] + pto);

  if (action == MPA_DECODE_CACHE)
    return (unsigned char *) (ptdc[pt1][pt2] + pto);

  return pt[pt1][pt2] + pto;
}

RX_Opcode_Decoded **
rx_mem_decode_cache (unsigned long address)
{
  return (RX_Opcode_Decoded **) rx_mem_ptr (address, MPA_DECODE_CACHE);
}

static inline int
is_reserved_address (unsigned int address)
{
  return (address >= 0x00020000 && address < 0x00080000)
    ||   (address >= 0x00100000 && address < 0x01000000)
    ||   (address >= 0x08000000 && address < 0xff000000);
}

static void
used (int rstart, int i, int j)
{
  int rend = i << (L2_BITS + OFF_BITS);
  rend += j << OFF_BITS;
  if (rstart == 0xe0000 && rend == 0xe1000)
    return;
  printf ("mem:   %08x - %08x (%dk bytes)\n", rstart, rend - 1,
	  (rend - rstart) / 1024);
}

static char *
mcs (int isput, int bytes)
{
  return comma (mem_counters[isput][bytes]);
}

void
mem_usage_stats (void)
{
  int i, j;
  int rstart = 0;
  int pending = 0;

  for (i = 0; i < L1_LEN; i++)
    if (pt[i])
      {
	for (j = 0; j < L2_LEN; j++)
	  if (pt[i][j])
	    {
	      if (!pending)
		{
		  pending = 1;
		  rstart = (i << (L2_BITS + OFF_BITS)) + (j << OFF_BITS);
		}
	    }
	  else if (pending)
	    {
	      pending = 0;
	      used (rstart, i, j);
	    }
      }
    else
      {
	if (pending)
	  {
	    pending = 0;
	    used (rstart, i, 0);
	  }
      }
  /*       mem foo: 123456789012 123456789012 123456789012 123456789012
            123456789012 */
  printf ("                 byte        short        3byte         long"
          "       opcode\n");
  if (verbose > 1)
    {
      /* Only use comma separated numbers when being very verbose.
	 Comma separated numbers are hard to parse in awk scripts.  */
      printf ("mem get: %12s %12s %12s %12s %12s\n", mcs (0, 1), mcs (0, 2),
	      mcs (0, 3), mcs (0, 4), mcs (0, 0));
      printf ("mem put: %12s %12s %12s %12s\n", mcs (1, 1), mcs (1, 2),
	      mcs (1, 3), mcs (1, 4));
    }
  else
    {
      printf ("mem get: %12u %12u %12u %12u %12u\n",
	      mem_counters[0][1], mem_counters[0][2],
	      mem_counters[0][3], mem_counters[0][4],
	      mem_counters[0][0]);
      printf ("mem put: %12u %12u %12u %12u\n",
	      mem_counters [1][1], mem_counters [1][2],
	      mem_counters [1][3], mem_counters [1][4]);
    }
}

unsigned long
mem_usage_cycles (void)
{
  unsigned long rv = mem_counters[0][0];
  rv += mem_counters[0][1] * 1;
  rv += mem_counters[0][2] * 2;
  rv += mem_counters[0][3] * 3;
  rv += mem_counters[0][4] * 4;
  rv += mem_counters[1][1] * 1;
  rv += mem_counters[1][2] * 2;
  rv += mem_counters[1][3] * 3;
  rv += mem_counters[1][4] * 4;
  return rv;
}

static int tpr = 0;
static void
s (int address, char *dir)
{
  if (tpr == 0)
    printf ("MEM[%08x] %s", address, dir);
  tpr++;
}

#define S(d) if (trace) s(address, d)
static void
e (void)
{
  if (!trace)
    return;
  tpr--;
  if (tpr == 0)
    printf ("\n");
}

static char
mtypec (int address)
{
  unsigned char *cp = rx_mem_ptr (address, MPA_CONTENT_TYPE);
  return "udp"[*cp];
}

#define E() if (trace) e()

static void
mem_put_byte (unsigned int address, unsigned char value)
{
  unsigned char *m;
  char tc = ' ';

  if (trace)
    tc = mtypec (address);
  m = rx_mem_ptr (address, MPA_WRITING);
  if (trace)
    printf (" %02x%c", value, tc);
  *m = value;
  switch (address)
    {
    case 0x0008c02a: /* PA.DR */
     {
	static int old_led = -1;
	int red_on = 0;
	int i;

	if (old_led != value)
	  {
	    fputs (" ", stdout);
	    for (i = 0; i < 8; i++)
	      if (value & (1 << i))
		{
		  if (! red_on)
		    {
		      fputs ("\033[31m", stdout);
		      red_on = 1;
		    }
		  fputs (" @", stdout);
		}
	      else
		{
		  if (red_on)
		    {
		      fputs ("\033[0m", stdout);
		      red_on = 0;
		    }
		  fputs (" *", stdout);
		}

	    if (red_on)
	      fputs ("\033[0m", stdout);

	    fputs ("\r", stdout);
	    fflush (stdout);
	    old_led = value;
	  }
      }
      break;

#ifdef WITH_PROFILE
    case 0x0008c02b: /* PB.DR */
      {
	if (value == 0)
	  halt_pipeline_stats ();
	else
	  reset_pipeline_stats ();
	break;
      }
#endif

    case 0x00088263: /* SCI4.TDR */
      {
	static int pending_exit = 0;
	if (pending_exit == 2)
	  {
	    step_result = RX_MAKE_EXITED(value);
	    longjmp (decode_jmp_buf, 1);
	  }
	else if (value == 3)
	  pending_exit ++;
	else
	  pending_exit = 0;

	putchar(value);
      }
      break;

    default:
      if (is_reserved_address (address))
	generate_access_exception ();
    }
}

void
mem_put_qi (int address, unsigned char value)
{
  S ("<=");
  mem_put_byte (address, value & 0xff);
  E ();
  COUNT (1, 1);
}

#ifdef CYCLE_ACCURATE
static int tpu_base;
#endif

void
mem_put_hi (int address, unsigned short value)
{
  S ("<=");
  switch (address)
    {
#ifdef CYCLE_ACCURATE
    case 0x00088126: /* TPU1.TCNT */
      tpu_base = regs.cycle_count;
      break;
    case 0x00088136: /* TPU2.TCNT */
      tpu_base = regs.cycle_count;
      break;
#endif
    default:
      if (rx_big_endian)
	{
	  mem_put_byte (address, value >> 8);
	  mem_put_byte (address + 1, value & 0xff);
	}
      else
	{
	  mem_put_byte (address, value & 0xff);
	  mem_put_byte (address + 1, value >> 8);
	}
    }
  E ();
  COUNT (1, 2);
}

void
mem_put_psi (int address, unsigned long value)
{
  S ("<=");
  if (rx_big_endian)
    {
      mem_put_byte (address, value >> 16);
      mem_put_byte (address + 1, (value >> 8) & 0xff);
      mem_put_byte (address + 2, value & 0xff);
    }
  else
    {
      mem_put_byte (address, value & 0xff);
      mem_put_byte (address + 1, (value >> 8) & 0xff);
      mem_put_byte (address + 2, value >> 16);
    }
  E ();
  COUNT (1, 3);
}

unsigned int converttoq131(float n) {
  unsigned result = (n < 0)? (1 << 31) : 0;
  float index = 1;
  if (n < 0) n = -n;
  for (int i = 1; i <= 31; ++i) {
    index /= 2;
    if (n >= index) {
      result |= (1 << 31 - i);
      n -= index;
    }
  }
  return result;
}

float convertfromq131(unsigned int n) {
  float result = 0;
  float index = 1;
  for (int i = 1; i <= 31; ++i) {
    index /= 2;
    result += ((n >> (31 - i)) & 1) * index;
  }
  result *= (n >> 31) ? -1 : 1;
  return result;
}

typedef union {
  unsigned long l;
  float d;
} U_d_ll;

int tfu;

void
mem_put_si (int address, unsigned long value)
{
  U_d_ll da, db;

  if (tfu)
  {
    int newAddr;
    unsigned long value2;
    switch (address)
    {
      case 0x00081410:
        break;
      case 0x00081414:
        da.l = value;
        newAddr = 0x00081410;
        da.d = cos (da.d);

        if (rx_big_endian)
        {
          mem_put_byte (newAddr + 0, (da.l >> 24) & 0xff);
          mem_put_byte (newAddr + 1, (da.l >> 16) & 0xff);
          mem_put_byte (newAddr + 2, (da.l >> 8) & 0xff);
          mem_put_byte (newAddr + 3, da.l & 0xff);
        }
        else
        {
          mem_put_byte (newAddr + 0, da.l & 0xff);
          mem_put_byte (newAddr + 1, (da.l >> 8) & 0xff);
          mem_put_byte (newAddr + 2, (da.l >> 16) & 0xff);
          mem_put_byte (newAddr + 3, (da.l >> 24) & 0xff);
        }
        da.l = value;
        da.d = sin (da.d);
        value = da.l;
        break;
      case 0x00081424:
        da.d = convertfromq131(value);
        da.d = cos (da.d);
        newAddr = 0x00081420;
        da.l = converttoq131(da.d);
        if (rx_big_endian)
        {
          mem_put_byte (newAddr + 0, (da.l >> 24) & 0xff);
          mem_put_byte (newAddr + 1, (da.l >> 16) & 0xff);
          mem_put_byte (newAddr + 2, (da.l >> 8) & 0xff);
          mem_put_byte (newAddr + 3, da.l & 0xff);
        }
        else
        {
          mem_put_byte (newAddr + 0, da.l & 0xff);
          mem_put_byte (newAddr + 1, (da.l >> 8) & 0xff);
          mem_put_byte (newAddr + 2, (da.l >> 16) & 0xff);
          mem_put_byte (newAddr + 3, (da.l >> 24) & 0xff);
        }
        da.d = convertfromq131(value);
        da.d = sin (da.d);
        value = converttoq131(da.d);
        break;
      case 0x00081420:
        break;
      case 0x00081418:
        break;
      case 0x0008141c:
        da.l = value;
        newAddr = 0x00081418;
        value2 = db.l = mem_get_si (newAddr);
        db.d = hypot (db.d, da.d) / 0.607253f;

        if (rx_big_endian)
        {
          mem_put_byte (newAddr + 0, (db.l >> 24) & 0xff);
          mem_put_byte (newAddr + 1, (db.l >> 16) & 0xff);
          mem_put_byte (newAddr + 2, (db.l >> 8) & 0xff);
          mem_put_byte (newAddr + 3, db.l & 0xff);
        }
        else
        {
          mem_put_byte (newAddr + 0, db.l & 0xff);
          mem_put_byte (newAddr + 1, (db.l >> 8) & 0xff);
          mem_put_byte (newAddr + 2, (db.l >> 16) & 0xff);
          mem_put_byte (newAddr + 3, (db.l >> 24) & 0xff);
        }
        db.l = value2;
        da.d = atan2 (da.d, db.d);
        value = da.l;
        break;
      case 0x00081428:
        break;
      case 0x0008142c:
        da.d = convertfromq131(value);
        newAddr = 0x00081428;
        value2 = mem_get_si (newAddr);
        db.d = convertfromq131(value2);
        db.d = hypot (db.d, da.d) / 0.607253f;
        db.l = converttoq131(db.d);
        if (rx_big_endian)
        {
          mem_put_byte (newAddr + 0, (db.l >> 24) & 0xff);
          mem_put_byte (newAddr + 1, (db.l >> 16) & 0xff);
          mem_put_byte (newAddr + 2, (db.l >> 8) & 0xff);
          mem_put_byte (newAddr + 3, db.l & 0xff);
        }
        else
        {
          mem_put_byte (newAddr + 0, db.l & 0xff);
          mem_put_byte (newAddr + 1, (db.l >> 8) & 0xff);
          mem_put_byte (newAddr + 2, (db.l >> 16) & 0xff);
          mem_put_byte (newAddr + 3, (db.l >> 24) & 0xff);
        }
	      db.d = convertfromq131(value2);
        da.d = atan2 (da.d, db.d);
        value = converttoq131(da.d);
        break;
    }
  }
  S ("<=");
  if (rx_big_endian)
    {
      mem_put_byte (address + 0, (value >> 24) & 0xff);
      mem_put_byte (address + 1, (value >> 16) & 0xff);
      mem_put_byte (address + 2, (value >> 8) & 0xff);
      mem_put_byte (address + 3, value & 0xff);
    }
  else
    {
      mem_put_byte (address + 0, value & 0xff);
      mem_put_byte (address + 1, (value >> 8) & 0xff);
      mem_put_byte (address + 2, (value >> 16) & 0xff);
      mem_put_byte (address + 3, (value >> 24) & 0xff);
    }
  E ();
  COUNT (1, 4);
}

void
mem_put_di (int address, unsigned long long value)
{
  S ("<=");
  if (rx_big_endian)
    {
      mem_put_byte (address + 0, (value >> 56) & 0xff);
      mem_put_byte (address + 1, (value >> 48) & 0xff);
      mem_put_byte (address + 2, (value >> 40) & 0xff);
      mem_put_byte (address + 3, (value >> 32) & 0xff);
      mem_put_byte (address + 4, (value >> 24) & 0xff);
      mem_put_byte (address + 5, (value >> 16) & 0xff);
      mem_put_byte (address + 6, (value >> 8) & 0xff);
      mem_put_byte (address + 7, value & 0xff);
    }
  else
    {
      mem_put_byte (address + 0, value & 0xff);
      mem_put_byte (address + 1, (value >> 8) & 0xff);
      mem_put_byte (address + 2, (value >> 16) & 0xff);
      mem_put_byte (address + 3, (value >> 24) & 0xff);
      mem_put_byte (address + 4, (value >> 32) & 0xff);
      mem_put_byte (address + 5, (value >> 40) & 0xff);
      mem_put_byte (address + 6, (value >> 48) & 0xff);
      mem_put_byte (address + 7, (value >> 56) & 0xff);
    }
  E ();
  COUNT (1, 8);
}

void
mem_put_context_si (unsigned long location, unsigned long offset, unsigned long value)
{
  if (rx_big_endian)
    {
      rx_context_mem_write_byte (location, offset, (value >> 24) & 0xff);
      rx_context_mem_write_byte (location, offset + 1, (value >> 16) & 0xff);
      rx_context_mem_write_byte (location, offset + 2, (value >> 8) & 0xff);
      rx_context_mem_write_byte (location, offset + 3, value & 0xff);
    }
  else
    {
      rx_context_mem_write_byte (location, offset, value & 0xff);
      rx_context_mem_write_byte (location, offset + 1, (value >> 8) & 0xff);
      rx_context_mem_write_byte (location, offset + 2, (value >> 16) & 0xff);
      rx_context_mem_write_byte (location, offset + 3, (value >> 24) & 0xff);
    }
  E ();
  COUNT (1, 4);
}

void
mem_put_context_acc (unsigned long location, unsigned long offset, unsigned long long *value)
{
  if (rx_big_endian)
    {
      rx_context_mem_write_byte (location, offset, (value[1] >> 24) & 0xff);
      rx_context_mem_write_byte (location, offset + 1, (value[1] >> 16) & 0xff);
      rx_context_mem_write_byte (location, offset + 2, (value[1] >> 8) & 0xff);
      rx_context_mem_write_byte (location, offset + 3, value[1] & 0xff);
      rx_context_mem_write_byte (location, offset + 4, (value[0] >> 56) & 0xff);
      rx_context_mem_write_byte (location, offset + 5, (value[0] >> 48) & 0xff);
      rx_context_mem_write_byte (location, offset + 6, (value[0] >> 40) & 0xff);
      rx_context_mem_write_byte (location, offset + 7, (value[0] >> 32) & 0xff);
      rx_context_mem_write_byte (location, offset + 8, (value[0] >> 24) & 0xff);
      rx_context_mem_write_byte (location, offset + 9, (value[0] >> 16) & 0xff);
      rx_context_mem_write_byte (location, offset + 10, (value[0] >> 8) & 0xff);
      rx_context_mem_write_byte (location, offset + 11, value[0] & 0xff);
    }
  else
    {
      rx_context_mem_write_byte (location, offset, value[0] & 0xff);
      rx_context_mem_write_byte (location, offset + 1, (value[0] >> 8) & 0xff);
      rx_context_mem_write_byte (location, offset + 2, (value[0] >> 16) & 0xff);
      rx_context_mem_write_byte (location, offset + 3, (value[0] >> 24) & 0xff);
      rx_context_mem_write_byte (location, offset + 4, (value[0] >> 32) & 0xff);
      rx_context_mem_write_byte (location, offset + 5, (value[0] >> 40) & 0xff);
      rx_context_mem_write_byte (location, offset + 6, (value[0] >> 48) & 0xff);
      rx_context_mem_write_byte (location, offset + 7, (value[0] >> 56) & 0xff);
      rx_context_mem_write_byte (location, offset + 8, value[1] & 0xff);
      rx_context_mem_write_byte (location, offset + 9, (value[1] >> 8) & 0xff);
      rx_context_mem_write_byte (location, offset + 10, (value[1] >> 16) & 0xff);
      rx_context_mem_write_byte (location, offset + 11, (value[1] >> 24) & 0xff);
    }
  E ();
  COUNT (1, 12);
}

void
mem_put_blk (int address, void *bufptr_void, int nbytes)
{
  unsigned char *bufptr = (unsigned char *) bufptr_void;

  S ("<=");
  if (enable_counting)
    mem_counters[1][1] += nbytes;
  while (nbytes--)
    mem_put_byte (address++, *bufptr++);
  E ();
}

unsigned char
mem_get_pc (int address)
{
  unsigned char *m = rx_mem_ptr (address, MPA_READING);
  COUNT (0, 0);
  return *m;
}

static unsigned char
mem_get_byte (unsigned int address)
{
  unsigned char *m;

  S ("=>");
  m = rx_mem_ptr (address, MPA_READING);
  switch (address)
    {
    case 0x00088264: /* SCI4.SSR */
      E();
      return 0x04; /* transmitter empty */
      break;
    default:
      if (trace)
	printf (" %02x%c", *m, mtypec (address));
      if (is_reserved_address (address))
	generate_access_exception ();
      break;
    }
  E ();
  return *m;
}

unsigned char
mem_get_qi (int address)
{
  unsigned char rv;
  S ("=>");
  rv = mem_get_byte (address);
  COUNT (0, 1);
  E ();
  return rv;
}

unsigned short
mem_get_hi (int address)
{
  unsigned short rv;
  S ("=>");
  switch (address)
    {
#ifdef CYCLE_ACCURATE
    case 0x00088126: /* TPU1.TCNT */
      rv = (regs.cycle_count - tpu_base) >> 16;
      break;
    case 0x00088136: /* TPU2.TCNT */
      rv = (regs.cycle_count - tpu_base) >> 0;
      break;
#endif

    default:
      if (rx_big_endian)
	{
	  rv = mem_get_byte (address) << 8;
	  rv |= mem_get_byte (address + 1);
	}
      else
	{
	  rv = mem_get_byte (address);
	  rv |= mem_get_byte (address + 1) << 8;
	}
    }
  COUNT (0, 2);
  E ();
  return rv;
}

unsigned long
mem_get_psi (int address)
{
  unsigned long rv;
  S ("=>");
  if (rx_big_endian)
    {
      rv = mem_get_byte (address + 2);
      rv |= mem_get_byte (address + 1) << 8;
      rv |= mem_get_byte (address) << 16;
    }
  else
    {
      rv = mem_get_byte (address);
      rv |= mem_get_byte (address + 1) << 8;
      rv |= mem_get_byte (address + 2) << 16;
    }
  COUNT (0, 3);
  E ();
  return rv;
}

unsigned long
mem_get_si (int address)
{
  unsigned long rv;
  S ("=>");
  if (rx_big_endian)
    {
      rv = mem_get_byte (address + 3);
      rv |= mem_get_byte (address + 2) << 8;
      rv |= mem_get_byte (address + 1) << 16;
      rv |= mem_get_byte (address) << 24;
    }
  else
    {
      rv = mem_get_byte (address);
      rv |= mem_get_byte (address + 1) << 8;
      rv |= mem_get_byte (address + 2) << 16;
      rv |= mem_get_byte (address + 3) << 24;
    }
  COUNT (0, 4);
  E ();
  return rv;
}

unsigned long long
mem_get_di (int address)
{
  unsigned long long rv;
  S ("=>");
  if (rx_big_endian)
    {
      rv = (unsigned long long)mem_get_byte (address + 7);
      rv |= (unsigned long long)mem_get_byte (address + 6) << 8;
      rv |= (unsigned long long)mem_get_byte (address + 5) << 16;
      rv |= (unsigned long long)mem_get_byte (address + 4) << 24;
      rv |= (unsigned long long)mem_get_byte (address + 3) << 32;
      rv |= (unsigned long long)mem_get_byte (address + 2) << 40;
      rv |= (unsigned long long)mem_get_byte (address + 1) << 48;
      rv |= (unsigned long long)mem_get_byte (address) << 56;
    }
  else
    {
      rv = (unsigned long long)mem_get_byte (address);
      rv |= (unsigned long long)mem_get_byte (address + 1) << 8;
      rv |= (unsigned long long)mem_get_byte (address + 2) << 16;
      rv |= (unsigned long long)mem_get_byte (address + 3) << 24;
      rv |= (unsigned long long)mem_get_byte (address + 4) << 32;
      rv |= (unsigned long long)mem_get_byte (address + 5) << 40;
      rv |= (unsigned long long)mem_get_byte (address + 6) << 48;
      rv |= (unsigned long long)mem_get_byte (address + 7) << 56;
    }
  COUNT (0, 8);
  E ();
  return rv;
}

unsigned long
mem_get_context_si (unsigned long location, unsigned long offset)
{
  unsigned long rv;

  if (rx_big_endian)
    {
      rv = rx_context_mem_read_byte (location, offset + 3);
      rv |= rx_context_mem_read_byte (location, offset + 2) << 8;
      rv |= rx_context_mem_read_byte (location, offset + 1) << 16;
      rv |= rx_context_mem_read_byte (location, offset) << 24;
    }
  else
    {
      rv = rx_context_mem_read_byte (location, offset);
      rv |= rx_context_mem_read_byte (location, offset + 1) << 8;
      rv |= rx_context_mem_read_byte (location, offset + 2) << 16;
      rv |= rx_context_mem_read_byte (location, offset + 3) << 24;
    }
  COUNT (0, 4);
  E ();
  return rv;
}

unsigned long long *
mem_get_context_acc (unsigned long location, unsigned long offset)
{
  unsigned long long *rv = malloc (sizeof(long long) * 2);

  if (rx_big_endian)
    {
      rv[0] = (unsigned long long)rx_context_mem_read_byte (location, offset + 11);
      rv[0] |= (unsigned long long)rx_context_mem_read_byte (location, offset + 10) << 8;
      rv[0] |= (unsigned long long)rx_context_mem_read_byte (location, offset + 9) << 16;
      rv[0] |= (unsigned long long)rx_context_mem_read_byte (location, offset + 8) << 24;
      rv[0] |= (unsigned long long)rx_context_mem_read_byte (location, offset + 7) << 32;
      rv[0] |= (unsigned long long)rx_context_mem_read_byte (location, offset + 6) << 40;
      rv[0] |= (unsigned long long)rx_context_mem_read_byte (location, offset + 5) << 48;
      rv[0] |= (unsigned long long)rx_context_mem_read_byte (location, offset + 4) << 56;
      rv[1] |= (unsigned long long)rx_context_mem_read_byte (location, offset + 3);
      rv[1] |= (unsigned long long)rx_context_mem_read_byte (location, offset + 2) << 8;
      rv[1] |= (unsigned long long)rx_context_mem_read_byte (location, offset + 1) << 16;
      rv[1] |= (unsigned long long)rx_context_mem_read_byte (location, offset) << 24;
    }
  else
    {
      rv[0] = (unsigned long long)rx_context_mem_read_byte (location, offset);
      rv[0] |= (unsigned long long)rx_context_mem_read_byte (location, offset + 1) << 8;
      rv[0] |= (unsigned long long)rx_context_mem_read_byte (location, offset + 2) << 16;
      rv[0] |= (unsigned long long)rx_context_mem_read_byte (location, offset + 3) << 24;
      rv[0] |= (unsigned long long)rx_context_mem_read_byte (location, offset + 4) << 32;
      rv[0] |= (unsigned long long)rx_context_mem_read_byte (location, offset + 5) << 40;
      rv[0] |= (unsigned long long)rx_context_mem_read_byte (location, offset + 6) << 48;
      rv[0] |= (unsigned long long)rx_context_mem_read_byte (location, offset + 7) << 56;
      rv[1] |= (unsigned long long)rx_context_mem_read_byte (location, offset + 8);
      rv[1] |= (unsigned long long)rx_context_mem_read_byte (location, offset + 9) << 8;
      rv[1] |= (unsigned long long)rx_context_mem_read_byte (location, offset + 10) << 16;
      rv[1] |= (unsigned long long)rx_context_mem_read_byte (location, offset + 11) << 24;
    }
  COUNT (0, 12);
  E ();
  return rv;
}

void
mem_get_blk (int address, void *bufptr_void, int nbytes)
{
  char *bufptr = (char *) bufptr_void;

  S ("=>");
  if (enable_counting)
    mem_counters[0][1] += nbytes;
  while (nbytes--)
    *bufptr++ = mem_get_byte (address++);
  E ();
}

int
sign_ext (int v, int bits)
{
  if (bits < 32)
    {
      v &= (1 << bits) - 1;
      if (v & (1 << (bits - 1)))
	v -= (1 << bits);
    }
  return v;
}

void
mem_set_content_type (int address, enum mem_content_type type)
{
  unsigned char *mt = rx_mem_ptr (address, MPA_CONTENT_TYPE);
  *mt = type;
}

void
mem_set_content_range (int start_address, int end_address, enum mem_content_type type)
{
  while (start_address < end_address)
    {
      int sz, ofs;
      unsigned char *mt;

      sz = end_address - start_address;
      ofs = start_address % L1_LEN;
      if (sz + ofs > L1_LEN)
	sz = L1_LEN - ofs;

      mt = rx_mem_ptr (start_address, MPA_CONTENT_TYPE);
      memset (mt, type, sz);

      start_address += sz;
    }
}

enum mem_content_type
mem_get_content_type (int address)
{
  unsigned char *mt = rx_mem_ptr (address, MPA_CONTENT_TYPE);
  return *mt;
}
