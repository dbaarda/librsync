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

/** Initialize a bucket list element. */
void bucket_init(bucket_t *b, bucket_t *n, void *e)
{
    b->next = n;
    b->entry = e;
}

/** Destroy a bucket list element. */
void bucket_done(bucket_t *b)
{
    if (b->next) {
        bucket_done(b->next);
        free(b->next);
#ifndef NDEBUG
        b->next = NULL;
#endif                          /* NDEBUG */
    }
#ifndef NDEBUG
    b->entry = NULL;
#endif                          /* NDEBUG */
}

/** Add and return an entry to the end of a bucket list. */
bucket_t *bucket_add(bucket_t *b, void *e)
{
    /* Note we add to the end so first added is first found. */
    if (!b->entry) {
	b->entry = e;
    } else if (!b->next) {
	b = b->next = malloc(sizeof(bucket_t));
	bucket_init(b, NULL, e);
    } else {
	b = bucket_add(b->next, e);
    }
    return b;
}

void hashtable_init(hashtable_t *t, int size, hash_f hash, cmp_f cmp)
{
    assert(t != NULL);
    t->size = size ? size : 1;
    t->count = 0;
    t->table = calloc(t->size, sizeof(bucket_t));
    t->hash = hash;
    t->cmp = cmp;
}

void hashtable_done(hashtable_t *t)
{
    assert(t != NULL);
    int i;

    for (i = 0; i < t->size; i++)
        bucket_done(&t->table[i]);
    free(t->table);
#ifndef NDEBUG
    t->size = 0;
    t->count = 0;
    t->hash = NULL;
    t->cmp = NULL;
#endif                          /* NDEBUG */
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
