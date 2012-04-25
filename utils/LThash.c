//  libLT: a free fountain code library

//  This file is part of libLT.

//  libLT is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation version 3 of the License.

//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "LThash.h"
#include "utilMem.h"

/*****************************************************************************/

unsigned int
hashfromkey(void *ky) {	// djb2
    struct key *k = (struct key *)ky;

    unsigned char str[32];
    unsigned int hash = 5381;
    int ch;

    int offset = 0;

    // Create string
    memcpy(&str[offset], (unsigned int *) &k->w, sizeof(unsigned int));
    offset += sizeof(unsigned int);
    memcpy(&str[offset], (unsigned int *) &k->s, sizeof(unsigned int));
    offset += sizeof(unsigned int);
    memcpy(&str[offset], (double *) &k->c, sizeof(double));
    offset += sizeof(double);
    memcpy(&str[offset], (double *) &k->d, sizeof(double));
    offset += sizeof(double);
    memcpy(&str[offset], (double *) &k->epsShok, sizeof(double));
    offset += sizeof(double);

    unsigned int tmp;
    unsigned char *st = str;
    int i;

    // Compute hash
    for (i=0; i<32; i++) {
        ch = *st++;
        tmp = ((hash << 5) + hash)%UINT_MAX;
        hash = (tmp + ch)%UINT_MAX; /* hash * 33 + c */
    }
//	printf("--hashfromkey: %u\n", hash);

    return hash;
}



int
equalkeys(void *k1, void *k2) {
    return (0 == memcmp(k1,k2,sizeof(struct key)));
}


/*****************************************************************************/

DEFINE_HASHTABLE_INSERT(insert_some, struct key, struct value);
DEFINE_HASHTABLE_SEARCH(search_some, struct key, struct value);
DEFINE_HASHTABLE_REMOVE(remove_some, struct key, struct value);
DEFINE_HASHTABLE_ITERATOR_SEARCH(search_itr_some, struct key);

/*******************************************************************************/

struct value* insertEntryInLThashTable(
    struct hashtable *h,
    unsigned int w,
    unsigned int s,
    double d,
    double c,
    double epsShok) {

    struct key *k;
    struct value *v = NULL;

    // Allocate key
    k = (struct key *)chk_malloc(1, sizeof(struct key));

    k->w = w;
    k->s = s;
    k->d = d;
    k->c = c;
    k->epsShok = epsShok;

    // Allocate value
    v = (struct value *) chk_malloc(1, sizeof(struct value));

    v->cumulative = NULL;
    v->cumulative = (double *) chk_calloc(k->w, sizeof(double));

    // Save the configuration
    v->config = k;
    // Set the number of genetated symbols to zero
    v->genSymbols = 0;

    // insert in table
    if (!insert_some(h,k,v)) exit(-1); /*oom*/

    return v;
}

Tbool removeEntryFromLThashTable(
    struct hashtable *h,
    unsigned int w,
    unsigned int s,
    double d,
    double c,
    double epsShok) {

    struct key *k;
    struct value *v;

    // Allocate key
    k = (struct key *) chk_malloc(1, sizeof(struct key));

    k->w = w;
    k->s = s;
    k->d = d;
    k->c = c;
    k->epsShok = epsShok;

    v = hashtable_search(h, k);
    if (NULL == v) {
        printf("Entry to be removed not found\n");
        return False;
    }
    v = hashtable_remove(h, k);

    if (NULL != v->cumulative)
        free(v->cumulative);
    free(v);
    free(k);

    return True;
}

void destroyLThashTable(struct hashtable *h) {
    unsigned int i;
    struct entry *e = NULL, *f = NULL;
    struct entry **table = h->table;
    struct value *v = NULL;
    for (i = 0; i < h->tablelength; i++) {
        e = table[i];
        while (NULL != e) {
            f = e;
            e = e->next;
            v = f->v;
            if (NULL != v->cumulative)
                free(v->cumulative);
        }
    }
    hashtable_destroy(h, 1);
    h = NULL;
    return;
}

Tbool isEntryInLThashTable(
    struct hashtable *h,
    unsigned int w,
    unsigned int s,
    double d,
    double c,
    double epsShok) {

    // Allocate key
    struct key *k = NULL;
    k = (struct key *) chk_malloc(1, sizeof(struct key));

    k->w = w;
    k->s = s;
    k->d = d;
    k->c = c;
    k->epsShok = epsShok;

    if (NULL != search_some(h,k)) {
        free(k);
        return True;
    }
    free(k);
    return False;
}

struct value* searchEntryInLThashTable(
    struct hashtable *h,
    unsigned int w,
    unsigned int s,
    double d,
    double c,
    double epsShok) {

    struct value *v = NULL;

    // Allocate key
    struct key *k = NULL;
    k = (struct key *) chk_malloc(1, sizeof(struct key));

    k->w = w;
    k->s = s;
    k->d = d;
    k->c = c;
    k->epsShok = epsShok;

    v = search_some(h,k);

    free(k);

    return v;
}
