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
// File       : decoder.h
// Author     : Pasquale Cataldi, Andrea Tomatis
// Company    :
// Last update: 2008/07/21
// Platform   : Ansi C
//*****************************************************************************
// Description: This file contains the LT decoder declarations
//*****************************************************************************
// Revisions  :
// Date        Version  Author  Description
// 2008/07/17  0.1      PC,AT   First release
// 2008/07/17  0.2      PC,AT   Added getBucket and addBucket returns
//								the name of the bucket created.
// 2008/07/24  0.3		PC,AT	Removed elapsed time from decoding_process_p2p
// 2008/11/20  0.4      PC,AT   The functions "initBucket" and "reSeedBucket"
//                              now return void instead of the not exloited int.
//                              Added the functions "initBucket_nw", "reSeedBucket_nw"
//                              and "addBucket_nw"
// 2009/05/08  0.4      PC	Moved the structure definition in this file in
//				order to add modularity
//*****************************************************************************


#ifndef DECODER_H_
#define DECODER_H_

#include "../utils/LTstruct.h"
#include "../list/node.h"
#include "../xor/xor.h"

typedef struct {
    char *decoded;
    Node *RXsymbList;
    Node *lastRXsymbol;
    unsigned int NumRecSymbols;		// Num of received symbols (all buckets)
    unsigned int NumDecSymbols;		// Num of decoded symbols
    Node *bucketList;				// Head of the bucket list
    XorType xorType;
    char *decodingStatus;		// Char array that indicates which symbols have been decoded (size k symbols * symbol length)
} LTdecoder;

typedef struct {
    int nameBucket;
    Mgenerator matrix;
    int symbLen;
    char *input;
} Bucket;

LTdecoder* mallocDecoder(void);
void freeDecoder(LTdecoder *decoder);
void reinitDecoder(LTdecoder *decoder);

// addBucket returns the name of the bucket
int addBucket(LTdecoder* decoder, unsigned long seed, int symblen,
              unsigned int k, unsigned int w, unsigned int s,
              double lt_delta, double lt_c, double eps, unsigned int N, DegDistr code);
void initBucket(Bucket *bucket, unsigned long seed, int symblen,
                unsigned int k, unsigned int w, unsigned int s,
                double lt_delta, double lt_c, double eps, unsigned int N, DegDistr code);
void reSeedBucket(Bucket *bucket, unsigned long seed);

int addBucket_nw(LTdecoder* decoder, unsigned long seed, int symblen,
                 unsigned int k, unsigned int w, unsigned int s,
                 double lt_delta, double lt_c, double eps, unsigned int N, DegDistr code,
                 unsigned int nw, unsigned int Ns);
void initBucket_nw(Bucket *bucket, unsigned long seed, int symblen,
                   unsigned int k, unsigned int w, unsigned int s,
                   double lt_delta, double lt_c, double eps, unsigned int N, DegDistr code,
                   unsigned int nw, unsigned int Ns);
void reSeedBucket_nw(Bucket *bucket, unsigned long seed, unsigned int nw, unsigned int Ns);

void rmBucket(LTdecoder **decoder, Node *node);

Bucket *getBucket(LTdecoder* decoder, int nameBucket);
Node *getNodeBucket(LTdecoder* decoder, int nameBucket);

unsigned int genRXSymb(Bucket *bucket, char *encodedInfo, unsigned int length, Node **RXsymbols, Node **lastRXsymbol, unsigned int IDfirstSymb);

unsigned int receiveSymb_BucketName(LTdecoder *decoder, int bucketName,
                                    char *encodedInfo, unsigned int encodedInfoLength,
                                    unsigned int IDfirstSymb);

Tbool decodingProcess(LTdecoder* decoder);

unsigned int getNumRecSymbols(LTdecoder *decoder);
unsigned int getNumDecSymbols(LTdecoder *decoder);
char *getDecoded(LTdecoder *decoder);



#endif /*DECODER_H_*/
