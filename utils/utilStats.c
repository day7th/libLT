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
// File       : utilStats.c
// Author     : Pasquale Cataldi
// Company    :
// Last update: 2010/12/30
// Platform   : Ansi C
//*****************************************************************************
// Description: This file contains the implementation of the functions
//				to compute statistics
//*****************************************************************************
// Revisions  :
// Date        Version  Author  Description
// 2010/12/30  0.1      PC      First release
//*****************************************************************************

#include "utilStats.h"

#include <math.h>

// it returns the standard deviation
double computeStdDevOnTheFly(unsigned int n, // current number of elements
                             double *mean, double *M2, double *variance,
                             double x) {
    double delta = x - *mean;
    *mean += delta/n;
    *M2 += delta*(x - *mean);
    if (n<2)
        *variance = 0.0;
    else
        *variance = *M2/(n-1.0);
    return sqrt(*variance);
}
