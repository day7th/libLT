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
// File       : utilInterleavers.c
// Author     : Pasquale Cataldi
// Company    :
// Last update: 2010/12/30
// Platform   : Ansi C
//*****************************************************************************
// Description: This file contains the implementation of the functions
//				for interleavers and de-interleavers
//*****************************************************************************
// Revisions  :
// Date        Version  Author  Description
// 2010/12/30  0.1      PC      Created this file and moved the declarations
//                              from sharedMethods.c
//*****************************************************************************

// ===========================================================================
//					INTERLEAVERS
// ===========================================================================

#include "utilInterleavers.h"
#include "utilErr.h"
#include "utilMem.h"

#include <string.h>
#include <stdlib.h>

void interleaver(char *input, char **interleaved, unsigned int vectlen, unsigned int blklen) {
    int i, j;
    int z = 0;
    unsigned int Nblks = vectlen/blklen;
    if (vectlen%blklen) {
        print_error("Warning: Interleaving not done because the input vector cannot be evenly divided in blocks as specified!\n");
        memcpy(input, (*interleaved), vectlen*sizeof(char));
    } else {
        for (i=0; i<blklen; i++) {
            for (j=0; j<Nblks; j++) {
                (*interleaved)[z] = input[j*blklen+i];
                z++;
            }
        }
    }
    return;
}


void deinterleaver(char *interleaved, char **output, unsigned int vectlen, unsigned int blklen) {
    int i, j;
    int z = 0;
    unsigned int Nblks = vectlen/blklen;
    if (vectlen%blklen) {
        print_error("Warning: De-interleaving not done because the input vector cannot be evenly divided in blocks as specified!\n");
        memcpy(interleaved, (*output), vectlen*sizeof(char));
    } else {
        for (i=0; i<Nblks; i++) {
            for (j=0; j<blklen; j++) {
                (*output)[z] = interleaved[j*Nblks+i];
                z++;
            }
        }
    }
    return;
}

void interleaver2(char **vect, unsigned int vectlen, unsigned int blklen) {
    int i, j;
    int z = 0;
    unsigned int Nblks = vectlen/blklen;
    char *tmp = NULL;

    if (vectlen%blklen) {
        print_error("Warning: Interleaving not done because the input vector cannot be evenly divided in blocks as specified!\n");
    } else {
        tmp = (char*) chk_malloc(vectlen, sizeof(char));
        for (i=0; i<blklen; i++) {
            for (j=0; j<Nblks; j++) {
                tmp[z] = (*vect)[j*blklen+i];
                z++;
            }
        }
        memcpy((*vect), tmp, vectlen*sizeof(char));
        free(tmp);
    }
    return;
}


void deinterleaver2(char **vect, unsigned int vectlen, unsigned int blklen) {
    int i, j;
    int z = 0;
    unsigned int Nblks = vectlen/blklen;
    char *tmp = NULL;

    if (vectlen%blklen) {
        print_error("Warning: Deinterleaving not done because the input vector cannot be evenly divided in blocks as specified!\n");
    } else {
        tmp = (char*) chk_malloc(vectlen, sizeof(char));
        for (i=0; i<Nblks; i++) {
            for (j=0; j<blklen; j++) {
                tmp[z] = (*vect)[j*Nblks+i];
                z++;
            }
        }
        memcpy((*vect), tmp, vectlen*sizeof(char));
        free(tmp);
    }
    return;
}

