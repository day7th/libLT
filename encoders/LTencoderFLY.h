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
// File       : LTencoderFLY.h
// Author     : Pasquale Cataldi
// Company    :
// Last update: 2009/05/08
// Platform   : Ansi C
//*****************************************************************************
// Description: This file contains the LT encoder declarations
//			This encoder differs from LTencoder because in this one
//			you can generate encoded symbols on-the-fly.
//*****************************************************************************
// Revisions  :
// Date        Version  Author  Description
// 2010/12/08  0.2      PC      Added new functions to set the encoder for LT
//                              and Raptor codes separately. Moved one function
//                              the .c file to hide it from the outside.
// 2010/12/30  0.3      PC      Added function that allows to pass to the decoder
//                              the encoded symbols with their headers, without buckets.
//*****************************************************************************

#ifndef ENCODERFLY_H_
#define ENCODERFLY_H_

#define MINHASHTABLESIZE 16

#define noDEBUGencoderPrintHeaders
#define noDEBUGencoderPrintBucketRandomSeed

#include "../utils/LTstruct.h"
#include "../utils/LThash.h"
#include "../xor/xor.h"

typedef struct {
    XorType xorType;
    DegDistr code;
    unsigned long refRandomSeed; // This is the first randomSeed of the encoder
    unsigned int totGenSymbols;  // Counter of total generated symbols
    struct hashtable *DistributionsTable;
    struct value *currConfig;    // current configuration
} LTencoderFLY;

// Allocate the encoder data structure
LTencoderFLY* mallocEncoderFLY(unsigned long seed);

void freeEncoderFLY(LTencoderFLY *encoder);

// Every time that you change the parameters with setEncoderFLY,
// a random seed is created the random is reset
//     with increasing the randomSeed and numConfigChanges.
void setEncoderFLY(LTencoderFLY *encoder, DegDistr code,
                   unsigned int w, unsigned int s,
                   double lt_delta, double lt_c, double raptor_eps);

// Set the encoder for the LT codes (as setEncoderFLY but specific to LT codes)
void setEncoderFLY_LT(LTencoderFLY *encoder,
                      unsigned int w, unsigned int s,
                      double lt_delta, double lt_c);

// Set the encoder for the Raptor codes (as setEncoderFLY but specific to Raptor codes)
void setEncoderFLY_Raptor(LTencoderFLY *encoder,
                          unsigned int w, unsigned int s,
                          double raptor_eps);

// Destroy the LThashTable, reinit values
void resetEncoderFLY(LTencoderFLY *encoder, unsigned long newSeed);

// Erase totGenSymbols, set a new randomSeed and update all the existing configurations
void reseedEncoderFLY(LTencoderFLY *encoder, unsigned long newSeed);

// Return the ID (index) of the first generated symbol
unsigned int LTencodeInfoFLY(LTencoderFLY *encoder, int symbLen,
                             char *info, unsigned long window_firstSymbolId,
                             unsigned int N, char *output);

// Get the random seed used in particular configuration
unsigned long getSeedFromConfig(LTencoderFLY *encoder,
                                unsigned int w, unsigned int s,
                                double lt_delta, double lt_c, double raptor_eps);

// Get the random seed used in the current configuration
unsigned long getSeedFromCurrConfig(LTencoderFLY *encoder);


// Return the ID (index) of the first generated symbol
unsigned int LTencodeSymbolFLY(LTencoderFLY *encoder, int symbLen,
                               char *info, unsigned long window_firstSymbolId,
                               unsigned int N, Symbol* encodedSymbols);

// Return the number of encoded symbols that have been generated with the current configuration.
unsigned int getNumEncodedSymbols(LTencoderFLY *encoder);

/* Generate from a window of input data N encoded symbols.
 *
 * @param[in] *encoder Pointer to the LTencoderFLY structure
 * @param[in] symbLen Number of bytes of the symbol's information.
 * @param[in] *info Buffer containing the input information data.
 * @param[in] info_firstSymbolId Id of the first input symbol of the info buffer.
 * @param[out] *encodedSymbols Pointer to the array of encoded symbols to be generated (already allocated).
 * @param[in] N Number of encoded symbols to be generated.
 * @param[in] window_firstSymbolId ID of the first symbol of the encoding window.
 * @return The ID (index) of the first generated symbol
 */
unsigned int LTencodeSymbolFLY_buffer(LTencoderFLY *encoder, int symbLen,
                                      char* info, unsigned long info_firstSymbolId,
                                      Symbol* encodedSymbols, unsigned int N,
                                      unsigned long window_firstSymbolId);

void LTencodeFLY_toFile(LTencoderFLY *encoder,
                        unsigned int N,
                        unsigned int l,
                        char *sourceInfo,
                        unsigned long info_firstSymbolId,
                        char *outfile);

#endif /*ENCODERFLY_H_*/
