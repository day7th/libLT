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
// File       : LTencoder.c
// Author     : Pasquale Cataldi, Andrea Tomatis
// Company    :
// Last update: 2010/12/30
// Platform   : Ansi C
//*****************************************************************************
// Description: This file contains the LT encoder
//*****************************************************************************
// Revisions  :
// Date        Version  Author  Description
// 2008/07/17  0.1      PC,AT   First release
// 2008/07/28  0.2      PC      Changed the name of the encode function
//								to LTencode (for compatibility).
// 2008/10/25  0.3      PC      Get the average degree (theoretical and real)
//								in init() from sharedMethods.
// 2008/11/20  0.4      PC      The functions initEncoder and reSeedEncoder will
//                              call the function "create_headers" that was
//                              formerly named "create_headers_p2p".
//                              The functions "initEncoder" and "reSeedEncoder"
//                              now return void instead of the not exploited int
// 2010/12/30  0.5      PC      Added memory checks and the "decodingStatus" variable.
// 2010/12/30  0.6      PC      Used new print_error functions from utilErr.h
//*****************************************************************************

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "LTencoder.h"
#include "../utils/sharedMethods.h"
#include "../utils/utilMem.h"
#include "../utils/utilErr.h"

LTencoder* mallocEncoder(void) {
    LTencoder *enc = chk_malloc(1, sizeof(LTencoder));

    enc->output 			= NULL;
    enc->matrix.symbols 	= NULL;
    enc->matrix.cumulative	= NULL;

    enc->xorType		= isSIMD();

    return enc;
}

void freeEncoder(LTencoder *encoder) {
    int i;

    if (encoder != NULL) {

        if (encoder->output != NULL)
            free(encoder->output);

        for (i=0; i<encoder->matrix.conf.N; i++) {
            if (encoder->matrix.symbols[i].header != NULL)
                free(encoder->matrix.symbols[i].header);
        }

        if (encoder->matrix.cumulative != NULL)
            free(encoder->matrix.cumulative);

        if (encoder->matrix.symbols != NULL)
            free(encoder->matrix.symbols);

        free(encoder);
    }
}

void initEncoder(LTencoder *encoder, unsigned long seed, int symblen,
                 unsigned int k, unsigned int w, unsigned int s,
                 double lt_delta, double lt_c, double raptor_eps,
                 unsigned int N, DegDistr code) {

    unsigned int i = 0;
//	double flag_size = 0.0;

    if (encoder->matrix.symbols != NULL) {
        for (i=0; i<encoder->matrix.conf.N; i++) {
            if (encoder->matrix.symbols[i].header != NULL)
                free(encoder->matrix.symbols[i].header);
        }
        free(encoder->matrix.symbols);
        encoder->matrix.symbols = NULL;
    }

    encoder->matrix.seed 	= seed;
    encoder->symbLen		= symblen;
    encoder->matrix.codetype = code;
    encoder->thAvgDegree	= 0.0;
    encoder->genAvgDegree	= 0.0;

    encoder->matrix.conf.k = k;
    encoder->matrix.conf.w = w;
    encoder->matrix.conf.s = s;
    encoder->matrix.conf.N = N;
    encoder->matrix.conf.delta = lt_delta;
    encoder->matrix.conf.c = lt_c;
    encoder->matrix.conf.epsShok = raptor_eps;

//	// Overflow flags
//	flag_size = (double) N*symblen;
//	if (flag_size>ULONG_MAX){
//		print_error("ERROR: The source information is too long. The maximum accepted value is 4294967295.\n");
//		exit(EXIT_FAILURE);
//	}

    encoder->matrix.symbols = (Symbol *) chk_calloc(N, sizeof(Symbol));

    if (encoder->matrix.cumulative != NULL)
        free(encoder->matrix.cumulative);
    encoder->matrix.cumulative = (double *) chk_calloc(w, sizeof(double));

    switch (code) {
    case Luby:
        encoder->thAvgDegree = rubust_soliton_distribution(&encoder->matrix);
        break;
    case Shokrollahi:
        encoder->thAvgDegree = shokrollahi_distribution(&encoder->matrix);
        break;
    default:
        print_error("ERROR: currently only LT and Raptor codes are implemented.\n");
        exit(EXIT_FAILURE);
        break;
    }

    encoder->genAvgDegree = create_headers(&encoder->matrix);

    return;
}

void initEncoder_LT(LTencoder *encoder, unsigned long seed, int symblen, unsigned int k, unsigned int w, unsigned int s, double lt_delta, double lt_c, unsigned int N) {
    initEncoder(encoder, seed, symblen, k, w, s, lt_delta, lt_c, 0.0, N, Luby);
}

void initEncoder_Raptor(LTencoder *encoder, unsigned long seed, int symblen, unsigned int k, unsigned int w, unsigned int s, double raptor_eps, unsigned int N) {
    initEncoder(encoder, seed, symblen, k, w, s, 0.0, 0.0, raptor_eps, N, Shokrollahi);
}

void initEncoder_nw(LTencoder *encoder, unsigned long seed, int symblen,
                    unsigned int k, unsigned int w, unsigned int s,
                    double lt_delta, double lt_c, double raptor_eps,
                    unsigned int N, DegDistr code, unsigned int nw, unsigned int Ns) {

    unsigned int i = 0;
//	double flag_size = 0.0;

    encoder->matrix.seed 	= seed;
    encoder->symbLen		= symblen;
    encoder->matrix.codetype = code;
    encoder->thAvgDegree	= 0.0;
    encoder->genAvgDegree	= 0.0;

    encoder->matrix.conf.k = k;
    encoder->matrix.conf.w = w;
    encoder->matrix.conf.s = s;
    encoder->matrix.conf.N = N;
    encoder->matrix.conf.delta = lt_delta;
    encoder->matrix.conf.c = lt_c;
    encoder->matrix.conf.epsShok = raptor_eps;

//	// Overflow flags
//	flag_size = (double) N*symblen;
//	if (flag_size>ULONG_MAX){
//		print_error("ERROR: The source information is too long. The maximum accepted value is 4294967295.\n");
//		exit(EXIT_FAILURE);
//	}

    if (encoder->matrix.symbols != NULL) {

        for (i=0; i<encoder->matrix.conf.N; i++) {
            if (encoder->matrix.symbols[i].header != NULL)
                free(encoder->matrix.symbols[i].header);
        }

        free(encoder->matrix.symbols);
    }

    encoder->matrix.symbols = (Symbol *) chk_calloc(N, sizeof(Symbol));

    if (encoder->matrix.cumulative != NULL)
        free(encoder->matrix.cumulative);
    encoder->matrix.cumulative = (double *) chk_calloc(w, sizeof(double));

    switch (code) {
    case Luby:
        encoder->thAvgDegree = rubust_soliton_distribution(&encoder->matrix);
        break;
    case Shokrollahi:
        encoder->thAvgDegree = shokrollahi_distribution(&encoder->matrix);
        break;
    default:
        print_error("ERROR: currently only p2p and streaming applications can be used.\n");
        exit(EXIT_FAILURE);
        break;
    }

    if (nw == 0)
        encoder->genAvgDegree = create_headers(&encoder->matrix);
    else
        encoder->genAvgDegree = create_headers_nw(&encoder->matrix, nw, Ns);

    return;
}

void initEncoder_LT_nw(LTencoder *encoder, unsigned long seed, int symblen, unsigned int k, unsigned int w, unsigned int s, double lt_delta, double lt_c, unsigned int N, unsigned int nw, unsigned int Ns) {
    initEncoder_nw(encoder, seed, symblen, k, w, s, lt_delta, lt_c, 0.0, N, Luby, nw, Ns);
}

void initEncoder_Raptor_nw(LTencoder *encoder, unsigned long seed, int symblen, unsigned int k, unsigned int w, unsigned int s, double raptor_eps, unsigned int N, unsigned int nw, unsigned int Ns) {
    initEncoder_nw(encoder, seed, symblen, k, w, s, 0.0, 0.0, raptor_eps, N, Shokrollahi, nw, Ns);
}



void reSeedEncoder(LTencoder *encoder, unsigned long seed) {
    unsigned int i;

    encoder->matrix.seed = seed;

    for (i=0; i<encoder->matrix.conf.N; i++) {
        if (encoder->matrix.symbols[i].header != NULL)
            free(encoder->matrix.symbols[i].header);
    }

    encoder->genAvgDegree = create_headers(&encoder->matrix);

    return;
}

void reSeedEncoder_nw(LTencoder *encoder, unsigned long seed, unsigned int nw, unsigned int Ns) {
    unsigned int i;

    encoder->matrix.seed = seed;

    for (i=0; i<encoder->matrix.conf.N; i++) {
        if (encoder->matrix.symbols[i].header != NULL)
            free(encoder->matrix.symbols[i].header);
    }

    if (nw == 0)
        encoder->genAvgDegree = create_headers(&encoder->matrix);
    else
        encoder->genAvgDegree = create_headers_nw(&encoder->matrix, nw, Ns);

    return;
}



Tbool LTencode(LTencoder *encoder, char* input_string, unsigned long long input_lenght) {
    unsigned int currdeg		= 0;
    unsigned int *currheader	= 0;
    unsigned int currSymb		= 0;
    unsigned long long NumSrcBits = 0;
    unsigned long long NumGenBits = 0;
    int symbLen = 0;
    char *currinfo = NULL;
    char *paddedInfo = NULL;
    unsigned int j,i,k;			// for cycles
    Tbool padded = False;

    NumSrcBits	= encoder->matrix.conf.k*encoder->symbLen;
    NumGenBits	= encoder->matrix.conf.N*encoder->symbLen;
    symbLen		= encoder->symbLen;

    if (input_lenght > NumSrcBits) {
        print_error("ERROR: encode - input_string len error.\n");
        exit(EXIT_FAILURE);
    }

    if (input_lenght % symbLen != 0) {
        paddedInfo = (char*) chk_calloc(NumSrcBits, sizeof(char));
        memcpy(paddedInfo, input_string, input_lenght*sizeof(char));
        padded = True;
    } else
        paddedInfo = input_string;

    if (encoder->output != NULL)
        free(encoder->output);
    encoder->output = (char *) chk_malloc(NumGenBits, sizeof(char));

    currinfo = (char*) chk_malloc(symbLen, sizeof(char));

    for (i=0; i<encoder->matrix.conf.N; i++) {
        memset(currinfo, 0x00, symbLen);
        // read current degre and header
        currdeg = encoder->matrix.symbols[i].deg;
        currheader = encoder->matrix.symbols[i].header;
        // Copy the Info of the first input symbol indicated in the header
        memcpy(currinfo, &paddedInfo[currheader[0]*symbLen], symbLen*sizeof(char));

        // Exor the other symbols of the header
        if (encoder->xorType != NO) {
            for (j=1; j<currdeg; j++) {

                // EX-OR COMPUTATION
                currSymb = currheader[j]*symbLen;
                calcXOR(&paddedInfo[currSymb], currinfo, symbLen, encoder->xorType);
            }
        } else {
            for (j=1; j<currdeg; j++) {

                // EX-OR COMPUTATION
                currSymb = currheader[j]*symbLen;
                for (k=0; k<symbLen; k++) {
                    currinfo[k] ^= paddedInfo[currSymb+k];
                }
            }
        }
        memcpy(&encoder->output[i*symbLen], currinfo, symbLen*sizeof(char));
    }

    if (padded == True)
        free(paddedInfo);

    free(currinfo);
    return padded;
}

char *getOutput(LTencoder *encoder) {
    return encoder->output;
}
