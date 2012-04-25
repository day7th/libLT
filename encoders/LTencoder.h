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
// File       : LTencoder.h
// Author     : Pasquale Cataldi, Andrea Tomatis
// Company    :
// Last update: 2008/07/17
// Platform   : Ansi C
//*****************************************************************************
// Description: This file contains the LT encoder declarations
//*****************************************************************************
// Revisions  :
// Date        Version  Author  Description
// 2008/07/17  0.1      PC,AT   First release
// 2008/07/28  0.2      PC      Changed the name of the encode function
//								to LTencode (for compatibility)
// 2008/11/20  0.3      PC      Added "initEncoder_nw" and "reSeedEncoder_nw" in
//                              order to use the "create_headers_nw" function
//                              The functions "initEncoder" and "reSeedEncoder"
//                              now return void instead of the not exloited int
// 2009/05/08  0.4      PC      Moved the structure definition in this file in
//                              order to add modularity
//*****************************************************************************

#ifndef ENCODER_H_
#define ENCODER_H_

#include "../utils/LTstruct.h"
#include "../xor/xor.h"

typedef struct {
    Mgenerator matrix;
    int symbLen;
    char *output;
    XorType xorType;
    double thAvgDegree;	// Theorical Avg Degree from distribution
    double genAvgDegree;	// Generated Avg Degree from distribution
} LTencoder;

LTencoder* mallocEncoder(void);

void freeEncoder(LTencoder *encoder);

void initEncoder(LTencoder *encoder, unsigned long seed, int symblen, unsigned int k, unsigned int w, unsigned int s, double lt_delta, double lt_c, double raptor_eps, unsigned int N, DegDistr code);
void initEncoder_LT(LTencoder *encoder, unsigned long seed, int symblen, unsigned int k, unsigned int w, unsigned int s, double lt_delta, double lt_c, unsigned int N);
void initEncoder_Raptor(LTencoder *encoder, unsigned long seed, int symblen, unsigned int k, unsigned int w, unsigned int s, double raptor_eps, unsigned int N);
void reSeedEncoder(LTencoder *encoder, unsigned long seed);

void initEncoder_nw(LTencoder *encoder, unsigned long seed, int symblen, unsigned int k, unsigned int w, unsigned int s, double lt_delta, double lt_c, double raptor_eps, unsigned int N, DegDistr code, unsigned int nw, unsigned int Ns);
void initEncoder_LT_nw(LTencoder *encoder, unsigned long seed, int symblen, unsigned int k, unsigned int w, unsigned int s, double lt_delta, double lt_c, unsigned int N, unsigned int nw, unsigned int Ns);
void initEncoder_Raptor_nw(LTencoder *encoder, unsigned long seed, int symblen, unsigned int k, unsigned int w, unsigned int s, double raptor_eps, unsigned int N, unsigned int nw, unsigned int Ns);
void reSeedEncoder_nw(LTencoder *encoder, unsigned long seed, unsigned int nw, unsigned int Ns);


Tbool LTencode(LTencoder *encoder, char* input_string, unsigned long long lenght);

char *getOutput(LTencoder *encoder);

#endif /*ENCODER_H_*/
