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
 * explicitFLY.c
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

LTencoderFLY* encoder = NULL;
LTdecoderFLY* decoder = NULL;

void freeSymbList(Symbol* list, uint numSymbs);

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "ERROR passing arguments!\n "
                "The first argument is the input file, the second is the decoded file.\n");
        return EXIT_FAILURE;
    }
    char* infile = argv[1];
    char* outfile = argv[2];

    printf("---Encoding file %s\n", infile);
    printf("---Recovered Info in file %s\n", outfile);

    // Create the input source
    ulong infilelen = 0;

    sourceInfo = readFile(infile, &infilelen);
    if (sourceInfo == NULL) {
        fprintf(stderr, "ERROR source file not correctly read.\n");
        return EXIT_FAILURE;
    }

    /* Simple initialization */
    l = 1;
    k = infilelen/l;
    w = k;
    s = w;
    c = 0.4;
    delta = 0.5;
    seed = 663;
    N = 2*k;

    printf("number of input symbols = %d\n", k);

    /*
     *
     * ENCODER FLY
     *
     */

    // Allocate the encoder data structure
    encoder = mallocEncoderFLY(seed);
    // Init the encoder data structure
    setEncoderFLY_LT(encoder, w, s, delta, c);

    // Allocate the space to store the encoded symbols
    Symbol* encodedSymbols = (Symbol*) chk_calloc(N, sizeof(Symbol));

    // encode all the source symbols together (Only one window and it generates N encoded symbols)
    LTencodeSymbolFLY(encoder, l, sourceInfo, 0, N, encodedSymbols);

    printf("Encoding the source information completed.\n");

    /*
     *
     * DECODER FLY
     *
     */

    // Allocate the decoder data structure
    decoder = mallocDecoderFLY(N);
    // Receive the encoded symbols
    uint numNewReceived = receiveEncodedSymbFLY(decoder, encodedSymbols, N);

    printf("Decoding the received information.\n");

    // Decode the symbols
    decodingProcessFLY(decoder);

    /*
     *
     * RESULTS
     *
     */

    // See new decoded symbols
    printf("\n");
    printf("number of received symbols = %d\n", numNewReceived);

    // Count all decoded symbols
    printf("TOT received symbols: %d\n", getNumReceived(decoder));
    printf("TOT decoded symbols: %d\n", getNumDecoded(decoder));

    char *decodedInfo = getDecodedInformation(decoder);

    // Write the decoded information on a file
    if (writeFile(outfile, decodedInfo, k*l) == k*l)
        printf("Output written correctly.\n");
    else
        fprintf(stderr, "Output NOT written correctly.\n");

    /*
     *
     * FREES
     *
     */

    // Finally, free the memory
    free(sourceInfo);

    freeEncoderFLY(encoder);
    freeDecoderFLY(decoder);

    // Free the allocated encodedSymbols
    freeSymbList(encodedSymbols, N);

    return EXIT_SUCCESS;
}


void freeSymbList(Symbol* list, uint numSymbs) {
    if (list == NULL)
        return;
    // Free the allocated encodedSymbols
    uint i;
    for (i = 0; i < numSymbs; i++) {
        Symbol* currSymb = &(list[i]);
        if (currSymb->info != NULL)
            free(currSymb->info);
        if (currSymb->header != NULL)
            free(currSymb->header);
    }
    free(list);
}
