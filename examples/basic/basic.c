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


/*
 * basic.c
 *
 *  Created on: Dec 8, 2010
 *      Author: Pasquale Cataldi
 */

#include "../../libLT_C.h"

#include <stdlib.h>
#include <stdio.h>

uint l;			/** Information Size of each symbol (bits) */
uint k;			/** Number of input symbols */
uint w;         /** Number of symbols in each window */
uint s;			/** Number of symbols of movements */
double c;		/** LT parameter c */
double delta;	/** LT parameter delta */
ulong seed;		/** Seed of the random generator for the encoding process */
uint N;			/** Maximum number of encoded symbols to be created */

char* sourceInfo = NULL;
char* encodedInfo = NULL;

LTencoder* encoder = NULL;
LTdecoder* decoder = NULL;

int main(int argc, char *argv[]) {

    /* Simple initialization */
    l = 1;
    k = 1000;
    w = k;
    s = w;
    c = 0.4;
    delta = 0.5;
    seed = 663;
    N = 2*k;

    // Create the input source
    sourceInfo = chk_calloc(k*l, sizeof(char));
    unsigned int i = 0;
    for (i = 0; i < k*l; i++)
        sourceInfo[i] = (unsigned char)(i%256);

    /*
     *
     * ENCODER
     *
     */

    // Allocate the encoder data structure
    encoder = mallocEncoder();
    // Init the encoder data structure
    initEncoder_LT(encoder, seed, l, k, w, s, delta, c, N);

    // encode all the source symbols together (Only one window and it generates N encoded symbols)
    LTencode(encoder, sourceInfo, k*l);

    // Reference to the space with the encoded symbol information
    encodedInfo = encoder->output;

    printf("Encoding the source information completed.\n");

    /*
     *
     * DECODER
     *
     */

    // Allocate the decoder data structure
    decoder = mallocDecoder();
    // Add one bucket to the decoder (you need to add one per each seed or different parameters used. In practice, one per encoder)
    int myBucket = addBucket(decoder, seed, l, k, w, s, delta, c, 0.0, N, Luby);

    // Receive the encoded symbols
    uint numNewReceived = receiveSymb_BucketName(decoder, myBucket, encodedInfo, N*l, 0);

    // Decode the symbols
    decodingProcess(decoder);

    // See new decoded symbols
    printf("\n");
    printf("number of source symbols = %d\n", k);
    printf("number of received symbols = %d\n", numNewReceived);

    // Count all decoded symbols
    printf("TOT received symbols: %d\n", decoder->NumRecSymbols);
    printf("TOT decoded symbols: %d\n", decoder->NumDecSymbols);

    for (i = 0; i < k*l; i++)
        printf("_%d", decoder->decoded[i]-sourceInfo[i]);
    printf("\n");

    // Finally, free the memory
    free(sourceInfo);

    freeEncoder(encoder);
    freeDecoder(decoder);

    return EXIT_SUCCESS;
}

