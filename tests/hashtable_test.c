/*= -*- c-basic-offset: 4; indent-tabs-mode: nil; -*-
 *
 * hashtable_test -- tests for the hashtable.
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
#include "hashtable.h"

/* Key type for the hashtable. */
typedef int key_t;
void key_init(key_t *k, int i)
{
    /* This is chosen to cause bad key collisions and clustering. */
    *k = (i / 2) * (i / 2);
}

int key_hash(const key_t *k)
{
    return *k;
}

int key_cmp(key_t *k, const key_t *o)
{
    return *k - *o;
}

/* Entry type for values in hashtable. */
typedef struct entry {
    key_t key;                  /* Inherit from key_t. */
    int value;
} entry_t;

void entry_init(entry_t *e, int i)
{
    key_init(&e->key, i);
    e->value = i;
}

/* Match type for finding matching entries in hashtable.
 *
 * This demonstrates using deferred calculation and comparison of the
 * expected value only when the key matches. */
typedef struct match {
    key_t key;                  /* Inherit from key_t. */
    int value;
    int source;
} match_t;

void match_init(match_t *m, int i)
{
    key_init(&m->key, i);
    m->value = 0;
    m->source = i;
}

int match_cmp(match_t *m, const entry_t *e)
{
    int ans = key_cmp(&m->key, &e->key);
    /* Calculate and compare value if key matches */
    if (ans == 0) {
        if (m->value != m->source)
            m->value = m->source;
        ans = m->value - e->value;
    }
    return ans;
}

/* Test driver for hashtable. */
int main(int argc, char **argv)
{
    hashtable_t t;
    entry_t entry[256];
    entry_t e;
    match_t m;
    int i;

    entry_init(&e, 0);
    for (i = 0; i < 256; i++)
        entry_init(&entry[i], i);

    /* Test hashtable_init() */
    hashtable_init(&t, 256, (hash_f)&key_hash, (cmp_f)&match_cmp);
    assert(t.size >= 256);
    assert(t.count == 0);
    assert(t.table != NULL);
    assert(t.hash == (hash_f)&key_hash);
    assert(t.cmp == (cmp_f)&match_cmp);

    /* Test hashtable_add() */
    assert(hashtable_add(&t, &e) == &e);        /* Added duplicated copy. */
    assert(hashtable_add(&t, &entry[0]) == &entry[0]);  /* Added duplicated instance. */
    for (i = 0; i < 256; i++)
        assert(hashtable_add(&t, &entry[i]) == &entry[i]);
    assert(t.count == 258);

    /* Test hashtable_find() */
    match_init(&m, 0);
    assert(hashtable_find(&t, &m) == &e);       /* Finds first duplicate added. */
    assert(m.value == m.source);        /* match_cmp() updated m.value. */
    for (i = 1; i < 256; i++) {
        match_init(&m, i);
        assert(hashtable_find(&t, &m) == &entry[i]);
        assert(m.value == m.source);    /* match_cmp() updated m.value. */
    }
    match_init(&m, 256);
    assert(hashtable_find(&t, &m) == NULL);     /* Find missing entry. */
    assert(m.value == 0);       /* match_cmp() didn't update m.value. */

    /* Test hashtable iterators */
    entry_t *p;
    hashtable_iter_t iter;
    int count = 0;
    for (p = hashtable_iter(&iter, &t); p != NULL; p = hashtable_next(&iter)) {
        assert(p == &e || (&entry[0] <= p && p <= &entry[255]));
        count++;
    }
    assert(count == 258);

    return 0;
}
