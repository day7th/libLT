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
// Description: This file contains the LT encoder LFY implementation
//*****************************************************************************
// Revisions  :
// Date        Version  Author  Description
// 2010/12/30  0.2      PC      First traced version.
//*****************************************************************************

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "LTencoderFLY.h"
#include "../utils/sharedMethods.h"
#include "../utils/utilMem.h"
#include "../utils/utilErr.h"
#include "../utils/utilTlv.h"
#include "../utils/ciofile.h"

// ============================================
// 			Private functions declarations
// ============================================

// Create the degree distribution
void createDegreeDistribution(LTencoderFLY *encoder,
                              double *cumulative, unsigned int w, unsigned int s,
                              double lt_delta, double lt_c, double raptor_eps);

// ============================================
// 			Public functions implementation
// ============================================

LTencoderFLY* mallocEncoderFLY(unsigned long seed) {

    LTencoderFLY *encoder = chk_malloc(1, sizeof(LTencoderFLY));
    encoder->refRandomSeed = seed;	// set the first randomSeed

    encoder->xorType	= isSIMD();
    encoder->totGenSymbols	= 0;

    // Create the LThashTable for the degree distributions
    encoder->DistributionsTable = create_hashtable(MINHASHTABLESIZE, hashfromkey, equalkeys);
    if (encoder->DistributionsTable == NULL)
        exit(-1); /*oom*/

#ifdef DEBUGencoderPrintBucketRandomSeed
    printf("Encoder[malloc()] - Seed: %lu\n", encoder->refRandomSeed);
#endif

    return encoder;
}

void freeEncoderFLY(LTencoderFLY *encoder) {

    destroyLThashTable(encoder->DistributionsTable);
    free(encoder);
    encoder = NULL;
}


void setEncoderFLY(LTencoderFLY *encoder, DegDistr code,
                   unsigned int w, unsigned int s,
                   double lt_delta, double lt_c, double raptor_eps) {

    struct hashtable *h = encoder->DistributionsTable;

    encoder->code	= code;			// set the code type (LT or Raptor)

    struct value *v = NULL;
    v = searchEntryInLThashTable(h, w, s, lt_delta, lt_c, raptor_eps);

    if (NULL == v) {
        if ((v = insertEntryInLThashTable(h, w, s, lt_delta, lt_c, raptor_eps)) != NULL) {
            // Create the degree distribution
            createDegreeDistribution(encoder, (v->cumulative), w, s, lt_delta, lt_c, raptor_eps);
            // Set the randm for this configuration
            v->confSeed = encoder->refRandomSeed + hashtable_count(h)-1;
            initRandom(&v->randomGenerator, v->confSeed);
        }
    }

    // Update the current cumulative and the current w.
    encoder->currConfig = v;

    return;
}

void reseedEncoderFLY(LTencoderFLY *encoder, unsigned long newSeed) {

    // Reinit the totGenSymbols and the refRandomSeed
    encoder->totGenSymbols = 0;
    unsigned long oldSeed = encoder->refRandomSeed;
    encoder->refRandomSeed = newSeed;
    unsigned long oldConfSeed;
    struct hashtable *h = encoder->DistributionsTable;

#ifdef DEBUGencoderPrintBucketRandomSeed
    printf("Encoder[reseed()] - Seed: %lu\n", encoder->refRandomSeed);
#endif

    struct hashtable_itr *itr;
    struct key *k;
    struct value *v;

    // Update all the configurations, if any
    itr = hashtable_iterator(h);
    if (hashtable_count(h) > 0) {
        do {
            k = hashtable_iterator_key(itr);
            v = hashtable_iterator_value(itr);

            // Update the confSeed
            oldConfSeed = v->confSeed;
            v->confSeed = newSeed + (oldConfSeed-oldSeed);

            // Reinit the random generator
            initRandom(&v->randomGenerator, v->confSeed);

            // Erase the number of generated symbols
            v->genSymbols = 0;
        } while (hashtable_iterator_advance(itr));
    }
    free(itr);
    itr = NULL;
    return;
}


void resetEncoderFLY(LTencoderFLY *encoder, unsigned long newSeed) {

    // Reinit the totGenSymbols and the refRandomSeed
    encoder->totGenSymbols	= 0;
    encoder->refRandomSeed = newSeed;

    // Destroy and reconstruct the hashtable
    destroyLThashTable(encoder->DistributionsTable);
    encoder->DistributionsTable = NULL;
    encoder->DistributionsTable = create_hashtable(MINHASHTABLESIZE, hashfromkey, equalkeys);
    if (NULL == encoder->DistributionsTable) exit(EXIT_FAILURE); /*oom*/

    return;
}

unsigned int LTencodeInfoFLY(LTencoderFLY *encoder, int symbLen,
                             char *info, unsigned long window_firstSymbolId,	// window_firstSymbolId indicates the the first symbol of the window
                             unsigned int N, char *output) {

    unsigned int genDeg = 0;
    unsigned int *selectedSymbols = NULL;
    unsigned int ii, jj, j, kk;
    double sel;

    unsigned int w = encoder->currConfig->config->w;
    double *cumulative = encoder->currConfig->cumulative;
    unsigned int IDfirstSymbol = encoder->currConfig->genSymbols;

    // The output must be already allocated
    if ((symbLen < 1)||(NULL == info)||(NULL == output)) {
        print_error("ERROR - encoding process - no encoded information generated.\n");
        if (symbLen < 1) print_error("                           symbLen = %d\n", symbLen);
        if (NULL == info) print_error("                           info vector not allocated\n");
        if (NULL == output) print_error("                           output vector not allocated\n");
        exit(EXIT_FAILURE);
    }

    // Allocate the vector for the selected symbols
    selectedSymbols = (unsigned int*) chk_calloc(w, sizeof(unsigned int));

    for (ii=0; ii<N; ii++) {

        genDeg = degree(cumulative, w, real(&(encoder->currConfig)->randomGenerator));
        if (genDeg > w)
            genDeg = w;

        if (genDeg == w) {
            chooseAllInWindowCore(w, window_firstSymbolId, selectedSymbols, genDeg);

        } else {
            for (jj=0; jj<genDeg; jj++) {
                sel	= real(&(encoder->currConfig)->randomGenerator);
                selectedSymbols[jj]	= rand2windowCore(w, window_firstSymbolId, sel);

                if (verifySelectedSymbol(selectedSymbols, jj) == False) {
                    jj--;		// Doesn't repeat encoding symbols
                }
            }
            // This encoder does not sort because does not save the headers
            // sort(selectedSymbols, 0, genDeg-1);	// Sort the chosen encoding symbols' list
        }

#ifdef DEBUGencoderPrintHeaders
        int deb_index;
        printf("ENCODER - symb[%d]\t: deg=%3d\t -> ", (int) encoder->currConfig->genSymbols, genDeg);
        for (deb_index=0; deb_index<genDeg; deb_index++)
            printf("%3d ", selectedSymbols[deb_index]);
        printf("\n");
#endif

        // Copy the Info of the first input symbol indicated in the header
        memcpy(&output[ii*symbLen], &info[selectedSymbols[0]*symbLen], symbLen*sizeof(char));

        // EX-OR the other symbols of the header
        if (encoder->xorType != NO) {
            for (j=1; j<genDeg; j++) {
                // EX-OR COMPUTATION
                calcXOR(&info[selectedSymbols[j]*symbLen], &output[ii*symbLen], symbLen, encoder->xorType);
            }
        } else {
            for (j=1; j<genDeg; j++) {
                // EX-OR COMPUTATION
                for (kk=0; kk<symbLen; kk++) {
                    output[ii*symbLen + kk] ^= info[selectedSymbols[j]*symbLen + kk];
                }
            }
        }

        // Increase the number of generated symbols
        encoder->currConfig->genSymbols++;
        encoder->totGenSymbols++;
    }

#ifdef DEBUGencoderPrintHeaders
    printf("\n");
#endif

    // free and initialize the tmp symbol
    free(selectedSymbols);
    selectedSymbols = NULL;

    return IDfirstSymbol;
}

// Return the ID (index) of the first generated symbol
unsigned int LTencodeSymbolFLY(LTencoderFLY *encoder, int symbLen,
                               char *info, unsigned long window_firstSymbolId,
                               unsigned int N, Symbol* encodedSymbols) {

    unsigned int genDeg = 0;
    unsigned int *selectedSymbols = NULL;
    unsigned int ii, jj, j, kk;
    double sel;

    unsigned int w = encoder->currConfig->config->w;
    double *cumulative = encoder->currConfig->cumulative;
    unsigned int IDfirstSymbol = encoder->currConfig->genSymbols;

    // The output must be already allocated
    if ((symbLen < 1)||(info == NULL)||(encodedSymbols == NULL)) {
        print_error("ERROR - encoding process - no encoded information generated.\n");
        if (symbLen < 1)
            print_error("                           symbLen = %d\n.", symbLen);
        if (info == NULL)
            print_error("                           info vector not allocated.\n");
        if (encodedSymbols == NULL)
            print_error("                           encoded symbol list not allocated.\n");
        exit(EXIT_FAILURE);
    }

    // Allocate the vector for the selected symbols
    selectedSymbols = (unsigned int*) chk_calloc(w, sizeof(unsigned int));

    for (ii=0; ii<N; ii++) {

        Symbol* currSymb = &(encodedSymbols[ii]);

        genDeg = degree(cumulative, w, real(&(encoder->currConfig)->randomGenerator));
        if (genDeg > w)
            genDeg = w;

        if (genDeg == w) {
            chooseAllInWindowCore(w, window_firstSymbolId, selectedSymbols, genDeg);

        } else {
            for (jj=0; jj<genDeg; jj++) {
                sel	= real(&(encoder->currConfig)->randomGenerator);
                selectedSymbols[jj]	= rand2windowCore(w, window_firstSymbolId, sel);

                if (verifySelectedSymbol(selectedSymbols, jj) == False) {
                    jj--;		// Doesn't repeat encoding symbols
                }
            }
            // This encoder can sort the selected symbols if the following line is uncommented.
            //sort(selectedSymbols, 0, genDeg-1);	// Sort the chosen encoding symbols' list
        }

        // Allocate memory for the header
        currSymb->header = (unsigned int*) chk_malloc(genDeg, sizeof(unsigned int));

        // Allocate memory for the encoded information
        currSymb->info = (char*) chk_malloc(symbLen, sizeof(char));

        // Saves the degree in the symbol's header
        currSymb->deg = genDeg;

        // Saves the symbol length in the symbol's header
        currSymb->symbLen = symbLen;

        // Assign the id to the new symbol
        currSymb->id = encoder->currConfig->genSymbols+1;

        // Copy the Info of the first input symbol indicated in the header
        memcpy(currSymb->header, selectedSymbols, genDeg*sizeof(unsigned int));

#ifdef DEBUGencoderPrintHeaders
        int deb_index;
        printf("ENCODER - symb[%d]\t: deg=%3d\t -> ", (int) encoder->currConfig->genSymbols, genDeg);
        for (deb_index=0; deb_index<genDeg; deb_index++)
            printf("%3d ", selectedSymbols[deb_index]);
        printf("\n");
#endif

        // Saves the selected source symbols' ID in the current Symbol
        memcpy(currSymb->info, &info[selectedSymbols[0]*symbLen], symbLen*sizeof(char));

        // EX-OR the other symbols of the header
        if (encoder->xorType != NO) {
            for (j=1; j<genDeg; j++) {
                // EX-OR COMPUTATION
                calcXOR(&info[selectedSymbols[j]*symbLen], currSymb->info, symbLen, encoder->xorType);
            }
        } else {
            for (j=1; j<genDeg; j++) {
                // EX-OR COMPUTATION
                for (kk=0; kk<symbLen; kk++) {
                    currSymb->info[kk] ^= info[selectedSymbols[j]*symbLen + kk];
                }
            }
        }

        // Increase the number of generated symbols
        encoder->currConfig->genSymbols++;
        encoder->totGenSymbols++;
    }

#ifdef DEBUGencoderPrintHeaders
    printf("\n");
#endif

    // free and initialize the tmp symbol
    free(selectedSymbols);
    selectedSymbols = NULL;

    return IDfirstSymbol;
}


// Get the random seed used in particular configuration
unsigned long getSeedFromConfig(LTencoderFLY *encoder,
                                unsigned int w, unsigned int s,
                                double lt_delta, double lt_c, double raptor_eps) {

    struct hashtable *h = encoder->DistributionsTable;

    struct value *v = NULL;
    v = searchEntryInLThashTable(h, w, s, lt_delta, lt_c, raptor_eps);

    if (NULL != v) {
        return v->confSeed;
    } else {
        print_error("(libLT_C:LTencoderFLY.c:getSeedFromConfig) ERROR - Configuration not found.\n");
        exit(EXIT_FAILURE);
    }
}

// Get the random seed used in the current configuration
unsigned long getSeedFromCurrConfig(LTencoderFLY *encoder) {

    struct value *v = encoder->currConfig;

    if (NULL != v) {
        return v->confSeed;
    } else {
        print_error("(libLT_C:LTencoderFLY.c:getSeedFromCurrConfig) ERROR - Configuration not found.\n");
        exit(EXIT_FAILURE);
    }
}


// Set the encoder for the LT codes (similar to setEncoderFLY but directly for LT codes)
void setEncoderFLY_LT(LTencoderFLY *encoder,
                      unsigned int w, unsigned int s,
                      double lt_delta, double lt_c) {
    setEncoderFLY(encoder, Luby, w, s, lt_delta, lt_c, 0.0);
}

// Set the encoder for the Raptor codes (similar to setEncoderFLY but directly for Raptor codes)
void setEncoderFLY_Raptor(LTencoderFLY *encoder,
                          unsigned int w, unsigned int s,
                          double raptor_eps) {
    setEncoderFLY(encoder, Shokrollahi, w, s, 0.0, 0.0, raptor_eps);
}

unsigned int getNumEncodedSymbols(LTencoderFLY *encoder) {
    return encoder->totGenSymbols;
}

unsigned int LTencodeSymbolFLY_buffer(LTencoderFLY *encoder, int symbLen,
                                      char* info, unsigned long info_firstSymbolId,
                                      Symbol* encodedSymbols, unsigned int N,
                                      unsigned long window_firstSymbolId) {
    unsigned int genDeg = 0;
    unsigned int *selectedSymbols = NULL;
    unsigned int ii, jj, j, kk;
    double sel;

    unsigned int w = encoder->currConfig->config->w;
    double *cumulative = encoder->currConfig->cumulative;
    unsigned int IDfirstSymbol = encoder->currConfig->genSymbols;

    // The output must be already allocated
    if ((symbLen < 1)||(info == NULL)||(encodedSymbols == NULL)) {
        print_error("ERROR - encoding process - no encoded information generated.\n");
        if (symbLen < 1)
            print_error("                           symbLen = %d\n.", symbLen);
        if (info == NULL)
            print_error("                           info vector not allocated.\n");
        if (encodedSymbols == NULL)
            print_error("                           encoded symbol list not allocated.\n");
        exit(EXIT_FAILURE);
    }

    // Allocate the vector for the selected symbols
    selectedSymbols = (unsigned int*) chk_calloc(w, sizeof(unsigned int));

    for (ii=0; ii<N; ii++) {

        Symbol* currSymb = &(encodedSymbols[ii]);

        genDeg = degree(cumulative, w, real(&(encoder->currConfig)->randomGenerator));
        if (genDeg > w)
            genDeg = w;

        if (genDeg == w) {
            chooseAllInWindowCore(w, window_firstSymbolId, selectedSymbols, genDeg);

        } else {
            for (jj=0; jj<genDeg; jj++) {
                sel	= real(&(encoder->currConfig)->randomGenerator);
                selectedSymbols[jj]	= rand2windowCore(w, window_firstSymbolId, sel);

                if (verifySelectedSymbol(selectedSymbols, jj) == False) {
                    jj--;		// Doesn't repeat encoding symbols
                }
            }
            // This encoder can sort the selected symbols if the following line is uncommented.
            //sort(selectedSymbols, 0, genDeg-1);	// Sort the chosen encoding symbols' list
        }

        // Allocate memory for the header
        currSymb->header = (unsigned int*) chk_malloc(genDeg, sizeof(unsigned int));

        // Allocate memory for the encoded information
        currSymb->info = (char*) chk_malloc(symbLen, sizeof(char));

        // Saves the degree in the symbol's header
        currSymb->deg = genDeg;

        // Saves the symbol length in the symbol's header
        currSymb->symbLen = symbLen;

        // Assign the id to the new symbol
        currSymb->id = encoder->currConfig->genSymbols+1;

        // Copy the Info of the first input symbol indicated in the header
        memcpy(currSymb->header, selectedSymbols, genDeg*sizeof(unsigned int));

#ifdef DEBUGencoderPrintHeaders
        int deb_index;
        printf("ENCODER - symb[%d]\t: deg=%3d\t -> ", (int) encoder->currConfig->genSymbols, genDeg);
        for (deb_index=0; deb_index<genDeg; deb_index++)
            printf("%3d ", selectedSymbols[deb_index]);
        printf("\n");
#endif

        // Saves the selected source symbols' ID in the current Symbol
        memcpy(currSymb->info, &info[(selectedSymbols[0]-info_firstSymbolId)*symbLen], symbLen*sizeof(char));

        // EX-OR the other symbols of the header
        if (encoder->xorType != NO) {
            for (j=1; j<genDeg; j++) {
                // EX-OR COMPUTATION
                calcXOR(&info[(selectedSymbols[j]-info_firstSymbolId)*symbLen], currSymb->info, symbLen, encoder->xorType);
            }
        } else {
            for (j=1; j<genDeg; j++) {
                // EX-OR COMPUTATION
                for (kk=0; kk<symbLen; kk++) {
                    currSymb->info[kk] ^= info[(selectedSymbols[j]-info_firstSymbolId)*symbLen + kk];
                }
            }
        }

        // Increase the number of generated symbols
        encoder->currConfig->genSymbols++;
        encoder->totGenSymbols++;
    }

#ifdef DEBUGencoderPrintHeaders
    printf("\n");
#endif

    // free and initialize the tmp symbol
    free(selectedSymbols);
    selectedSymbols = NULL;

    return IDfirstSymbol;
}

void LTencodeFLY_toFile(LTencoderFLY *encoder,
                        unsigned int N,
                        unsigned int l,
                        char *sourceInfo,
                        unsigned long info_firstSymbolId,
                        char *outfile ) {
    // Allocate the space to store the encoded symbols
    Symbol* encodedSymbols = (Symbol*) chk_calloc(N, sizeof(Symbol));

    // Encode the source symbols
    LTencodeSymbolFLY(encoder, l, sourceInfo, info_firstSymbolId, N, encodedSymbols);

    // Serialize symbols
    char* serializedBuff = NULL;
    unsigned long serializedBuffLen = 0;
    serializedBuffLen = serializeSymbolList_explicit_append(encodedSymbols, N, &serializedBuff);

    // Delete the symbol list
    unsigned int i;
    for(i = 0; i < N; i++ ) {
        Symbol* currSymb = &(encodedSymbols[i]);
        if (currSymb->info != NULL)
            free(currSymb->info);
        if (currSymb->header != NULL)
            free(currSymb->header);
    }
    free(encodedSymbols);

    if (writeFile(outfile, serializedBuff, serializedBuffLen) != serializedBuffLen) {
        fprintf(stderr, "Encoded Information NOT written correctly.\n");
        exit(EXIT_FAILURE);
    }

    // Free buffer
    free(serializedBuff);
}

// ============================================
// 			Private functions implementation
// ============================================

void createDegreeDistribution(LTencoderFLY *encoder, double *cumulative,
                              unsigned int w, unsigned int s,
                              double lt_delta, double lt_c, double raptor_eps) {

    //NOTE: cumulative must be previously allocated
    if (cumulative == NULL) {
        print_error("ERROR: The cumulative vector has to be previously allocated.\n");
        exit(EXIT_FAILURE);
    }
    switch (encoder->code) {
    case Luby:
        RSDcumulative(cumulative, w, s, lt_delta, lt_c);
        break;
    case Shokrollahi:
        SHOKcumulative(cumulative, w, s, raptor_eps);
        break;
    default:
        print_error("ERROR: currently only LT and Raptor codes are implemented.\n");
        exit(EXIT_FAILURE);
        break;
    }
    return;
}

