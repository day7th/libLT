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
// File       : utilMem.c
// Author     : Pasquale Cataldi
// Company    :
// Last update: 2010/12/30
// Platform   : Ansi C
//*****************************************************************************
// Description: This file contains the implementation of the functions
//				for memory allocation
//*****************************************************************************
// Revisions  :
// Date        Version  Author  Description
// 2010/12/30  0.1      PC      First version.
//*****************************************************************************

#include "utilMem.h"
#include "utilErr.h"

#include <stdlib.h>

void *chk_calloc(
    unsigned n,     /* Number of elements */
    unsigned size   /* Size of each element */
) {
    void *p;

    p = calloc(n,size);

    if (p==0) {
        print_error("chk_calloc: Ran out of memory (while trying to allocate %d bytes)\n", n*size);
        exit(EXIT_FAILURE);
    }
    return p;
}

void *chk_malloc(
    unsigned n,     /* Number of elements */
    unsigned size   /* Size of each element */
) {
    void *p;

    p = malloc(n*size);

    if (p==0) {
        print_error("chk_malloc: Ran out of memory (while trying to allocate %d bytes)\n", n*size);
        exit(EXIT_FAILURE);
    }
    return p;
}

void *chk_realloc(
    void * ptr,    /* Pointer to the allocated block */
    unsigned n,    /* Number of elements */
    unsigned size  /* Size of each element */
) {
    void *p;

    p = realloc(ptr, n*size);

    if (p==0) {
        print_error("chk_realloc: Ran out of memory (while trying to (re)allocating %d bytes)\n", n*size);
        exit(EXIT_FAILURE);
    }
    return p;
}

