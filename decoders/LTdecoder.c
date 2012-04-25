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
// File       : decoder.c
// Author     : Pasquale Cataldi, Andrea Tomatis
// Company    :
// Last update: 2009/04/25
// Platform   : Ansi C
//*****************************************************************************
// Description: This file contains the LT decoder
//*****************************************************************************
// Revisions  :
// Date        Version  Author  Description
// 2008/07/17  0.1      PC,AT   First release
// 2008/07/17  0.2      PC,AT   Added getBucket and addBucket returns
//								the name of the bucket created.
// 2008/07/24  0.3		PC,AT	Removed elapsed time from decoding_process_p2p
// 2008/11/20  0.4      PC      The functions initBucket and reSeedBucket will
//                              call the function "create_headers" that was
//                              formerly named "create_headers_p2p".
//                              The functions "initBucket" and "reSeedBucket"
//                              now return void instead of the not exloited int.
//                              Added the functions "initBucket_nw", "reSeedBucket_nw"
//                              and "addBucket_nw"
// 2008/12/09  0.5      PC      In process_receive check if current == NULL. In
//                              this case return false (no symbols have been received yet.
//                              In process_receive, after decoding one symbol, if the
//                              decoding is complete, then stop the iterations
// 2009/02/19  0.6      PC      Changed head (the list of buckets) to bucketList
// 2009/02/22  0.7	    PC      Changed "NumSymbols" to "NumRecSymbols"
//								Changed "RXsymbols" to "RXsymbList"
//								Changed "while (current->next!=NULL)" to "while (current!=NULL)"
//								in process_received
// 2009/04/25  0.8      PC		Added at the end of process_received() a while
//								to update the pointer lastRXsymbol
// 2010/12/30  0.9      PC      Added safe allocation and error printing
//*****************************************************************************


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "LTdecoder.h"
#include "../utils/sharedMethods.h"
#include "../utils/utilMem.h"
#include "../utils/utilErr.h"

#define noDEBUGprintReleasedSymbol

LTdecoder* mallocDecoder(void) {
    LTdecoder *dec = chk_malloc(1, sizeof(LTdecoder));

    dec->RXsymbList = NULL;
    dec->lastRXsymbol = NULL;
    dec->decoded = NULL;
    dec->bucketList = NULL;
    dec->NumRecSymbols = 0;
    dec->NumDecSymbols = 0;
    dec->decodingStatus = NULL;

    // SIMD
    dec->xorType = isSIMD();

    return dec;
}

void freeDecoder(LTdecoder *decoder) {
    Node *temp= NULL;
    int i;

    if (decoder != NULL) {

        if (decoder->decoded != NULL)
            free(decoder->decoded);

        if (decoder->decodingStatus != NULL)
            free(decoder->decodingStatus);

        if (decoder->bucketList != NULL) {

            while (decoder->bucketList != NULL) {
                temp = decoder->bucketList->next;

                if (decoder->bucketList->data != NULL) {
                    Mgenerator currBucketMatrix = ((Bucket *) decoder->bucketList->data)->matrix;
                    unsigned int N = currBucketMatrix.conf.N;

                    // Free Bucket structures
                    for (i=0; i<N; i++) {
                        if (currBucketMatrix.symbols[i].header
                                != NULL)
                            free(currBucketMatrix.symbols[i].header);
                    }

                    if (currBucketMatrix.cumulative!=NULL)
                        free(currBucketMatrix.cumulative);

                    if (currBucketMatrix.symbols != NULL)
                        free(currBucketMatrix.symbols);

                    free(decoder->bucketList->data);
                    decoder->bucketList->data = NULL;
                }

                free(decoder->bucketList);
                decoder->bucketList = temp;
            }
        }

        if (decoder->RXsymbList != NULL) {

            while (decoder->RXsymbList != NULL) {
                temp = decoder->RXsymbList->next;

                if (decoder->RXsymbList->data != NULL) {
                    // Free Symbol structure
                    free(((Symbol *)decoder->RXsymbList->data)->header);
                    free(((Symbol *)decoder->RXsymbList->data)->info);

                    free(decoder->RXsymbList->data);
                    decoder->RXsymbList->data = NULL;
                }

                free(decoder->RXsymbList);
                decoder->RXsymbList = temp;
            }
        }
        free(decoder);
    }
}

void reinitDecoder(LTdecoder *decoder) {
    Node *temp= NULL;

    if ((Bucket *)decoder->bucketList == NULL) {
        print_error("ERROR: reinitDecoder - no bucket.\n");
        exit(EXIT_FAILURE);
    }

    unsigned int k = ((Bucket *)decoder->bucketList->data)->matrix.conf.k;
    unsigned int symbLen = ((Bucket *)decoder->bucketList->data)->symbLen;

    if (decoder->bucketList != NULL) {
        decoder->NumDecSymbols = 0;
        decoder->NumRecSymbols = 0;

        // Free RXsymbList list
        if (decoder->RXsymbList != NULL) {

            while (decoder->RXsymbList != NULL) {
                temp = decoder->RXsymbList->next;

                if (decoder->RXsymbList->data != NULL) {
                    // Free Symbol structure
                    free(((Symbol *)decoder->RXsymbList->data)->header);
                    free(((Symbol *)decoder->RXsymbList->data)->info);

                    free(decoder->RXsymbList->data);
                    decoder->RXsymbList->data = NULL;
                }

                free(decoder->RXsymbList);
                decoder->RXsymbList = temp;
            }
        }

        // Reset decoded symbol vector
        if (decoder->decoded == NULL) {
            decoder->decoded = (char *) chk_malloc(k*symbLen, sizeof(char));
        }
    } else
        print_error("Error: reinitDecoder failed - No buckets allocated.\n");
}

void initBucket(Bucket *bucket, unsigned long seed, int symblen,
                unsigned int k, unsigned int w, unsigned int s, double lt_delta,
                double lt_c, double eps, unsigned int N, DegDistr code) {

    unsigned int i;

    bucket->matrix.seed = seed;
    bucket->symbLen = symblen;
    bucket->matrix.codetype = code;

    bucket->matrix.conf.k = k;
    bucket->matrix.conf.w = w;
    bucket->matrix.conf.s = s;
    bucket->matrix.conf.N = N;
    bucket->matrix.conf.delta = lt_delta;
    bucket->matrix.conf.c = lt_c;
    bucket->matrix.conf.epsShok = eps;

    if (bucket->matrix.symbols != NULL) {

        for (i=0; i<N; i++) {
            if (bucket->matrix.symbols[i].header != NULL)
                free(bucket->matrix.symbols[i].header);
        }

        free(bucket->matrix.symbols);
    }

    bucket->matrix.symbols = (Symbol *) chk_calloc(N, sizeof(Symbol));

    if (bucket->matrix.cumulative != NULL)
        free(bucket->matrix.cumulative);
    bucket->matrix.cumulative = (double *) chk_calloc(w, sizeof(double));

    switch (code) {
    case Luby:
        rubust_soliton_distribution(&bucket->matrix);
        break;
    case Shokrollahi:
        shokrollahi_distribution(&bucket->matrix);
        break;
    default:
        print_error("ERROR: currently only LT and Raptor codes are implemented.\n");
        exit(EXIT_FAILURE);
        break;
    }

    create_headers(&bucket->matrix);

    return;
}

void initBucket_nw(Bucket *bucket, unsigned long seed, int symblen,
                   unsigned int k, unsigned int w, unsigned int s, double lt_delta,
                   double lt_c, double eps, unsigned int N, DegDistr code, unsigned int nw, unsigned int Ns) {

    unsigned int i;

    bucket->matrix.seed = seed;
    bucket->symbLen = symblen;
    bucket->matrix.codetype = code;

    bucket->matrix.conf.k = k;
    bucket->matrix.conf.w = w;
    bucket->matrix.conf.s = s;
    bucket->matrix.conf.N = N;
    bucket->matrix.conf.delta = lt_delta;
    bucket->matrix.conf.c = lt_c;
    bucket->matrix.conf.epsShok = eps;

    if (bucket->matrix.symbols != NULL) {

        for (i=0; i<N; i++) {
            if (bucket->matrix.symbols[i].header != NULL)
                free(bucket->matrix.symbols[i].header);
        }

        free(bucket->matrix.symbols);
    }

    bucket->matrix.symbols = (Symbol *) chk_calloc(N, sizeof(Symbol));

    if (bucket->matrix.cumulative != NULL)
        free(bucket->matrix.cumulative);
    bucket->matrix.cumulative = (double *) chk_calloc(w, sizeof(double));

    switch (code) {
    case Luby:
        rubust_soliton_distribution(&bucket->matrix);
        break;
    case Shokrollahi:
        shokrollahi_distribution(&bucket->matrix);
        break;
    default:
        print_error("ERROR: currently only p2p and streaming applications can be used.\n");
        exit(EXIT_FAILURE);
        break;
    }

    if (nw == 0)
        create_headers(&bucket->matrix);
    else
        create_headers_nw(&bucket->matrix, nw, Ns);

    return;
}

void reSeedBucket(Bucket *bucket, unsigned long seed) {
    unsigned int i;

    bucket->matrix.seed = seed;

    for (i=0; i<bucket->matrix.conf.N; i++) {
        if (bucket->matrix.symbols[i].header != NULL)
            free(bucket->matrix.symbols[i].header);
    }

    create_headers(&bucket->matrix);

    return;
}

void reSeedBucket_nw(Bucket *bucket, unsigned long seed, unsigned int nw, unsigned int Ns) {
    unsigned int i;

    bucket->matrix.seed = seed;

    for (i=0; i<bucket->matrix.conf.N; i++) {
        if (bucket->matrix.symbols[i].header != NULL)
            free(bucket->matrix.symbols[i].header);
    }

    if (nw == 0)
        create_headers(&bucket->matrix);
    else
        create_headers_nw(&bucket->matrix, nw, Ns);

    return;
}

int addBucket(LTdecoder* decoder, unsigned long seed, int symblen,
              unsigned int k, unsigned int w, unsigned int s, double lt_delta,
              double lt_c, double eps, unsigned int N, DegDistr code) {

    int nameNewBucket = 0;
    Node *current = decoder->bucketList;
    Node *ptr= NULL;

    if (decoder->bucketList == NULL) {
        ptr = addNode(NULL);
        decoder->bucketList = ptr;
    } else {
        while (current->next != NULL) {
            current = current->next;
            nameNewBucket++;
        }
        ptr = addNode(current);
        current->next = ptr;
    }

    ptr->data = (void *) chk_malloc(1, sizeof(Bucket));

    ((Bucket *)ptr->data)->matrix.symbols = NULL;
    ((Bucket *)ptr->data)->matrix.cumulative = NULL;
    ((Bucket *)ptr->data)->nameBucket = nameNewBucket;

    initBucket((Bucket *) ptr->data, seed, symblen, k, w, s, lt_delta, lt_c,
               eps, N, code);

    return nameNewBucket;
}

int addBucket_nw(LTdecoder* decoder, unsigned long seed, int symblen,
                 unsigned int k, unsigned int w, unsigned int s, double lt_delta,
                 double lt_c, double eps, unsigned int N, DegDistr code, unsigned int nw, unsigned int Ns) {

    int nameNewBucket = 0;
    Node *current = decoder->bucketList;
    Node *ptr= NULL;

    if (decoder->bucketList == NULL) {
        ptr = addNode(NULL);
        decoder->bucketList = ptr;
    } else {
        while (current->next != NULL) {
            current = current->next;
            nameNewBucket++;
        }
        ptr = addNode(current);
        current->next = ptr;
    }

    ptr->data = (void *) chk_malloc(1, sizeof(Bucket));

    ((Bucket *)ptr->data)->matrix.symbols = NULL;
    ((Bucket *)ptr->data)->matrix.cumulative = NULL;
    ((Bucket *)ptr->data)->nameBucket = nameNewBucket;

    if (nw == 0)
        initBucket((Bucket *) ptr->data, seed, symblen, k, w, s, lt_delta,
                   lt_c, eps, N, code);
    else
        initBucket_nw((Bucket *) ptr->data, seed, symblen, k, w, s, lt_delta,
                      lt_c, eps, N, code, nw, Ns);

    return nameNewBucket;
}

void rmBucket(LTdecoder** decoder, Node *node) {

    unsigned int i;
    unsigned int N = ((Bucket *)node->data)->matrix.conf.N;

    if (node == (*decoder)->bucketList)
        (*decoder)->bucketList = node->next;

    if (node->data != NULL) {
        // Free Bucket structures
        for (i=0; i<N; i++) {
            if (((Bucket *)node->data)->matrix.symbols[i].header
                    != NULL)
                free(((Bucket *)node->data)->matrix.symbols[i].header);
        }

        if (((Bucket *)node->data)->matrix.cumulative != NULL)
            free(((Bucket *)node->data)->matrix.cumulative);

        if (((Bucket *)node->data)->matrix.symbols != NULL)
            free(((Bucket *)node->data)->matrix.symbols);
        free(node->data);
    }

    rmNode(&node);
}

Bucket *getBucket(LTdecoder* decoder, int nameBucket) {
    Node *current = decoder->bucketList;
    while (current != NULL) {
        if (((Bucket *) current->data)->nameBucket == nameBucket)
            return (Bucket *) current->data;
        else
            current = current->next;
    }
    return NULL;
}

Node *getNodeBucket(LTdecoder* decoder, int nameBucket) {
    Node *current = decoder->bucketList;
    while (current != NULL) {
        if (((Bucket *) current->data)->nameBucket == nameBucket)
            return (Node *) current;
        else
            current = current->next;
    }
    return NULL;
}

unsigned int genRXSymb(Bucket *bucket, char *encodedInfo, unsigned int length,
                       Node **RXsymbList, Node **lastRXsymbol, unsigned int start) {

    unsigned int i;
    unsigned int count = 0;
    Node *ptr= NULL;
    unsigned int packetSymbols = (unsigned int)(length/(bucket->symbLen));


    for (i=0; i<packetSymbols; i++) {

        // Create Node
        if ((*RXsymbList) == NULL) {
            ptr = addNode(NULL);
            (*RXsymbList) = ptr;
        } else {
            ptr = addNode((*lastRXsymbol));
            (*lastRXsymbol)->next = ptr;
        }
        (*lastRXsymbol) = ptr;
        count++;

        ptr->data = (void *) chk_malloc(1, sizeof(Symbol));

        // Deg
        ((Symbol *)ptr->data)->deg
        = bucket->matrix.symbols[start+i].deg;

        // Header
        ((Symbol *)ptr->data)->header = (unsigned int *) chk_malloc(
                                            ((Symbol *)ptr->data)->deg, sizeof(unsigned int));
        memcpy(((Symbol *)ptr->data)->header,
               bucket->matrix.symbols[start+i].header,
               ((Symbol *)ptr->data)->deg * sizeof(unsigned int));

        // Info
        ((Symbol *)ptr->data)->info
        = (char *) chk_malloc(bucket->symbLen, sizeof(char));
        memcpy(((Symbol *)ptr->data)->info,
               &encodedInfo[i*bucket->symbLen],
               bucket->symbLen * sizeof(char));

    }
    return count;
}

unsigned int receiveSymb_BucketName(LTdecoder *decoder, int bucketName,
                                    char *encodedInfo, unsigned int encodedInfoLength,
                                    unsigned int IDfirstSymb) {

    Bucket *bucket = getBucket(decoder, bucketName);

    return genRXSymb(bucket, encodedInfo, encodedInfoLength,
                     &(decoder->RXsymbList), &(decoder->lastRXsymbol), IDfirstSymb);
}

#ifdef DEBUGprintReleasedSymbol
void printReleasedSymbol(unsigned int check, unsigned int k) {
    int i;
    for (i=0; i<k; i++) {
        if (i==check) {
            printf("1");
        } else
            printf("0");
    }
    printf("\n");
    return;
}
#endif

// Processes the received symbols
Tbool processReceived(LTdecoder* decoder) {
    Tbool one_decoded = False;
    unsigned int curr_degree = 0;
    unsigned int real_degree = 0;
    unsigned int check = 0;
    Node *nextNode= NULL;
    int symbLen = 0;
    unsigned int k = 0;

    // Start from the first received symbol (if exists)
    Node *current = NULL;
    current = decoder->RXsymbList;

    // All the symbols in the decode MUST have the same length!!
    symbLen = ((Bucket *)decoder->bucketList->data)->symbLen;
    k = ((Bucket *)decoder->bucketList->data)->matrix.conf.k;
    if ((symbLen==0)||(k==0)) {
        print_error("ERROR: process_received - either symbLen or k invalid.\n");
        exit(EXIT_FAILURE);
    }

    // If the decoded buffer has not been allocated yet, allocate it
    if (decoder->decoded == NULL) {
        decoder->decoded = (char *) chk_malloc(k*symbLen, sizeof(char));
//        memset(decoder->decoded, 'X', k*symbLen*sizeof(char));
    }
    // If the decodingStatus buffer has not been allocated yet, allocate it
    if (decoder->decodingStatus == NULL) {
        decoder->decodingStatus = (char *) chk_malloc(k*symbLen, sizeof(char));
        memset(decoder->decodingStatus, 0x00, k*symbLen*sizeof(char));
    }

    while (current != NULL) {

        nextNode = current->next;

        if (current->data == NULL) {
            print_error("ERROR: process_received - current data empty.\n");
            exit(EXIT_FAILURE);
        }

        curr_degree = ((Symbol *) current->data)->deg;

        if ((curr_degree<1)&&(curr_degree>k)) {	// It shouldn't happen
            print_error("ERROR: process_received - out of bounds degree found.\n");
            rmSymbol(&(decoder->RXsymbList), current);
        }

        if (curr_degree == 1) { // If this symbol has deg = 1 then directly decode it
            check = ((Symbol *) current->data)->header[0];

            if ((check > k)||(check < 0)) {
                print_error("ERROR: encoded symbols contain a check out of bounds!\n");
                exit(EXIT_FAILURE);
            }

            // Check if the symbol in the header has not being decoded yet.
            if (isSymbolDecoded(decoder->decodingStatus, check*symbLen, symbLen) == False) {

                // Copy the information in the decoded information array
                memcpy(&decoder->decoded[check*symbLen],
                       ((Symbol *) current->data)->info,
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
            real_degree = computeRealDegree(((Symbol *) current->data),
                                            decoder->decodingStatus,
                                            symbLen, curr_degree, k);
            switch (real_degree) {
            case 0:
                rmSymbol(&(decoder->RXsymbList), current);
                break;
            case 1:
                check = reduceDegree((Symbol **)(&current->data),
                                     decoder->decoded, decoder->decodingStatus, curr_degree, symbLen, real_degree,
                                     decoder->xorType);

                if (((Symbol *) current->data)->info == NULL) {
                    print_error("ERROR: symbol info not allocated!\n");
                    exit(EXIT_FAILURE);
                }

                if (check>=k) {
                    print_error("ERROR: decoded symbol out of buffer.\n");
                    exit(EXIT_FAILURE);
                }

                if (decoder->decodingStatus[check*symbLen] == 0x00) {
                    memcpy(&decoder->decoded[check*symbLen],
                           ((Symbol *) current->data)->info,
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

        if (decoder->NumDecSymbols == k) {
            return False;
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

Tbool decodingProcess(LTdecoder* decoder) {

    Tbool one_decoded = True;
    unsigned int k = ((Bucket *) decoder->bucketList->data)->matrix.conf.k;

    if (decoder == NULL) {
        print_error("Error: decodingProcess - No decoder!\n");
        exit(EXIT_FAILURE);
    }

    if (decoder->bucketList == NULL) {
        print_error("Error: decodingProcess - No bucket.\n");
        exit(EXIT_FAILURE);
    }

    while ((decoder->RXsymbList != NULL)&&(one_decoded == True)) {
        one_decoded = processReceived(decoder);
    }

    // Important only for LT codes
    if (decoder->NumDecSymbols == k)
        return True;

    return False;
}

unsigned int getNumRecSymbols(LTdecoder *decoder) {
    return decoder->NumRecSymbols;
}

unsigned int getNumDecSymbols(LTdecoder *decoder) {
    return decoder->NumDecSymbols;
}

char *getDecoded(LTdecoder *decoder) {
    return decoder->decoded;
}
