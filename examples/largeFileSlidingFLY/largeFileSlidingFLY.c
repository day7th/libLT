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
 * largeFileSlidingFLY.c
 *
 *  Created on: Jan 3, 2010
 *      Author: Pasquale Cataldi
 *
 *  Based on "Sliding-Window Digital Fountain Codes for Streaming of Multimedia Contents", ISCAS 2007
 *
 */

#include "../../libLT_C.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define RED_EPS 0.5	/** Redundancy ratio [=(N-k)/k] */

uint l;			/** Information Size of each symbol (bits) */
uint k;			/** Number of input symbols */
uint w;         /** Number of symbols in each window */
uint s;			/** Number of symbols of movements */
double c;		/** LT parameter c */
double delta;	/** LT parameter delta */
ulong seed;		/** Seed of the random generator for the encoding process */
uint N;			/** Total number of encoded symbols to be created */

uint Ns;		/** Number of encoding windows of size w */
uint nw;		/** Number of encoded symbols to be created per window */

char* sourceInfoBuffer = NULL;

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

    // Get the infile length
    ulong infilelen = getFileSize(infile);

    printf("The file size is %llu byte.\n", infilelen);

    // Since the file is very large, we read and encode chunks of 1 MB
    uint chunkSize = 1048576;

    /* Simple initialization */
    l = 1;
    k = infilelen/l;
    w = chunkSize/l/128;
    s = w/2;
    c = 0.4;
    delta = 0.5;
    seed = 663;
    N = ceil((1.0+RED_EPS)*k);

    uint numChunks = infilelen/chunkSize;

    Ns = ceil((float)(chunkSize/l - w)/s + 1)+1;
    nw = ceil((1.0+RED_EPS)*s);


    /*
     *
     * ALLOCATIONS
     *
     */

    // Allocate the encoder data structure
    encoder = mallocEncoderFLY(seed);
    // Init the encoder data structure
    setEncoderFLY_LT(encoder, w, s, delta, c);

    // Allocate the decoder data structure
    decoder = mallocDecoderFLY(N);

    /*
     *
     * START THE PROCESS PER CHUNK
     *
     */

    uint cycle;
    for (cycle = 0; cycle < numChunks; cycle++) {

        printf("Cycle: %d\n", cycle);

        // Read the buffer from the info data file
        sourceInfoBuffer = readFileChunk(infile, cycle*chunkSize, chunkSize);
        if (sourceInfoBuffer == NULL) {
            fprintf(stderr, "ERROR source file not correctly read.\n");
            return EXIT_FAILURE;
        }

        uint i;
        Symbol* encodedSymbols;
        for (i = 0; i < Ns; i++) {

            // === ENCODING ===

            // Allocate the space to store the encoded symbols
            encodedSymbols = (Symbol*) chk_calloc(nw, sizeof(Symbol));

            // Consider the source symbols of the current window to generate nw encoded symbols
            LTencodeSymbolFLY_buffer(encoder, l, sourceInfoBuffer, cycle*(chunkSize/l), encodedSymbols, nw, cycle*(chunkSize/l)+s*i);

            printf("\t\tEncoding window %d completed (Number encoded symbols: %u).\n", i, getNumEncodedSymbols(encoder));

            // === DECODING ===

            // Receive the encoded symbols
            receiveEncodedSymbFLY(decoder, encodedSymbols, nw);

            printf("\t\t\tDecoding the received information (up to the %d-th window).\tDecoded: %d\n", i, getNumDecoded(decoder));

            // Decode the symbols
            decodingProcessFLY(decoder);

            // Free the allocated encodedSymbols
            freeSymbList(encodedSymbols, nw);
        }

        // Free the info buffer
        free(sourceInfoBuffer);

    }

    /*
     *
     * RESULTS
     *
     */

    // See new decoded symbols
    printf("\n");

    printf("number of chunks = %d\n", numChunks);
    printf("number of input symbols = %d\n", k);
    printf("number of encoding windows = %d\n", Ns);
    printf("number of encoded symbols per window = %d\n", nw);
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
    freeEncoderFLY(encoder);
    freeDecoderFLY(decoder);

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
