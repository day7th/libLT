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


#ifndef LTHASH_H_
#define LTHASH_H_

#include "../hashtable/libHash_C.h"
#include "LTstruct.h"
#include "../random/randomPA.h"

/*****************************************************************************/
struct key {
    unsigned int w;		// window size (in symbols)
    unsigned int s;		// shift size (in symbols)
    double d;			// lt_delta (RSD distribution)
    double c;			// lt_c (RSD distribution)
    double epsShok;		// epsilon (Shokrollahi's distribution)
};

struct value {
    double *cumulative;		// cumulative degree distribution
    struct key *config;	// (w,s,d,c,epsShok)

    unsigned int genSymbols;// number of generated symbs with this configuration
    unsigned long confSeed;		// random seed for the this configuration
    randomPA randomGenerator;	// random generator for this configuration
};

unsigned int hashfromkey(void *ky);

int equalkeys(void *k1, void *k2);


struct value* insertEntryInLThashTable(
    struct hashtable *h,
    unsigned int w,
    unsigned int s,
    double d,
    double c,
    double epsShok);

Tbool removeEntryFromLThashTable(
    struct hashtable *h,
    unsigned int w,
    unsigned int s,
    double d,
    double c,
    double epsShok);

void destroyLThashTable(struct hashtable *h);

Tbool isEntryInLThashTable(
    struct hashtable *h,
    unsigned int w,
    unsigned int s,
    double d,
    double c,
    double epsShok);

struct value* searchEntryInLThashTable(
    struct hashtable *h,
    unsigned int w,
    unsigned int s,
    double d,
    double c,
    double epsShok);

#endif /*LTHASH_H_*/
