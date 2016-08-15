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
#include "rollsum.h"

#define DO1(buf,i)  {s1 += buf[i]*buf[i]; s2 += s1;}
#define DO2(buf,i)  DO1(buf,i); DO1(buf,i+1);
#define DO4(buf,i)  DO2(buf,i); DO2(buf,i+2);
#define DO8(buf,i)  DO4(buf,i); DO4(buf,i+4);
#define DO16(buf)   DO8(buf,0); DO8(buf,8);

void RollsumUpdate(Rollsum *sum,const unsigned char *buf,unsigned int len) {
    /* ANSI C says no overflow for unsigned. Adler32 goes to extra effort to
     avoid overflow for its mod prime, which we don't have. */
    unsigned int s1 = sum->s1;
    unsigned int s2 = sum->s2;

    sum->count+=len;                   /* increment sum count */
    while (len >= 16) {
        DO16(buf);
        buf += 16;
        len -= 16;
    }
    while (len != 0) {
        s1 += *buf * *buf;
        s2 += s1;
        buf++;
        len--;
    }
    sum->s1=s1;
    sum->s2=s2;
}
