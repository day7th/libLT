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
// File       : LTdecoderFLY.c
// Author     : Pasquale Cataldi
// Company    :
// Last update: 2010/12/30
// Platform   : Ansi C
//*****************************************************************************
// Description: This file contains the LT decoder FLY structure and function
//              declarations
//*****************************************************************************
// Revisions  :
// Date        Version  Author  Description
// 2010/12/30  0.2      PC      First traced release.
//*****************************************************************************

#ifndef LTDECODERFLY_H_
#define LTDECODERFLY_H_

#include "../utils/LThash.h"
#include "../utils/LTstruct.h"
#include "../utils/utilTlv.h"
#include "../utils/ciofile.h"
#include "../list/node.h"
#include "../xor/xor.h"

#define noDEBUGprintReleasedSymbol
#define noDEBUGdecoderPrintHeaders
#define noDEBUGdecoderPrintBucketRandomSeed
#define noDEBUGdecoderFree

typedef struct {
    Node *RXsymbList;
    Node *lastRXsymbol;
    unsigned int NumRecSymbols;	// Number of received symbols (all buckets)
    unsigned int NumDecSymbols;	// Number of decoded symbols
    Node *bucketFLYList;		// Head of the bucket list
    XorType xorType;
    // output
    char *decoded;		// It is a buffer with the decoded information (size k symbols * symbol length)
    unsigned int k;             // size (in symbols) of the output circular buffer
    char *decodingStatus;		// Char array that indicates which symbols have been decoded (size k symbols * symbol length)
} LTdecoderFLY;

typedef struct {
    int nameBucket;
    Symbol *symbols;			// symbol list
    unsigned int genSymbs;		// Number of headers already generated
    // configuration
    unsigned int w;
    unsigned int s;
    double lt_delta;
    double lt_c;
    double epsShok;
    DegDistr code;
    int symbLen;
    unsigned long seed;
    double *cumulative;
    randomPA buckRan;
} BucketFLY;



// --- LTdecoderFLY - functions

LTdecoderFLY *mallocDecoderFLY(unsigned int decodedSymbolsBufferSize);
void freeDecoderFLY(LTdecoderFLY *decoder);

// Reset the number of received/decoded symbols and the decoded buffer
void resetDecoderFLY(LTdecoderFLY *decoder);

void decodingProcessFLY(LTdecoderFLY *decoder);

char *getDecodedInformation(LTdecoderFLY *decoder);

char *getDecodingStatusInformation(LTdecoderFLY *decoder);

int getNumDecoded(LTdecoderFLY *decoder);

int getNumReceived(LTdecoderFLY *decoder);

// --- BucketFLY - functions

int addBucketFLY(LTdecoderFLY* decoder);

int addAndInitBucketFLY(LTdecoderFLY *decoder, unsigned long seed,
                        int symblen, unsigned int w, unsigned int s,
                        double lt_delta, double lt_c, double epsShok, DegDistr code);

void initBucketFLY(BucketFLY *bucket, unsigned long seed, int symblen,
                   unsigned int w, unsigned int s,
                   double lt_delta, double lt_c, double epsShok, DegDistr code);

void rmBucketFLY(LTdecoderFLY **decoder, Node *node);

BucketFLY *getBucketFLY(LTdecoderFLY *decoder, int nameBucket);
Node *getNodeBucketFLY(LTdecoderFLY *decoder, int nameBucket);
int getNameBucketFLY(LTdecoderFLY *decoder, unsigned long seed, unsigned int w, unsigned int s,
                     double lt_delta, double lt_c, double epsShok);


void createHeadersFly(BucketFLY *bucket,
                      unsigned int newHeadersToGenerate,
                      unsigned int offset);	// first symbol ID of the window

unsigned int receiveSymbFLY(LTdecoderFLY *decoder, BucketFLY *bucket,
                            char *encodedInfo, unsigned int encodedInfoLength,
                            unsigned int IDfirstSymb, unsigned long offset);

unsigned int receiveEncodedSymbFLY(LTdecoderFLY *decoder, Symbol* encodedSymbols,
                                   unsigned int numberEncodedSymbols);

unsigned int countUndecSymbInWindow(char *decoded, unsigned int startIndex,
                                    unsigned int winLengthByte, int symbLen);

int addAndInitBucketFLY_LT(LTdecoderFLY *decoder, unsigned long seed,
                           int symblen, unsigned int w, unsigned int s,
                           double lt_delta, double lt_c);

int addAndInitBucketFLY_Raptor(LTdecoderFLY *decoder, unsigned long seed,
                               int symblen, unsigned int w, unsigned int s,
                               double epsShok);

unsigned int receiveSymbFLY_BucketName(LTdecoderFLY *decoder, int bucketName,
                                       char *encodedInfo, unsigned int encodedInfoLength,
                                       unsigned int IDfirstSymb, unsigned long offset);

char *LTdecodeFLY_fromFile(LTdecoderFLY *decoder, char *infile);

char *LTdecodeFLY_fromFiles(LTdecoderFLY *decoder, int, char **infiles);

#endif /*LTDECODERFLY_H_*/
