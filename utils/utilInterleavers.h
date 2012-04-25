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
// File       : utilInterleavers.h
// Author     : Pasquale Cataldi
// Company    :
// Last update: 2010/12/30
// Platform   : Ansi C
//*****************************************************************************
// Description: This file contains the declarations of the functions
//				that implement interleavers and de-interleavers
//*****************************************************************************
// Revisions  :
// Date        Version  Author  Description
// 2010/12/30  0.1      PC      Created this file and moved the declarations
//                              from sharedMethods.h
//*****************************************************************************

#ifndef UTILINTERLEAVERS_H_
#define UTILINTERLEAVERS_H_

void interleaver(char *input, char **interleaved, unsigned int vectlen, unsigned int blklen);
void deinterleaver(char *interleaved, char**output, unsigned int vectlen, unsigned int blklen);

void interleaver2(char **vect, unsigned int vectlen, unsigned int blklen);
void deinterleaver2(char **vect, unsigned int vectlen, unsigned int blklen);

#endif /* UTILINTERLEAVERS_H_ */
