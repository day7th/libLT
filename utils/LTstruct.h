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
// File       : Struct.h
// Author     : Pasquale Cataldi, Andrea Tomatis
// Company    :
// Last update: 2008/07/17
// Platform   : Ansi C
//*****************************************************************************
// Description: This file contains all the structures utilized in the software
//*****************************************************************************
// Revisions  :
// Date        Version  Author  Description
// 2008/07/17  0.1      PC,AT   First release
// 2008/10/25  0.2      PC      Added THavgDegree in LTencoder struct
// 2009/02/19  0.3      PC      Changed "head" (the list of buckets) "to bucketList"
// 2009/02/22  0.4	    PC      Changed "NumSymbols" to "NumRecSymbols"
//                              Changed "RXsymbols" to "RXsymbList"
// 2009/05/08  0.5      PC      Moved some structures to their corresponding .h files
//*****************************************************************************

#ifndef STRUCT_H_
#define STRUCT_H_

typedef enum {
    False,
    True
} Tbool;

typedef enum {
    Shokrollahi,
    Luby
} DegDistr;

typedef struct {
    unsigned int k;     // Number of input symbols
    unsigned int w;     // Number of input symbols in the window
    unsigned int s;     // Number of input symbols for the shift
    unsigned int N;     // Number of generated encoded-symbols
    double c;           // Luby RSD parameter
    double delta;       // Luby RSD parameter
    double epsShok;     // Shokrollahi distribution parameter
} Configuration;

typedef struct {
    unsigned int id;
    unsigned int deg;
    unsigned int *header;
    char *info;
    int symbLen;
} Symbol;

typedef struct {
    double *cumulative;
    Symbol *symbols;
    DegDistr codetype;	// Type of the code: RSD or Shokrollahi
    unsigned long seed;
    Configuration conf;
} Mgenerator;


#endif /*STRUCT_H_*/
