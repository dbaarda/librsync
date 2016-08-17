/*= -*- c-basic-offset: 4; indent-tabs-mode: nil; -*-
 *
 * rollsum_test -- tests for the librsync rolling checksum.
 *
 * Copyright (C) 2003 by Donovan Baarda <abo@minkirri.apana.org.au>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include "rollsum2.h"

/*
 * Test driver for rollsum.
 */
int main(int argc, char **argv)
{
    Rollsum2 r;
    int i;
    unsigned char buf[256];

    /* Test Rollsum2Init() */
    Rollsum2Init(&r);
    assert(r.count == 0);
    assert(r.s1 == 1);
    assert(r.s2 == 0);
    assert(Rollsum2Digest(&r) == 0x00000001);

    /* Test Rollsum2Rollin() */
    Rollsum2Rollin(&r, 0);  /* [0] */
    assert(r.count == 1);
    assert(Rollsum2Digest(&r) == 0x00010001);
    Rollsum2Rollin(&r, 1);
    Rollsum2Rollin(&r, 2);
    Rollsum2Rollin(&r, 3);  /* [0,1,2,3] */
    assert(r.count == 4);
    assert(Rollsum2Digest(&r) == 0x0018000f);

    /* Test Rollsum2Rotate() */
    Rollsum2Rotate(&r,0,4);  /* [1,2,3,4] */
    assert(r.count == 4);
    assert(Rollsum2Digest(&r) == 0x0036001f);
    Rollsum2Rotate(&r,1,5);
    Rollsum2Rotate(&r,2,6);
    Rollsum2Rotate(&r,3,7);  /* [4,5,6,7] */
    assert(r.count == 4);
    assert(Rollsum2Digest(&r) == 0x0108007f);

    /* Test Rollsum2Rollout() */
    Rollsum2Rollout(&r, 4);  /* [5,6,7] */
    assert(r.count == 3);
    assert(Rollsum2Digest(&r) == 0x00c7006f);
    Rollsum2Rollout(&r, 5);
    Rollsum2Rollout(&r, 6);
    Rollsum2Rollout(&r, 7); /* [] */
    assert(r.count == 0);
    assert(Rollsum2Digest(&r) == 0x00000001);

    /* Test Rollsum2Update() */
    for (i=0; i < 256; i++)
        buf[i] = i;
    Rollsum2Update(&r, buf, 256);
    assert(Rollsum2Digest(&r) == 0x4100d581);
    return 0;
}
