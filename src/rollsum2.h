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
#ifndef _ROLLSUM2_H_
#define _ROLLSUM2_H_

#include <stddef.h>
#include <stdint.h>

/* Adler32 style initial s1 value to ensure length is encoded in s2. */
#define ROLLSUM2_INIT 1

/* the Rollsum2 struct type*/

/** \private */
typedef struct _Rollsum2 {
    size_t count;      /* count of bytes included in sum */
    uint_fast16_t s1;  /* s1 part of sum */
    uint_fast16_t s2;  /* s2 part of sum */
} Rollsum2;

void Rollsum2Update(Rollsum2 *sum, const unsigned char *buf, size_t len);

/* static inline implementations of simple routines */
static inline void Rollsum2Init(Rollsum2 *sum)
{
    sum->count = 0;
    sum->s1 = ROLLSUM2_INIT;
    sum->s2 = 0;
}

static inline void Rollsum2Rotate(Rollsum2 *sum, unsigned char out, unsigned char in)
{
    uint_fast16_t in2 = in * in;
    uint_fast16_t out2 = out * out;
    sum->s1 += in2 - out2;
    sum->s2 += sum->s1 - sum->count*out2 - ROLLSUM2_INIT;
}

static inline void Rollsum2Rollin(Rollsum2 *sum, unsigned char in)
{
    uint_fast16_t in2 = in * in;
    sum->s1 += in2;
    sum->s2 += sum->s1;
    sum->count++;
}

static inline void Rollsum2Rollout(Rollsum2 *sum, unsigned char out)
{
    uint_fast16_t out2 = out * out;
    sum->s1 -= out2;
    sum->s2 -= sum->count*out2 + ROLLSUM2_INIT;
    sum->count--;
}

static inline uint32_t Rollsum2Digest(Rollsum2 *sum)
{
    return ((uint32_t)sum->s2 << 16) | ((uint32_t)sum->s1 & 0xffff);
}

#endif /* _ROLLSUM2_H_ */
