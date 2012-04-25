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
// Description: This file contains the LT decoder FLY implementation
//*****************************************************************************
// Revisions  :
// Date        Version  Author  Description
// 2010/12/30  0.2      PC      First traced release.
//*****************************************************************************

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "LTdecoderFLY.h"
#include "../utils/sharedMethods.h"
#include "../utils/utilMem.h"
#include "../utils/utilErr.h"

// =======================================================================
//					LTdecoderFLY
// =======================================================================


#ifdef DEBUGprintReleasedSymbol
void printReleasedSymbol(unsigned int check, unsigned int k);
#endif

LTdecoderFLY* mallocDecoderFLY(unsigned int decodedSymbolsBufferSize) {
    LTdecoderFLY *dec = chk_malloc(1, sizeof(LTdecoderFLY));

    dec->RXsymbList = NULL;
    dec->lastRXsymbol = NULL;
    dec->bucketFLYList = NULL;
    dec->NumRecSymbols = 0;
    dec->NumDecSymbols = 0;
    dec->decoded = NULL;
    dec->k = decodedSymbolsBufferSize;
    dec->decodingStatus = NULL;

    // SIMD
    dec->xorType = isSIMD();

    return dec;
}


void freeDecoderFLY(LTdecoderFLY *decoder) {
    Node *temp= NULL;

    if (decoder != NULL) {

        // Free the buffer of the decoded info
        if (decoder->decoded != NULL) {
            free(decoder->decoded);
            decoder->decoded = NULL;
        }

        // Free the buffer of the decoding status info
        if (decoder->decodingStatus != NULL) {
            free(decoder->decodingStatus);
            decoder->decodingStatus = NULL;
        }

        // Remove all the buckets
        if (decoder->bucketFLYList != NULL) {
            while (decoder->bucketFLYList != NULL)
                rmBucketFLY(&decoder, decoder->bucketFLYList);
        }

        // Remove and free all the received symbols
#ifdef DEBUGdecoderFree
        int i = 0;
#endif

        if (decoder->RXsymbList != NULL) {

            while (decoder->RXsymbList != NULL) {
                temp = decoder->RXsymbList->next;

                if (decoder->RXsymbList->data != NULL) {
                    // Free Symbol structure
                    free(((Symbol *)decoder->RXsymbList->data)->header);
                    ((Symbol *)decoder->RXsymbList->data)->header = NULL;
                    free(((Symbol *)decoder->RXsymbList->data)->info);
                    ((Symbol *)decoder->RXsymbList->data)->info = NULL;
                    free(decoder->RXsymbList->data);
                    decoder->RXsymbList->data = NULL;
                }

#ifdef DEBUGdecoderFree
                printf("removed symb[%d] from free decoder\n", i);
                i++;
#endif

                free(decoder->RXsymbList);
                decoder->RXsymbList = temp;
            }
        }
        free(decoder);
        decoder = NULL;
    }
}

void resetDecoderFLY(LTdecoderFLY *decoder) {
    Node *temp= NULL;

    if (NULL != decoder->decoded)
        free(decoder->decoded);
    decoder->decoded = NULL;

    if (NULL != decoder->decodingStatus)
        free(decoder->decodingStatus);
    decoder->decodingStatus = NULL;

    if (decoder->RXsymbList != NULL) {

        while (decoder->RXsymbList != NULL) {
            temp = decoder->RXsymbList->next;

            if (decoder->RXsymbList->data != NULL) {
                // Free Symbol structure
                free(((Symbol *)decoder->RXsymbList->data)->header);
                ((Symbol *)decoder->RXsymbList->data)->header = NULL;
                free(((Symbol *)decoder->RXsymbList->data)->info);
                ((Symbol *)decoder->RXsymbList->data)->info = NULL;
                free(decoder->RXsymbList->data);
                decoder->RXsymbList->data = NULL;
            }
            free(decoder->RXsymbList);
            decoder->RXsymbList = temp;
        }
    }
    decoder->lastRXsymbol = NULL;
    decoder->NumRecSymbols = 0;
    decoder->NumDecSymbols = 0;
    return;
}


// Processes the received symbols
Tbool processReceivedFLY(LTdecoderFLY* decoder) {
    Tbool one_decoded = False;
    unsigned int currDeg = 0;
    unsigned int realDeg = 0;
    unsigned int check = 0;
    Node *nextNode= NULL;
    int symbLen = 0;
    unsigned int k = decoder->k;

    // Start from the first received symbol (if exists)
    Node *current = NULL;
    current = decoder->RXsymbList;

    // All the symbols in the decode MUST have the same length!!
    if (current->data != NULL)
        symbLen = ((Symbol *)decoder->RXsymbList->data)->symbLen;
    else if (decoder->bucketFLYList != NULL)
        symbLen = ((BucketFLY *)decoder->bucketFLYList->data)->symbLen;
    if (symbLen==0) {
        print_error("ERROR: processReceivedFLY. The symbol length cannot be defined.\n");
        exit(EXIT_FAILURE);
    }

    // If the decoded buffer has not been allocated yet, allocate it
    if (decoder->decoded == NULL) {
        decoder->decoded = (char *) chk_malloc(k*symbLen, sizeof(char));
    }
    // If the decodingStatus buffer has not been allocated yet, allocate it
    if (decoder->decodingStatus == NULL) {
        decoder->decodingStatus = (char *) chk_calloc(k*symbLen, sizeof(char));
    }

    while (current != NULL) {

        nextNode = current->next;

        if (current->data == NULL) {
            print_error("ERROR: process_received - current data empty.\n");
            exit(EXIT_FAILURE);
        }

        Symbol *currSymb = (Symbol *) current->data;

        // Get the degree of the current encoded symbol
        currDeg = currSymb->deg;
        if ((currDeg<1)&&(currDeg>k)) {	// It should never happen!!
            print_error("ERROR: process_received - out of bounds degree found.\n");
            rmSymbol(&(decoder->RXsymbList), current);
        }

#ifdef DEBUGPrintReceivedSymbolHeaders
        unsigned int debIndex;
        printf("Decoding Header: Deg = %d \t Check Symbs =", currDeg);
        for (debIndex = 0; debIndex < currDeg; debIndex++)
            printf(" %d", currSymb->header[debIndex]);
        printf("\n");
#endif


        if (currDeg == 1) { // If this symbol has degree 1 then directly decode it
            check = currSymb->header[0];

            // Check that the encoded information exists
            if (currSymb->info == NULL) {
                print_error("ERROR: symbol info not allocated!\n");
                exit(EXIT_FAILURE);
            }
            // Check that the header exists
            if (currSymb->header == NULL) {
                print_error("ERROR: symbol header not allocated!\n");
                exit(EXIT_FAILURE);
            }
            // Check that the symbol in the header is within the k input symbols
            if (check >= k) {
                print_error("ERROR: decoded symbol header out of buffer.\n");
                exit(EXIT_FAILURE);
            }

            // Check if the symbol in the header has not being decoded yet.
            if (isSymbolDecoded(decoder->decodingStatus, check*symbLen, symbLen) == False) {

                // Copy the information in the decoded information array
                memcpy(&decoder->decoded[check*symbLen],
                       currSymb->info,
                       symbLen*sizeof(char));
                // Set the bytes in the decoding status array to 0x01 to indicate that the symbols is decoded.
                memset(&decoder->decodingStatus[check*symbLen],
                       0x01,
                       symbLen*sizeof(char));
                one_decoded = True;
                decoder->NumDecSymbols++;
            }

#ifdef DEBUGprintReleasedSymbol
            printReleasedSymbol(check, k);
#endif

            rmSymbol(&(decoder->RXsymbList), current);

        } else {
            // Compute the real degree of the symbol
            realDeg = computeRealDegree(currSymb,
                                        decoder->decodingStatus,
                                        symbLen, currDeg, k);
            switch (realDeg) {
            case 0:
                rmSymbol(&(decoder->RXsymbList), current);
                break;
            case 1:
                check = reduceDegree(&currSymb,
                                     decoder->decoded, decoder->decodingStatus, currDeg, symbLen, realDeg,
                                     decoder->xorType);

                if (currSymb->info == NULL) {
                    print_error("ERROR: symbol info not allocated!\n");
                    exit(EXIT_FAILURE);
                }

                if (check>=k) {
                    print_error("ERROR: decoded symbol out of buffer.\n");
                    exit(EXIT_FAILURE);
                }

                if (decoder->decodingStatus[check*symbLen] == 0x00) {
                    memcpy(&decoder->decoded[check*symbLen],
                           currSymb->info,
                           symbLen*sizeof(char));
                    memset(&decoder->decodingStatus[check*symbLen],
                           0x01,
                           symbLen*sizeof(char));
                    one_decoded = True;
                    decoder->NumDecSymbols++;
                }
                rmSymbol(&(decoder->RXsymbList), current);

#ifdef DEBUGprintReleasedSymbol
                printReleasedSymbol(check, k);
#endif

                break;
            default:
                break;
            }
        }
        current = nextNode;
    }
    // Update lastRXsymbol (necessary for adding new received symbols)
    current = decoder->RXsymbList;
    while (current != NULL) {
        if (current->next == NULL) {
            decoder->lastRXsymbol = current;
            break;
        }
        current = current->next;
    }
    return one_decoded;
}

void decodingProcessFLY(LTdecoderFLY* decoder) {

    Tbool one_decoded = True;

    if (decoder == NULL) {
        print_error("ERROR: decodingProcessFLY - No decoder!\n");
        exit(EXIT_FAILURE);
    }

    while ((decoder->RXsymbList != NULL)&&(one_decoded == True)) {
        one_decoded = processReceivedFLY(decoder);
    }

    return;
}


// =======================================================================
//					BucketFLY
// =======================================================================

int addBucketFLY(LTdecoderFLY* decoder) {

    int nameNewBucket = 0;
    Node *current = decoder->bucketFLYList;	// Set the current pointer to the first bucket (if any)
    Node *ptr= NULL;

    if (decoder->bucketFLYList == NULL) {	// If there are no buckets
        ptr = addNode(NULL);
        decoder->bucketFLYList = ptr;
    } else {
        // find the last bucket
        while (current->next != NULL) {
            current = current->next;
        }
        // The new bucket name will have the name of the previous one +1
        nameNewBucket = ((BucketFLY *)current->data)->nameBucket +1;
        // Create a new node
        ptr = addNode(current);
        // Add the new node to the bucketFLYList
        current->next = ptr;
    }

    // Allocate a new bucket and initialize its parameters
    ptr->data = (void *) chk_calloc(1, sizeof(BucketFLY));
    ((BucketFLY *)ptr->data)->nameBucket = nameNewBucket;
    ((BucketFLY *)ptr->data)->symbols = NULL;
    ((BucketFLY *)ptr->data)->cumulative = NULL;
    ((BucketFLY *)ptr->data)->genSymbs = 0;

    return nameNewBucket;
}


int addAndInitBucketFLY(LTdecoderFLY* decoder, unsigned long seed, int symblen,
                        unsigned int w, unsigned int s,
                        double lt_delta, double lt_c, double epsShok, DegDistr code) {

    int nameNewBucket = 0;
    Node *current = decoder->bucketFLYList;	// Set the current pointer to the first bucket (if any)
    Node *ptr= NULL;

    if (decoder->bucketFLYList == NULL) {	// If there are no buckets
        ptr = addNode(NULL);
        decoder->bucketFLYList = ptr;
    } else {
        // find the last bucket
        while (current->next != NULL) {
            current = current->next;
        }
        // The new bucket name will have the name of the previous one +1
        nameNewBucket = ((BucketFLY *)current->data)->nameBucket +1;
        // Create a new node
        ptr = addNode(current);
        // Add the new node to the bucketFLYList
        current->next = ptr;
    }

    // Allocate a new bucket and initialize its parameters
    ptr->data = (void *) chk_calloc(1, sizeof(BucketFLY));
    ((BucketFLY *)ptr->data)->nameBucket = nameNewBucket;
    ((BucketFLY *)ptr->data)->symbols = NULL;
    ((BucketFLY *)ptr->data)->cumulative = NULL;
    ((BucketFLY *)ptr->data)->genSymbs = 0;

    initBucketFLY((BucketFLY *) ptr->data, seed, symblen,
                  w, s, lt_delta, lt_c, epsShok, code);

    return nameNewBucket;
}

void rmBucketFLY(LTdecoderFLY** decoder, Node *node) {
    int i, genSymbs;
    Symbol *symbols;
    BucketFLY *buck = (BucketFLY *) node->data;

    if (node == (*decoder)->bucketFLYList)
        (*decoder)->bucketFLYList = node->next;
    // Search the bucket in the list, if it exists
    if (buck != NULL) {
        // free the cumulative
        if (buck->cumulative != NULL) {
            free(buck->cumulative);
            buck->cumulative = NULL;
        }
        // free the symbol list
        genSymbs = buck->genSymbs;
        symbols = buck->symbols;

        for (i=0; i<genSymbs; i++) {

//#ifdef DEBUGdecoderPrintHeaders
//            int j;
//            printf("RMBUCKET - symb[%d]\t: deg=%3d\t -> ", i, buck->symbols[i].deg);
//            for (j=0; j<buck->symbols[i].deg; j++)
//                printf("%3d", buck->symbols[i].header[j]);
//            printf("\n");
//#endif

            if (NULL != buck->symbols[i].header) {
                free(buck->symbols[i].header);
                buck->symbols[i].header = NULL;
            }
            if (NULL != buck->symbols[i].info) {
                free(buck->symbols[i].info);
                buck->symbols[i].info = NULL;
            }
            buck->genSymbs--;
        }
        free(buck->symbols);
        buck->symbols = NULL;
        free(buck);
        buck = NULL;
    }
    rmNode(&node);
}

void initBucketFLY(BucketFLY *bucket, unsigned long seed, int symblen,
                   unsigned int w, unsigned int s, double lt_delta,
                   double lt_c, double epsShok, DegDistr code) {

    bucket->seed = seed;
    bucket->symbLen = symblen;
    bucket->w = w;
    bucket->s = s;
    bucket->lt_delta = lt_delta;
    bucket->lt_c = lt_c;
    bucket->epsShok = epsShok;
    bucket->code = code;

    initRandom(&bucket->buckRan, seed);

    unsigned int i;
    if (bucket->symbols != NULL) {
        for (i=0; i<bucket->genSymbs; i++) {
            if (bucket->symbols[i].header != NULL)
                free(bucket->symbols[i].header);
        }
        free(bucket->symbols);
        bucket->symbols = NULL;
    }

    bucket->genSymbs = 0;

    if (bucket->cumulative != NULL) {
        free(bucket->cumulative);
        bucket->cumulative = NULL;
    }
    bucket->cumulative = (double *) chk_calloc(w, sizeof(double));

    switch (code) {
    case Luby:
        RSDcumulative(bucket->cumulative, w, s, lt_delta, lt_c);
        break;
    case Shokrollahi:
        SHOKcumulative(bucket->cumulative, w, s, epsShok);
        break;
    default:
        print_error("ERROR: currently only LT and Raptor codes are implemented.\n");
        exit(EXIT_FAILURE);
        break;
    }

#ifdef DEBUGdecoderPrintBucketRandomSeed
    printf("Bucket[%d] - Seed: %lu\n", bucket->nameBucket, bucket->seed);
#endif

    return;
}

void createHeadersFly(BucketFLY *bucket,
                      unsigned int newHeadersToGenerate,	// Number of symbols to be retrieved
                      unsigned int offset) {

    double sel;
    unsigned int genDeg;
    unsigned int i, j;

    unsigned int w = bucket->w;
    double *cumulative = bucket->cumulative;

    if (newHeadersToGenerate<1)
        return;

    if ((bucket->genSymbs == 0)&&(bucket->symbols == NULL))
        bucket->symbols = (Symbol *) chk_calloc(newHeadersToGenerate, sizeof(Symbol));
    else {
        bucket->symbols = (Symbol *) chk_realloc(bucket->symbols,
                          bucket->genSymbs + newHeadersToGenerate,
                          sizeof(Symbol));
        for (i=0; i<newHeadersToGenerate; i++) {
            bucket->symbols[bucket->genSymbs+i].info = NULL;
            bucket->symbols[bucket->genSymbs+i].header = NULL;
            bucket->symbols[bucket->genSymbs+i].deg = 0;
            bucket->symbols[bucket->genSymbs+i].symbLen = bucket->symbLen;
        }
    }
    unsigned int *selectedSymbols = NULL;
    selectedSymbols = (unsigned int*) chk_malloc(w, sizeof(unsigned int));

    for (i=0; i<newHeadersToGenerate; i++) {
        // Create header (the symbols are chosen between 0 and w)
        //  therefore shifting is needed.
        genDeg = degree(cumulative, w, real(&bucket->buckRan));
        if (genDeg > w)
            genDeg = w;
        if (genDeg == w) {
            chooseAllInWindowCore(w, offset, selectedSymbols, genDeg);
        } else {
            for (j=0; j<genDeg; j++) {
                sel	= real(&bucket->buckRan);
                selectedSymbols[j]	= rand2windowCore(w, offset, sel);

                if (verifySelectedSymbol(selectedSymbols, j) == False) {
                    j--;		// Doesn't repeat encoding symbols
                }
            }
//			sort(selectedSymbols, 0, genDeg-1);	// Sort the chosen encoding symbols' list
        }

#ifdef DEBUGdecoderPrintHeaders
        int deb_index;
        printf("BUCKET[%d] - symb[%d]\t: deg=%3d\t -> ",
               bucket->nameBucket, bucket->genSymbs, genDeg);
        for (deb_index=0; deb_index<genDeg; deb_index++)
            printf("%3d ", selectedSymbols[deb_index]);
        printf("\n");
#endif

        bucket->symbols[bucket->genSymbs].header =
            (unsigned int *) chk_malloc(genDeg, sizeof(unsigned int *));

        memcpy(bucket->symbols[bucket->genSymbs].header,
               selectedSymbols,
               genDeg*sizeof(unsigned int));
        bucket->symbols[bucket->genSymbs].deg = genDeg;
        bucket->symbols[bucket->genSymbs].symbLen = bucket->symbLen;

        bucket->genSymbs++;
    }

#ifdef DEBUGdecoderPrintHeaders
    printf("\n");
#endif

    free(selectedSymbols);
    selectedSymbols = NULL;
    return;
}

unsigned int receiveSymbFLY(LTdecoderFLY *decoder, BucketFLY *bucket,
                            char *encodedInfo,			// encodedInfo is the vector of encoded bits
                            unsigned int encodedInfoLength,  	// length of the encodedInfo vector (thus number of encoded bits)
                            unsigned int IDfirstSymb,			// ID of the first encoded symbol
                            unsigned long offset) {				// ID of the first source symbol considered in the encoding window

    unsigned int i;
    unsigned int count = 0;
    Node *ptr= NULL;
    unsigned int packetSymbols = (unsigned int)(encodedInfoLength/(bucket->symbLen));

    if (IDfirstSymb+packetSymbols > bucket->genSymbs) {
        createHeadersFly(bucket,
                         (IDfirstSymb+packetSymbols)-(bucket->genSymbs),
                         offset);
    }

    for (i=0; i<packetSymbols; i++) {

        // Create Node
        if (decoder->RXsymbList == NULL) {
            ptr = addNode(NULL);
            decoder->RXsymbList = ptr;
        } else {
            ptr = addNode(decoder->lastRXsymbol);
            decoder->lastRXsymbol->next = ptr;
        }
        decoder->lastRXsymbol = ptr;
        count++;

        // Alloc a new symbol in the decoder
        ptr->data = (Symbol *) chk_calloc(1, sizeof(Symbol));
        Symbol *newSymbol = (Symbol *)ptr->data;

        // Create pointer to the current symbol in the bucket
        Symbol* currSymbol = &(bucket->symbols[IDfirstSymb+i]);

        // Deg
        newSymbol->deg = currSymbol->deg;
        // Symb Len
        newSymbol->symbLen = currSymbol->symbLen;
        // Header
        newSymbol->header =
            (unsigned int *)chk_malloc(newSymbol->deg, sizeof(unsigned int));
        memcpy(newSymbol->header,
               currSymbol->header,
               newSymbol->deg*sizeof(unsigned int));
        // Info
        newSymbol->info =
            (char *)chk_malloc(bucket->symbLen, sizeof(char));
        memcpy(newSymbol->info,
               &encodedInfo[i*bucket->symbLen],
               bucket->symbLen*sizeof(char));

#ifdef DEBUGdecoderPrintHeaders
        int deb_index;
        printf("DECODER[%d] - symb[%d]\t: deg=%3d\t -> ",
               bucket->nameBucket, IDfirstSymb+i, newSymbol->deg);
        for (deb_index=0; deb_index<newSymbol->deg; deb_index++)
            printf("%3d ", currSymbol->header[deb_index]);
        printf("\n");
#endif

    }
    // Update the number of received symbols
    decoder->NumRecSymbols += count;

    return count;
}

unsigned int receiveEncodedSymbFLY(LTdecoderFLY *decoder, Symbol* encodedSymbols,
                                   unsigned int numberEncodedSymbols) {

    unsigned int i;
    unsigned int count = 0;
    Node *ptr= NULL;

    if (decoder == NULL) {
        print_error("(libLT_C:LTdecoderFLY.c:receiveEncodedSymbFLY) ERROR - Decoder pointer not correctly passed.\n");
        return 0;
    }

    if ((encodedSymbols == NULL)||(numberEncodedSymbols == 0))
        return 0;

    for (i = 0; i < numberEncodedSymbols; i++) {

        Symbol* currSymb = &(encodedSymbols[i]);

        // Create Node
        if (decoder->RXsymbList == NULL) {
            ptr = addNode(NULL);
            decoder->RXsymbList = ptr;
        } else {
            ptr = addNode(decoder->lastRXsymbol);
            decoder->lastRXsymbol->next = ptr;
        }
        decoder->lastRXsymbol = ptr;
        count++;

        // Alloc a new symbol in the decoder
        ptr->data = (Symbol *) chk_calloc(1, sizeof(Symbol));
        Symbol *newSymbol = (Symbol *)ptr->data;

        // Deg
        newSymbol->deg = currSymb->deg;

        // Symb Len
        newSymbol->symbLen = currSymb->symbLen;

        // Header
        newSymbol->header =
            (unsigned int *) chk_malloc(newSymbol->deg, sizeof(unsigned int));
        memcpy(newSymbol->header,
               currSymb->header,
               newSymbol->deg*sizeof(unsigned int));

        // Info
        newSymbol->info =
            (char *) chk_malloc(currSymb->symbLen, sizeof(char));
        memcpy(newSymbol->info,
               currSymb->info,
               currSymb->symbLen*sizeof(char));

        // Update the number of received symbols
        decoder->NumRecSymbols++;
    }
    return count;
}

BucketFLY *getBucketFLY(LTdecoderFLY* decoder, int nameBucket) {
    Node *current = decoder->bucketFLYList;
    BucketFLY *currBuck = NULL;
    while (current != NULL) {
        currBuck = (BucketFLY *) current->data;
        if (currBuck->nameBucket != nameBucket)
            current = current->next;
        else
            return currBuck;
    }
    return NULL;
}

Node *getNodeBucketFLY(LTdecoderFLY* decoder, int nameBucket) {
    Node *current = decoder->bucketFLYList;

    if (nameBucket < 0) return NULL;

    while (current != NULL) {
        if (((BucketFLY *) current->data)->nameBucket == nameBucket)
            return (Node *) current;
        else
            current = current->next;
    }
    return NULL;
}

int getNameBucketFLY(LTdecoderFLY* decoder, unsigned long seed,
                     unsigned int w, unsigned int s,
                     double lt_delta, double lt_c, double epsShok) {

    Node *current = decoder->bucketFLYList;
    BucketFLY *tmp = NULL;
    while (current != NULL) {
        tmp = (BucketFLY *) current->data;
        if ((tmp->seed == seed) &&
                (tmp->w == w) && (tmp->s == s) &&
                (tmp->lt_delta == lt_delta) && (tmp->lt_c == lt_c) &&
                (tmp->epsShok == epsShok))
            return tmp->nameBucket;
        else
            current = current->next;
    }
    return -1;
}

// ------------------------------------------------------------------

unsigned int countUndecSymbInWindow(char *decodingStatus,       // vector of decodingStatus
                                    unsigned int startIndex,    // index of the fist byte to be checked
                                    unsigned int winLengthByte, // number of bytes to be checked
                                    int symbLen) {              // symbol length
    unsigned int undecodedBytes = 0;
    int i;
    for (i=0; i<winLengthByte; i++)
        if (0x00 == decodingStatus[startIndex+i])
            undecodedBytes++;
    return undecodedBytes/symbLen;
}

char *getDecodedInformation(LTdecoderFLY *decoder) {
    return decoder->decoded;
}

char *getDecodingStatusInformation(LTdecoderFLY *decoder) {
    return decoder->decodingStatus;
}

int getNumDecoded(LTdecoderFLY *decoder) {
    return decoder->NumDecSymbols;
}

int getNumReceived(LTdecoderFLY *decoder) {
    return decoder->NumRecSymbols;
}

int addAndInitBucketFLY_LT(LTdecoderFLY *decoder, unsigned long seed,
                           int symblen, unsigned int w, unsigned int s,
                           double lt_delta, double lt_c) {
    return addAndInitBucketFLY(decoder, seed, symblen, w, s, lt_delta, lt_c, 0.0, Luby);
}

int addAndInitBucketFLY_Raptor(LTdecoderFLY *decoder, unsigned long seed,
                               int symblen, unsigned int w, unsigned int s,
                               double epsShok) {
    return addAndInitBucketFLY(decoder, seed, symblen, w, s, 0.0, 0.0, epsShok, Shokrollahi);
}

unsigned int receiveSymbFLY_BucketName(LTdecoderFLY *decoder, int bucketName,
                                       char *encodedInfo, unsigned int encodedInfoLength,
                                       unsigned int IDfirstSymb, unsigned long offset) {

    BucketFLY *bucket = getBucketFLY(decoder, bucketName);

    return receiveSymbFLY(decoder, bucket,
                          encodedInfo, encodedInfoLength,
                          IDfirstSymb, offset);
}

char *LTdecodeFLY_fromFile(LTdecoderFLY *decoder,
                          char *infile) {
    char* recSerializedBuf = NULL;
    unsigned long long recSerializedBufLen = 0;
    recSerializedBuf = readFile(infile, &recSerializedBufLen);

    // De-serialize buffer
    Symbol* receivedEncodedSymbols = NULL;
    unsigned int recN = 0;
    deserializeSymbolList(recSerializedBuf, &receivedEncodedSymbols, &recN);

    printf("End deserialize symbol list\n");

    // Free buffer
    free(recSerializedBuf);

    // Receive the encoded symbols
    receiveEncodedSymbFLY(decoder, receivedEncodedSymbols, recN);

    printf("End receive encoded symbols, recN:%d\n",recN);

    // Delete symbols
    unsigned int i;
    for(i = 0; i < recN; i++ ) {
        Symbol* currSymb = &(receivedEncodedSymbols[i]);
        if (currSymb->info != NULL)
            free(currSymb->info);
        if (currSymb->header != NULL)
            free(currSymb->header);
    }
    free(receivedEncodedSymbols);

    printf("Start decoding process\n");

    decodingProcessFLY(decoder);

    printf("End decoding process\n");

    return getDecodedInformation(decoder);
}

char *LTdecodeFLY_fromFiles(LTdecoderFLY *decoder, int numinfiles, char **infiles) {
    int i = 0;
    for (; i<numinfiles; i++) {
        LTdecodeFLY_fromFile(decoder,infiles[i]);
    }

    return getDecodedInformation(decoder);
}

#ifdef DEBUGprintReleasedSymbol
void printReleasedSymbol(unsigned int check, unsigned int k) {
//    int i;
//    for (i=0; i<k; i++) {
//        if (i==check) {
//            printf("1");
//        } else
//            printf("0");
//    }
//    printf("\n");
//    return;
    printf("Symbol %u decoded\n", check);
}
#endif
