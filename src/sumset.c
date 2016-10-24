/*= -*- c-basic-offset: 4; indent-tabs-mode: nil; -*-
 *
 * librsync -- library for network deltas
 *
 * Copyright (C) 1999, 2000, 2001 by Martin Pool <mbp@sourcefrog.net>
 * Copyright (C) 1999 by Andrew Tridgell
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

#include "config.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "librsync.h"
#include "sumset.h"
#include "util.h"
#include "trace.h"

const int RS_MD4_SUM_LENGTH = 16;
const int RS_BLAKE2_SUM_LENGTH = 32;

void rs_block_sig_init(rs_block_sig_t *sig, rs_weak_sum_t weak_sum, rs_strong_sum_t *strong_sum,
                       int strong_len)
{
    sig->weak_sum = weak_sum;
    memcpy(sig->strong_sum, strong_sum, strong_len);
}

rs_result rs_signature_init(rs_signature_t *sig, int magic, int block_len, int strong_len, rs_long_t sig_fsize)
{
    int max_strong_len;

    /* Check and set default arguments. */
    magic = magic ? magic : RS_BLAKE2_SIG_MAGIC;
    switch (magic) {
    case RS_BLAKE2_SIG_MAGIC:
        max_strong_len = RS_BLAKE2_SUM_LENGTH;
        break;
    case RS_MD4_SIG_MAGIC:
        max_strong_len = RS_MD4_SUM_LENGTH;
        break;
    default:
        rs_error("invalid magic %#x", magic);
        return RS_BAD_MAGIC;
    }
    strong_len = strong_len ? strong_len : max_strong_len;
    if (strong_len < 1 || max_strong_len < strong_len) {
        rs_error("invalid strong_sum_len %d for magic %#x", strong_len, magic);
        return RS_PARAM_ERROR;
    }
    /* Set attributes from args. */
    sig->magic = magic;
    sig->block_len = block_len;
    sig->strong_sum_len = strong_len;
    sig->count = 0;
    /* Calculate the number of blocks if we have the signature file size. */
    /* Magic+header is 12 bytes, each block thereafter is 4 bytes weak_sum+strong_sum_len bytes */
    sig->size = (int)(sig_fsize ? (sig_fsize - 12) / (4 + strong_len) : 0);
    sig->block_sigs = NULL;
    sig->buckets = NULL;
    sig->strong_sums = NULL;
    if (sig->size)
        sig->block_sigs = rs_alloc(sig->size * sizeof(rs_block_sig_t), "signature->block_sigs");
    rs_signature_check(sig);
    return RS_DONE;
}

void rs_signature_done(rs_signature_t *sig)
{
    free(sig->block_sigs);
    free(sig->buckets);
    free(sig->strong_sums);
    rs_bzero(sig, sizeof(*sig));
}

rs_block_sig_t *rs_signature_add_block(rs_signature_t *sig, rs_weak_sum_t weak_sum, rs_strong_sum_t *strong_sum)
{
    rs_signature_check(sig);
    /* If block_sigs is full, allocate more space. */
    if (sig->count == sig->size) {
        sig->size = sig->size ? sig->size * 2 : 16;
        sig->block_sigs = rs_realloc(sig->block_sigs, sig->size * sizeof(rs_block_sig_t), "signature->block_sigs");
    }
    rs_block_sig_t *b = &sig->block_sigs[sig->count++];
    rs_block_sig_init(b, weak_sum, strong_sum, sig->strong_sum_len);
    return b;
}

void rs_free_sumset(rs_signature_t *psums)
{
    rs_signature_done(psums);
    free(psums);
}

void rs_sumset_dump(rs_signature_t const *sums)
{
    int i;
    char strong_hex[RS_MAX_STRONG_SUM_LENGTH * 3];

    rs_log(RS_LOG_INFO, "sumset info: magic=%x, block_len=%d, block_num=%d", sums->magic, sums->block_len, sums->count);

    for (i = 0; i < sums->count; i++) {
        rs_hexify(strong_hex, sums->block_sigs[i].strong_sum, sums->strong_sum_len);
        rs_log(RS_LOG_INFO, "sum %6d: weak=%08x, strong=%s", i, sums->block_sigs[i].weak_sum, strong_hex);
    }
}
