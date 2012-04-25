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



//*****************************************************************************
// Title      : Lib LT
// Project    : -
//*****************************************************************************
// File       : randomPA.c
// Author     : Pasquale Cataldi, Andrea Tomatis
// Company    :
// Last update: 2008/07/17
// Platform   : Ansi C
//*****************************************************************************
// Description: This file contains the random generator
//*****************************************************************************
// Revisions  :
// Date        Version  Author  Description
// 2008/07/17  0.1      PC,AT   First release
//*****************************************************************************

#include "randomPA.h"

/* Long period (> 2.0e18) random number generator of
L'Ecuyer with Bays­Durham shuffle and added safeguards. Returns a
uniform random deviate between 0.0 and 1.0 (exclusive of the
endpoint values). Call with idum a negative integer to initialize;
thereafter, do not alter idum between successive deviates in a
sequence. RNMX should approximate the largest floating value that
is less than 1. -- */

// Random setting


void initRandom(randomPA *r, unsigned long seed) {
    int j;
    r->idum2	= 123456789;
    r->idum		= 0;
    r->iy		= 0;
    r->seed_ 	= seed;

    if (r->seed_ == 0) {
        time_t t;
        time(&t);
        r->seed_ = (unsigned long)t;
    }
    r->idum = r->seed_;

    for (j=NTAB+7; j>=0; j--) {	// Load the shuffle table (after 8 warm­ups).

        long k = (r->idum)/IQ1;

        r->idum = IA1*(r->idum-k*IQ1)-k*IR1;
        if (r->idum < 0)
            r->idum += IM1;
        if (j < NTAB)
            r->iv[j] = r->idum;
    }
    r->iy=r->iv[0];

    r->set_init = True;
}
// Random Shift Register

double ran2(randomPA *r) {
    int j;
    long k;
    double temp;

    if (r->set_init == False) {
        printf("Error: Random has not been initialized by the method 'init(long)'!\n");
        exit(1);
    }

    k		= (r->idum)/IQ1;
    r->idum	= IA1*(r->idum-k*IQ1)-k*IR1;	// Compute idum=(IA1*idum) % IM1 without overflows by Schrage's method.
    if (r->idum < 0)
        r->idum += IM1;
    k		= r->idum2/IQ2;
    r->idum2	= IA2*(r->idum2-k*IQ2)-k*IR2;	// Compute idum2=(IA2*idum) % IM2 likewise.
    if (r->idum2 < 0)
        r->idum2 += IM2;
    j	= r->iy/NDIV;
    r->iy	= r->iv[j]-r->idum2;			// Here idum is shuffled, idum and idum2 are combined to generate output.
    r->iv[j] = r->idum;
    if (r->iy < 1)
        r->iy += IMM1;
    if ((temp=AM*r->iy) > RNMX)
        return RNMX;					// Because users don't expect endpoint values.
    else
        return temp;
}

// Return a real value between [0, 1)

double real(randomPA *r) {
    double t;
    t = ran2(r);
    return t;
}


// Return a long between 0 and range-1

long integer(randomPA *r, long range) {
    double t;
    t = real(r);
    return((long)(t*(double)range));
}

// Return a random boolean value (True or False)

Tbool boolrandom(randomPA *r) {
    double t=real(r);

    if (t>0.5)
        return True;
    else
        return False;
}

// Return a real value with a gaussian distribution

double gaussian(randomPA *r, double variance) {
    double returnvalue=0;
    double k;
    int x;

    k = sqrt(variance/2.0);
    // add 24 uniform RV to obtain a simulation of normality
    for (x=0; x<24; x++)
        returnvalue += ran2(r);

    return k*(returnvalue-0.5*24);
}
