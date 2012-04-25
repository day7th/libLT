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
 * fileFLY.c
 *
 *  Created on: Dec 8, 2010
 *      Author: Pasquale Cataldi
 */

#include "../../libLT_C.h"

#include <stdlib.h>
#include <stdio.h>

#define BYTES	// To uncomment to use the input as sequence of bytes

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

LTencoderFLY* encoder = NULL;
LTdecoderFLY* decoder = NULL;

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "ERROR passing arguments!\n The first argument is the input file, the second is the decoded file.\n");
        return EXIT_FAILURE;
    }
    char* infile = argv[1];
    char* outfile = argv[2];

    printf("---Encoding file %s\n", infile);
    printf("---Recovered Info in file %s\n", outfile);

    // Create the input source
    ulong infilelen = 0;

#ifdef BYTES
    sourceInfo = readFile(infile, &infilelen);
    if (sourceInfo == NULL) {
        fprintf(stderr, "ERROR source file not correctly read.\n");
        return EXIT_FAILURE;
    }
#else
    fprintf(stdout, "-Converting the byte buffer to a bit buffer-\n");
    char *tmpIn = NULL;
    tmpIn = readFile(infile, &infilelen);
    if (NULL == tmpIn) {
        fprintf(stderr, "ERROR source file not correctly read.\n");
        return EXIT_FAILURE;
    }
    sourceInfo = strToHexstr(infilelen, tmpIn);
    infilelen *= 8;
#endif

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

    // Allocate the space to store the encoded symbols' information
    encodedInfo = chk_malloc(N*l, sizeof(char));

    // encode all the source symbols together (Only one window and it generates N encoded symbols)
    LTencodeInfoFLY(encoder, l, sourceInfo, 0, N, encodedInfo);

    printf("Encoding the source information completed.\n");

    /*
     *
     * DECODER FLY
     *
     */

    // Allocate the decoder data structure
    decoder = mallocDecoderFLY(N);
    // Add one bucket to the decoder (you need to add one per each seed or different parameters used. In practice, one per encoder)
    int myBucket = addAndInitBucketFLY_LT(decoder, seed, l, w, s, delta, c);
    // Receive the encoded symbols
    uint numNewReceived = receiveSymbFLY_BucketName(decoder, myBucket, encodedInfo, N*l, 0, 0);

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
    printf("TOT received symbols: %d\n", decoder->NumRecSymbols);
    printf("TOT decoded symbols: %d\n", decoder->NumDecSymbols);

    char *decodedInfo = getDecodedInformation(decoder);

    char *outbuffer = NULL;
    ulong bytesToWrite = 0;
#ifdef BYTES
    outbuffer = decodedInfo;
    bytesToWrite = k*l;
#else
    outbuffer = hexstrToStr(k*l, decodedInfo);
    bytesToWrite = k*l/8;
#endif

    // Write the decoded information on a file
    if (writeFile(outfile, outbuffer, bytesToWrite) == bytesToWrite)
        printf("Output written correctly.\n");
    else
        fprintf(stderr, "Output NOT written correctly.\n");

    // Finally, free the memory
    free(sourceInfo);
    free(encodedInfo);

    freeEncoderFLY(encoder);
    freeDecoderFLY(decoder);

#ifndef BYTES
    free(tmpIn);
    free(outbuffer);
#endif

    return EXIT_SUCCESS;
}
