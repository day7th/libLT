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
// File       : randomPA.h
// Author     : Pasquale Cataldi, Andrea Tomatis
// Company    :
// Last update: 2008/07/17
// Platform   : Ansi C
//*****************************************************************************
// Description: This file contains the random generator declarations
//*****************************************************************************
// Revisions  :
// Date        Version  Author  Description
// 2008/07/17  0.1      PC,AT   First release
//*****************************************************************************

#ifndef RANDOMPA_H_
#define RANDOMPA_H_

#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <time.h>
#include <limits.h>

#include "../utils/LTstruct.h"

#define IM1 2147483563
#define IM2 2147483399
#define AM (1.0/IM1)
#define IMM1 (IM1-1)
#define IA1 40014
#define IA2 40692
#define IQ1 53668
#define IQ2 52774
#define IR1 12211
#define IR2 3791
#define NTAB 32
#define NDIV (1+IMM1/NTAB)
#define EPS DBL_MIN
#define RNMX (1.0-EPS)

typedef struct {
    long idum2;
    long idum;
    long iy;
    long iv[NTAB];
    unsigned long seed_;
    unsigned memory;
    Tbool set_init;

} randomPA;

void initRandom(randomPA *r, unsigned long seed);
double ran2(randomPA *r);
double real(randomPA *r) ;
long integer(randomPA *r, long range);
Tbool boolrandom(randomPA *r);
double gaussian(randomPA *r, double variance);


#endif /*RANDOMPA_H_*/
