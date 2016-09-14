/*= -*- c-basic-offset: 4; indent-tabs-mode: nil; -*-
 *
 * hashtable.c -- a generic hashtable implementation.
 *
 * Copyright (C) 2016 by Donovan Baarda <abo@minkirri.apana.org.au>
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
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "hashtable.h"

/** Allocate and initialize a bucket list element. */
bucket_t *bucket_new(bucket_t *n, void *e)
{
    bucket_t *b = malloc(sizeof(bucket_t));

    if (b) {
        b->next = n;
        b->entry = e;
    }
    return b;
}

/** Destroy and free a bucket list element. */
void bucket_free(bucket_t *b)
{
    if (b) {
        bucket_free(b->next);
        free(b);
    }
}

/** Add and return an entry to the end of a bucket list. */
bucket_t *bucket_add(bucket_t *b, void *e)
{
    /* Note we add to the end so first added is first found. */
    if (!b->entry) {
        b->entry = e;
    } else if (b->next) {
        b = bucket_add(b->next, e);
    } else {
        b = b->next = bucket_new(NULL, e);
    }
    return b;
}

hashtable_t *hashtable_new(int size, hash_f hash, cmp_f cmp)
{
    hashtable_t *t;

    /* Use min size of 1. */
    size = size ? size : 1;
    if (!(t = calloc(1, sizeof(hashtable_t)+ size * sizeof(bucket_t))))
        return NULL;
    t->size = size;
    t->count = 0;
    t->hash = hash;
    t->cmp = cmp;
    return t;
}

void hashtable_free(hashtable_t *t)
{
    int i;

    if (t) {
        for (i = 0; i < t->size; i++)
            bucket_free(t->table[i].next);
        free(t);
    }
}

static inline unsigned mix32(unsigned int h)
{
    /* MurmurHash3 finalization mix function. */
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    return h;
}

static inline bucket_t *hashtable_getb(hashtable_t *t, void *k)
{
    return &t->table[mix32(t->hash(k)) & (t->size - 1)];
}

void *hashtable_add(hashtable_t *t, void *e)
{
    assert(e != NULL);
    bucket_t *b = hashtable_getb(t, e);

    t->count++;
    return bucket_add(b, e)->entry;
}

void *hashtable_find(hashtable_t *t, void *k)
{
    assert(k != NULL);
    void *e;
    bucket_t *b = hashtable_getb(t, k);

    do {
        if (!(e = b->entry) || !t->cmp(k, e))
            return e;
    } while ((b = b->next));
    return NULL;
}

void *hashtable_iter(hashtable_iter_t *i, hashtable_t *t)
{
    assert(i != NULL);
    assert(t != NULL);
    i->htable = t;
    i->bucket = &t->table[0];
    i->index = 1;
    return hashtable_next(i);
}

void *hashtable_next(hashtable_iter_t *i)
{
    assert(i->htable != NULL);
    assert(i->index <= i->htable->size);
    hashtable_t *t = i->htable;
    void *e;

    while (i->bucket) {
        e = i->bucket->entry;
        i->bucket = i->bucket->next;
        if (!i->bucket && i->index < t->size)
            i->bucket = &t->table[i->index++];
        if (e)
            return e;
    }
    return NULL;
}
