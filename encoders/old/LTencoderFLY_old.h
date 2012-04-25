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
// File       : LTencoderFLY.h
// Author     : Pasquale Cataldi
// Company    :
// Last update: 2009/05/08
// Platform   : Ansi C
//*****************************************************************************
// Description: This file contains the LT encoder declarations
//			This encoder differs from LTencoder because in this one
//			you can generate encoded symbols on-the-fly.
//*****************************************************************************
// Revisions  :
// Date        Version  Author  Description
//*****************************************************************************

#ifndef ENCODERFLY_H_
#define ENCODERFLY_H_

#define MINHASHTABLESIZE 16

#include "LTstruct.h"
#include "./random/randomPA.h"
#include "LThash.h"

typedef struct {
    XorType xorType;

    DegDistr code;
    double lt_delta;
    double lt_c;
    double raptor_eps;

    randomPA randomEncoder;	// This is the random of the encoder
    unsigned int genSymbols;// It is a counter of the generated symbols

    struct hashtable *DistributionsTable;
    double *currCumulative;
    unsigned int currW;
} LTencoderFLY;


LTencoderFLY* mallocEncoderFLY(unsigned long seed);

void initEncoderFLY(LTencoderFLY *encoder, DegDistr code,
                    double lt_delta, double lt_c, double raptor_eps);

void reinitEncoderFLY(LTencoderFLY *encoder, DegDistr code,
                      double lt_delta, double lt_c, double raptor_eps,
                      unsigned long seed);

void setEncoderFLY(LTencoderFLY *encoder, unsigned int w, unsigned int s);

void reSeedEncoderFLY(LTencoderFLY *encoder, unsigned long seed);

void freeEncoderFLY(LTencoderFLY *encoder);

// Return the ID (index) of the first generated symbol
unsigned int LTencodeFLY(LTencoderFLY *encoder, int symbLen,
                         char *info, unsigned long offset,
                         unsigned int N, char *output);








//void initEncoder(LTencoder2 *encoder, unsigned long seed, int symblen, unsigned int k, unsigned int w, unsigned int s, double lt_delta, double lt_c, double eps, unsigned int N, DegDistr code);
//void reSeedEncoder2(LTencoder *encoder, unsigned long seed);

//void initEncoder_nw(LTencoder *encoder, unsigned long seed, int symblen, unsigned int k, unsigned int w, unsigned int s, double lt_delta, double lt_c, double eps, unsigned int N, DegDistr code, unsigned int nw, unsigned int Ns);
//void reSeedEncoder_nw(LTencoder *encoder, unsigned long seed, unsigned int nw, unsigned int Ns);


//Tbool LTencode(LTencoder *encoder, char* input_string, unsigned long long lenght);

#endif /*ENCODERFLY_H_*/
