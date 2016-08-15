/*= -*- c-basic-offset: 4; indent-tabs-mode: nil; -*-
 *
 * rollsum -- the librsync rolling checksum
 *
 * Copyright (C) 2003 by Donovan Baarda <abo@minkirri.apana.org.au>
 * based on work, Copyright (C) 2000, 2001 by Martin Pool <mbp@sourcefrog.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#ifndef _ROLLSUM_H_
#define _ROLLSUM_H_

/* Adler32 style initial s1 value to ensure length is encoded in s2. */
#define ROLLSUM_INIT 1

/* the Rollsum struct type*/

/** \private */
typedef struct _Rollsum {
    unsigned long count;               /* count of bytes included in sum */
    unsigned int s1;                   /* s1 part of sum */
    unsigned int s2;                   /* s2 part of sum */
} Rollsum;

void RollsumUpdate(Rollsum *sum,const unsigned char *buf,unsigned int len);

/* static inline implementations of simple routines */
static inline void RollsumInit(Rollsum *sum)
{
    sum->count = 0;
    sum->s1 = ROLLSUM_INIT;
    sum->s2 = 0;
}

static inline void RollsumRotate(Rollsum *sum, unsigned char out, unsigned char in)
{
    unsigned int in2 = in * in;
    unsigned int out2 = out * out;
    sum->s1 += in2 - out2;
    sum->s2 += sum->s1 - sum->count*out2 - ROLLSUM_INIT;
}

static inline void RollsumRollin(Rollsum *sum, unsigned char c)
{
    unsigned int c2 = c * c;
    sum->s1 += c2;
    sum->s2 += sum->s1;
    sum->count++;
}

static inline void RollsumRollout(Rollsum *sum, unsigned char c)
{
    unsigned int c2 = c * c;
    sum->s1 -= c2;
    sum->s2 -= sum->count*c2 - ROLLSUM_INIT;
    sum->count--;
}

static inline unsigned long RollsumDigest(Rollsum *sum)
{
     return ((unsigned long)sum->s2 << 16) | ((unsigned long)sum->s1 & 0xffff);
}

#endif /* _ROLLSUM_H_ */
