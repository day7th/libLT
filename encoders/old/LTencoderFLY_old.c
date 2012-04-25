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
// File       : LTencoderFLY.c
// Author     : Pasquale Cataldi
// Company    :
// Last update: 2009/05/08
// Platform   : Ansi C
//*****************************************************************************
// Description: This file contains the LT encoder
//*****************************************************************************
// Revisions  :
// Date        Version  Author  Description
//*****************************************************************************

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "LTencoderFLY.h"
#include "./xor/xor.h"
#include "sharedMethods.h"

// Private functions
void createDegreeDistribution(LTencoderFLY *encoder, double *cumulative, unsigned int w, unsigned int s);


LTencoderFLY* mallocEncoderFLY(unsigned long seed) {

    LTencoderFLY *encoder = malloc(sizeof(LTencoderFLY));
    encoder->randomEncoder.seed_ = seed;	// set the seed of the random
    initRandom(&encoder->randomEncoder, seed);

    encoder->xorType	= isSIMD();
    encoder->genSymbols	= 0;
    encoder->currCumulative = NULL;

    // Create the LThashTable for the degree distributions
    encoder->DistributionsTable = create_hashtable(MINHASHTABLESIZE, hashfromkey, equalkeys);
    if (NULL == encoder->DistributionsTable) exit(-1); /*oom*/

    return encoder;
}

void freeEncoderFLY(LTencoderFLY *encoder) {

    destroyLThashTable(encoder->DistributionsTable);
    free(encoder);

}

void initEncoderFLY(LTencoderFLY *encoder, DegDistr code,
                    double lt_delta, double lt_c, double raptor_eps) {

    encoder->code	= code;			// set the code type (LT or Raptor)
    encoder->lt_delta = lt_delta;
    encoder->lt_c = lt_c;
    encoder->raptor_eps = raptor_eps;

    return;
}

void reinitEncoderFLY(LTencoderFLY *encoder, DegDistr code,
                      double lt_delta, double lt_c, double raptor_eps,
                      unsigned long seed) {

    encoder->randomEncoder.seed_ = seed;	// set the seed of the random
    initRandom(&encoder->randomEncoder, seed);
    encoder->genSymbols	= 0;

    encoder->code	= code;			// set the code type (LT or Raptor)
    encoder->lt_delta = lt_delta;
    encoder->lt_c = lt_c;
    encoder->raptor_eps = raptor_eps;

    // Destroy and recreate the LThashTable for the degree distributions
    destroyLThashTable(encoder->DistributionsTable);
    encoder->DistributionsTable = NULL;
    encoder->DistributionsTable = create_hashtable(MINHASHTABLESIZE, hashfromkey, equalkeys);
    if (NULL == encoder->DistributionsTable) exit(-1); /*oom*/

    return;
}

void setEncoderFLY(LTencoderFLY *encoder, unsigned int w, unsigned int s) {

    struct value *v = NULL;
    v = searchEntryInLThashTable(encoder->DistributionsTable, w, s);

    if (NULL == v) {
        if (NULL != (v = insertEntryInLThashTable(encoder->DistributionsTable, w, s))) {
            createDegreeDistribution(encoder, (v->cumulative), w, s);
        }
    }

    // Update the current cumulative and the current w.
    encoder->currCumulative = v->cumulative;
    encoder->currW = w;	// Set w and not v->parameters->w because the first is the newest (in case of
    // collision (i.e. if v refers to more than one w), the first is the right one.

    return;
}

void createDegreeDistribution(LTencoderFLY *encoder, double *cumulative,
                              unsigned int w, unsigned int s) {

    //NOTE: comulative must be prevoiusly allocated
    switch (encoder->code) {
    case Luby:
        RSDcumulative(cumulative, w, s, encoder->lt_delta, encoder->lt_c);
        break;
    case Shokrollahi:
        SHOKcumulative(cumulative, w, s, encoder->raptor_eps);
        break;
    default:
        printf("ERROR: currently only LT and Raptor codes are implemented.\n");
        exit(EXIT_FAILURE);
        break;
    }
    return;
}

void reSeedEncoderFLY(LTencoderFLY *encoder, unsigned long seed) {
    encoder->randomEncoder.seed_ = seed;	// set the seed of the random
    encoder->genSymbols	= 0;
    return;
}

unsigned int LTencodeFLY(LTencoderFLY *encoder, int symbLen,
                         char *info,	unsigned long offset,
                         unsigned int N, char *output) {

    unsigned int IDfirstSymbol = 0;
    unsigned int generated_degree = 0;
    unsigned int *selected_symbols = NULL;
    unsigned int ii, jj, j, kk;
    double sel;

    unsigned int w = encoder->currW;
    double *cumulative = encoder->currCumulative;

    // The output must be already allocated
    if ((NULL == info)||(NULL == output)) {
        printf("ERROR: no encoded information generated.\n");
        exit(EXIT_FAILURE);
    }

    // Allocate the vector for the selected symbols
    selected_symbols = (unsigned int*) malloc(w*sizeof(unsigned int));
    if (selected_symbols == NULL) {										// check alloc
        printf("(libLT_C:LTencoderFLY.c:LTencodeFLY) ERROR - encoding process - Unexpected Memory Allocation Error.\n");
        exit(EXIT_FAILURE);
    }

    IDfirstSymbol = encoder->genSymbols;

    for (ii=0; ii<N; ii++) {

        generated_degree = degree(cumulative, w, real(&encoder->randomEncoder));
        if (generated_degree > w)
            generated_degree = w;

        if (generated_degree == w) {
            choose_all_in_windowCore(w, offset, selected_symbols, generated_degree);

        } else {
            for (jj=0; jj<generated_degree; jj++) {
                sel	= real(&encoder->randomEncoder);
                selected_symbols[jj]	= rand2windowCore(w, offset, sel);

                if (verify_selected_symbol(selected_symbols, jj) == False) {
                    jj--;		// Doesn't repeat encoding symbols
                }
            }
            // This encoder does not sort because does not save the headers
            // sort(selected_symbols, 0, generated_degree-1);	// Sort the chosen encoding symbols' list
        }

        // Copy the Info of the first input symbol indicated in the header
        memcpy(&output[ii*symbLen], &info[selected_symbols[0]*symbLen], symbLen*sizeof(char));

        // EX-OR the other symbols of the header
        if (encoder->xorType != NO) {
            for (j=1; j<generated_degree; j++) {
                // EX-OR COMPUTATION
                calcXOR(&info[selected_symbols[j]*symbLen], &output[ii*symbLen], symbLen, encoder->xorType);
            }
        } else {
            for (j=1; j<generated_degree; j++) {
                // EX-OR COMPUTATION
                for (kk=0; kk<symbLen; kk++) {
                    output[ii*symbLen + kk] ^= info[selected_symbols[j]*symbLen + kk];
                }
            }
        }

        // Increase the number of generated symbols
        encoder->genSymbols++;
    }

    // free and initialize the tmp symbol
    free(selected_symbols);
    selected_symbols = NULL;

    return IDfirstSymbol;
}






























































//void initEncoderFLY(LTencoderFLY *encoder, unsigned long seed, int symblen,
//			unsigned int k, unsigned int w, unsigned int s,
//			double lt_delta, double lt_c, double eps, DegDistr code){
//
//	unsigned int i = 0;
//
//	// free generated symbols, if any
//	if(encoder->matrix.symbols != NULL) {
//		for(i=0; i<encoder->matrix.conf.N;i++) {
//			if (encoder->matrix.symbols[i].header != NULL)
//				free(encoder->matrix.symbols[i].header);
//		}
//		free(encoder->matrix.symbols);
//		encoder->matrix.symbols = NULL;
//	}
//
//	encoder->matrix.seed 	= seed;
//	encoder->symbLen	= symblen;
//	encoder->matrix.codetype = code;
//	encoder->thAvgDegree	= 0.0;		// STATS
//	encoder->genAvgDegree	= 0.0;		// STATS
//
//	encoder->matrix.conf.k = k;
//	encoder->matrix.conf.w = w;
//	encoder->matrix.conf.s = s;
//	encoder->matrix.conf.N = 0;		// no symbs generated
//	encoder->matrix.conf.delta = lt_delta;
//	encoder->matrix.conf.c = lt_c;
//	encoder->matrix.conf.epsShok = eps;
//
//	encoder->randomEncoder.seed_ = seed;	// set the seed of the random
//
//	switch (code) {
//	case Luby:
//		encoder->thAvgDegree = rubust_soliton_distribution(&encoder->matrix);
//		break;
//	case Shokrollahi:
//		encoder->thAvgDegree = shokrollahi_distribution(&encoder->matrix);
//		break;
//	default:
//		printf("ERROR: currently only file and streaming applications can be used.\n");
//		exit(EXIT_FAILURE);
//		break;
//	}
//
//	return;
//}










































/*
void initEncoder(LTencoder2 *encoder, unsigned long seed, int symblen,
				unsigned int k, unsigned int w, unsigned int s,
				double lt_delta, double lt_c, double eps,
				unsigned int N, DegDistr code){

	unsigned int i = 0;

	if(encoder->matrix.symbols != NULL) {
		for(i=0; i<encoder->matrix.conf.N;i++) {
			if (encoder->matrix.symbols[i].header != NULL)
				free(encoder->matrix.symbols[i].header);
		}
		free(encoder->matrix.symbols);
		encoder->matrix.symbols = NULL;
	}

	encoder->matrix.seed 	= seed;
	encoder->symbLen	= symblen;
	encoder->matrix.codetype = code;
	encoder->thAvgDegree	= 0.0;
	encoder->genAvgDegree	= 0.0;

	encoder->matrix.conf.k = k;
	encoder->matrix.conf.w = w;
	encoder->matrix.conf.s = s;
	encoder->matrix.conf.N = N;
	encoder->matrix.conf.delta = lt_delta;
	encoder->matrix.conf.c = lt_c;
	encoder->matrix.conf.epsShok = eps;

	encoder->matrix.symbols = (Symbol *) calloc(N,sizeof(Symbol));

	if(encoder->matrix.cumulative != NULL)
		free(encoder->matrix.cumulative);
	encoder->matrix.cumulative = (double *) calloc(w,sizeof(double));

	switch (code) {
	case Luby:
		encoder->thAvgDegree = rubust_soliton_distribution(&encoder->matrix);
		break;
	case Shokrollahi:
		encoder->thAvgDegree = shokrollahi_distribution(&encoder->matrix);
		break;
	default:
		printf("ERROR: currently only p2p and streaming applications can be used.\n");
		exit(EXIT_FAILURE);
		break;
	}

	encoder->genAvgDegree = create_headers(&encoder->matrix);

	return;
}


void initEncoder_nw(LTencoder *encoder, unsigned long seed, int symblen,
				unsigned int k, unsigned int w, unsigned int s,
				double lt_delta, double lt_c, double eps,
				unsigned int N, DegDistr code, unsigned int nw, unsigned int Ns){

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
	encoder->matrix.conf.epsShok = eps;

//	// Overflow flags
//	flag_size = (double) N*symblen;
//	if (flag_size>ULONG_MAX){
//		printf("ERROR: The source information is too long. The maximum accepted value is 4294967295.\n");
//		exit(EXIT_FAILURE);
//	}

	if(encoder->matrix.symbols != NULL) {

		for(i=0; i<encoder->matrix.conf.N;i++) {
			if (encoder->matrix.symbols[i].header != NULL)
				free(encoder->matrix.symbols[i].header);
		}

		free(encoder->matrix.symbols);
	}

	encoder->matrix.symbols = (Symbol *) calloc(N,sizeof(Symbol));

	if(encoder->matrix.cumulative != NULL)
		free(encoder->matrix.cumulative);
	encoder->matrix.cumulative = (double *) calloc(w,sizeof(double));

	switch (code) {
	case Luby:
		encoder->thAvgDegree = rubust_soliton_distribution(&encoder->matrix);
		break;
	case Shokrollahi:
		encoder->thAvgDegree = shokrollahi_distribution(&encoder->matrix);
		break;
	default:
		printf("ERROR: currently only p2p and streaming applications can be used.\n");
		exit(EXIT_FAILURE);
		break;
	}

	if (nw == 0)
		encoder->genAvgDegree = create_headers(&encoder->matrix);
	else
		encoder->genAvgDegree = create_headers_nw(&encoder->matrix, nw, Ns);

	return;
}



void reSeedEncoder(LTencoder *encoder, unsigned long seed){
	unsigned int i;

	encoder->matrix.seed = seed;

	for(i=0; i<encoder->matrix.conf.N;i++) {
		if (encoder->matrix.symbols[i].header != NULL)
			free(encoder->matrix.symbols[i].header);
	}

	encoder->genAvgDegree = create_headers(&encoder->matrix);

	return;
}

void reSeedEncoder_nw(LTencoder *encoder, unsigned long seed, unsigned int nw, unsigned int Ns){
	unsigned int i;

	encoder->matrix.seed = seed;

	for(i=0; i<encoder->matrix.conf.N;i++) {
		if (encoder->matrix.symbols[i].header != NULL)
			free(encoder->matrix.symbols[i].header);
	}

	if (nw == 0)
		encoder->genAvgDegree = create_headers(&encoder->matrix);
	else
		encoder->genAvgDegree = create_headers_nw(&encoder->matrix, nw, Ns);

	return;
}



Tbool LTencode(LTencoder *encoder, char* input_string, unsigned long long input_lenght){
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
		printf("Error: encode - input_string len error.\n");
		exit(EXIT_FAILURE);
	}

	if(input_lenght % symbLen != 0) {
		paddedInfo = (char*) calloc(NumSrcBits, sizeof(char));
		memcpy(paddedInfo, input_string, input_lenght*sizeof(char));
		padded = True;
	} else
		paddedInfo = input_string;

	if(encoder->output != NULL)
		free(encoder->output);
	encoder->output = (char *) malloc(NumGenBits*sizeof(char));

	if (encoder->output == NULL){
		printf("ERROR: Encoder memory allocation failed! (%.0lf bytes not allocated) \n", (double) NumGenBits*sizeof(char));
		exit(EXIT_FAILURE);
	}

	currinfo = (char*) malloc(symbLen*sizeof(char));

	for (i=0; i<encoder->matrix.conf.N; i++){
		memset(currinfo, 0x00, symbLen);
		// read current degre and header
		currdeg = encoder->matrix.symbols[i].deg;
		currheader = encoder->matrix.symbols[i].header;
		// Copy the Info of the first input symbol indicated in the header
		memcpy(currinfo, &paddedInfo[currheader[0]*symbLen], symbLen*sizeof(char));

		// Exor the other symbols of the header
		if(encoder->xorType != NO) {
			for (j=1; j<currdeg; j++){

				// EX-OR COMPUTATION
				currSymb = currheader[j]*symbLen;
				calcXOR(&paddedInfo[currSymb], currinfo, symbLen, encoder->xorType);
			}
		} else {
			for (j=1; j<currdeg; j++){

				// EX-OR COMPUTATION
				currSymb = currheader[j]*symbLen;
				for (k=0; k<symbLen; k++){
					currinfo[k] ^= paddedInfo[currSymb+k];
				}
			}
		}
		memcpy(&encoder->output[i*symbLen], currinfo, symbLen*sizeof(char));
	}

	if(padded == True)
		free(paddedInfo);

	free(currinfo);
	return padded;
}
*/

