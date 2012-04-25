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
 * utilTlv.h
 *
 *  Created on: Jan 10, 2011
 *      Author: cataldi
 */

#ifndef TLV_H_
#define TLV_H_


// ********************************************************************************************************************
// ********************************************************************************************************************
//                     TLV-like data format for encoded symbols (de)serialization:
// ********************************************************************************************************************
// ********************************************************************************************************************
//
//|===================================================================================================================|
//|           Header format:                                                                                          |
//|-------------------------------------------------------------------------------------------------------------------|
//|                                                                                                                   |
//| 1 byte    TLV_HEADER__LENGTH       (0x00)                                                                         |
//| 1 byte    length: Length of the Header part of the tlv stream from the following byte to the end of the header.   |
//| 1 byte    TLV_HEADER__VERSION      (0x01)                                                                         |
//| 1 byte    value: If first bit == 0 then EXPLICIT. Else IMPLICIT.                                                  |
//|                  The other bits can be used to create other versions of the tlv stream                            |
//|                  (however, the implicit and explicit rules have to be respected)                                  |
//| 1 byte    TLV_HEADER__NUM_SYMBS    (0x02)                                                                         |
//| 4 bytes   value: Number of encoded symbols in the tlv stream.                                                     |
//| 1 byte    TLV_HEADER__DATA_LENGTH  (0x03)                                                                         |
//| 8 bytes   value: Length in bytes of the data part of the stream.                                                  |
//|                                                                                                                   |
//| // OPTIONAL: Only for the IMPLICIT versions. All the values are assumed to be constant for the all stream!        |
//| 1 byte    TLV_HEADER__SEED         (0x04)                                                                         |
//| 8 bytes   value: Seed of the random generation used to generate the encoded symbols                               |
//| 1 byte    TLV_HEADER__W            (0x05)                                                                         |
//| 8 bytes   value: Number of source symbols that compose an encoding window                                         |
//| 1 byte    TLV_HEADER__DEG_DISTRIB  (0x06)                                                                         |
//| 8 bytes   value: Flag that indicates the degree distribution used (0x00 = RSD, 0x01 = Shokrollahi,                |
//                   the others have not been defined yet).                                                           |
//| 1 byte    TLV_HEADER__RSD_C        (0x07)                                                                         |
//| 8 bytes   value: C parameter of the RSD distribution.                                                             |
//| 1 byte    TLV_HEADER__RSD_DELTA    (0x08)                                                                         |
//| 8 bytes   value: Delta parameter of the RSD distribution.                                                         |
//| 1 byte    TLV_HEADER__SHOK_EPS     (0x09)                                                                         |
//| 8 bytes   value: Epsilon parameter of the Shokrollahi distribution.                                               |                              |
//|                                                                                                                   |
//| // OPTIONAL EXTRA (for Sliding window mode)                                                                       |
//| 1 byte    TLV_HEADER__S            (0x0A)                                                                         |
//| 8 bytes   value: Number of source symbols of shifts between encoding windows.                                     |
//| 1 byte    TLV_HEADER__NW           (0x0B)                                                                         |
//| 4 bytes   value: Number of encoded symbols per encoding window before shifting.                                   |
//|                                                                                                                   |
//|===================================================================================================================|
//|           Data format:                                                                                            |
//|-------------------------------------------------------------------------------------------------------------------|
//|                                                                                                                   |
//| 1 byte    TLV_DATA__SYMBOL         (0x00)                                                                         |
//| 4 bytes   length: Number of bytes to the end of the current symbol.                                               |
//| 1 byte    TLV_DATA__INFO           (0x01)                                                                         |
//| 4 bytes   length: Number of bytes composing the encoded information.                                              |
//| * bytes   value: encoded information.                                                                             |
//|                                                                                                                   |
//| // IMPLICIT versions:                                                                                             |
//| 1 byte    TLV_DATA__ID             (0x02)                                                                         |
//| 4 bytes   value: ID of the encoded symbol, i.e. the number of encoded symbols generated before this one.          |
//|                                                                                                                   |
//| // EXPLICIT versions:                                                                                             |
//| 1 byte    TLV_DATA__ENCODING_SYMBS (0x03)                                                                         |
//| 4 bytes   length: Number of source symbols that were selected to generate this one.                               |
//| * bytes   value: IDs of the selected source symbols.                                                              |
//|                                                                                                                   |
//|===================================================================================================================|






#include "LTstruct.h"

/******************************************
 * TLV TYPES definition
 ******************************************/

// ======= HEADER =====
// Common to both implicit and explicit formats
#define TLV_HEADER__LENGTH       0x00 // Value: Length of the Header part of the tlv stream.
#define TLV_HEADER__VERSION      0x01 // Value: If first bit == 0 then EXPLICIT. Else IMPLICIT.
#define TLV_HEADER__NUM_SYMBS    0x02 // Value: Number of encoded symbols in the tlv stream.
#define TLV_HEADER__DATA_LENGTH  0x03 // Value: Length in bytes of the data part of the stream.
// Only for the implicit formats
#define TLV_HEADER__SEED         0x04
#define TLV_HEADER__W            0x05
#define TLV_HEADER__DEG_DISTRIB  0x06
#define TLV_HEADER__RSD_C        0x07
#define TLV_HEADER__RSD_DELTA    0x08
#define TLV_HEADER__SHOK_EPS     0x09
// For sliding windows only
#define TLV_HEADER__S            0x0A
#define TLV_HEADER__NW           0x0B
// ======= DATA =====
#define TLV_DATA__SYMBOL         0x00
#define TLV_DATA__INFO           0x01
#define TLV_DATA__ID             0x02
#define TLV_DATA__ENCODING_SYMBS 0x03

/******************************************
 * TLV Function definitions
 ******************************************/

////////////////////////////////////////////
//               SERIALIZE
////////////////////////////////////////////


/* Generate from an encoded symbols, a TLV-like data buffer.
 *
 * @param[in] encodedSymbol Encoded symbol pointer.
 * @param[in] version Version of the TLV stream to create
 *            (for a symbol it is only important if the serialization will be implicit or explicit).
 * @param[out] **ptlvData Pointer to the data encoded in TLV-like format.
 * @return Length in bytes of the generated data.
 */
unsigned int serializeSymbol(Symbol* encodedSymbol, char version, char** pTlvData);

/* Generate from a list of encoded symbols, the TLV-like buffer (header+data). The symbols are all explicit.
 *
 * @param[in] *encodedSymbols Encoded symbol list pointer.
 * @param[in] N Number of encoded symbols.
 * @param[out] **pTlvData Pointer to the data encoded in TLV-like format.
 * @return Length in bytes of the generated data.
 */
unsigned long long serializeSymbolList_explicit(Symbol* encodedSymbols, unsigned int N, char** pTlvData);

/* Append to an already TLV-like buffer, a list of encoded symbols. The symbols are all explicit.
 *
 * @param[in] *encodedSymbols Encoded symbol list pointer.
 * @param[in] N Number of encoded symbols.
 * @param[out] **pTlvData Pointer to the data encoded in TLV-like format. If NULL, the serializeSymbolList_explicit() function will be returned.
 * @return Length in bytes of the generated data.
 */
unsigned long serializeSymbolList_explicit_append(Symbol* encodedSymbols, unsigned int N, char** pTlvData);

/* Generate from a list of encoded symbols, the TLV-like buffer. The symbols are all implicit.
 *
 * @param[in] *encodedSymbols Encoded symbol list pointer.
 * @param[in] N Number of encoded symbols.
 * @param[out] **pTlvData Pointer to the data encoded in TLV format.
 * @return Length in bytes of the generated data.
 */
unsigned long serializeSymbolList_implicit(Symbol* encodedSymbols, unsigned int N, char** pTlvData, ...);

/* Append to an already TLV-like buffer, a list of encoded symbols. The symbols are all implicit.
 *
 * @param[in] *encodedSymbols Encoded symbol list pointer.
 * @param[in] N Number of encoded symbols.
 * @param[out] **pTlvData Pointer to the data encoded in TLV-like format. If NULL, the serializeSymbolList_explicit() function will be returned.
 * @return Length in bytes of the generated data.
 */
unsigned long serializeSymbolList_implicit_append(Symbol* encodedSymbols, unsigned int N, char** pTlvData);

////////////////////////////////////////////
//               DESERIALIZE
////////////////////////////////////////////

/* Read from the tlv data buffer the information about one symbol and return the associated Symbol structure.
 *
 * @param[in] *tlvData Data encoded in TLV-like format.
 * @param[in] startPos Position (in number of bytes) of the symbol to be read with respect to the beginning of the tlv data buffer.
 * @param[in] *encodedSymbols Pointer to the encoded symbol.
 * @return Position in the buffer after the end of the deserialized symbol (0 means error!).
 *
 * @return Position in number of bites read from the buffer at the end of the read symbol.
 */
unsigned long deserializeSymbol(char* tlvData, unsigned long long startPos, Symbol* encodedSymbol);

/* Read from the tlv data buffer the information about a list of symbols and return the associated Symbol list structure.
 *
 * @param[in] *tlvData Data encoded in TLV-like format.
 * @param[in] **pEncodedSymbolList Pointer to the encoded symbol list.
 * @param[in] *N Pointer to the number of symbols read.
 * @return Position in the buffer after the end of the deserialized symbol (0 means error!).
 *
 * @return Length in bytes of the generated data.
 */
void deserializeSymbolList(char* tlvData, Symbol** pEncodedSymbolList, unsigned int* N);



////////////////////////////////////////////
//               COMMON
////////////////////////////////////////////

void readSerializedBasicHeader(char* tlvData, unsigned short* headerLen, char* version, unsigned int* N, unsigned long long* dataLen, unsigned long long *pos);

void readSerializedHeaderExtensions(char* tlvData, unsigned long long *pos, ...);


#endif /* TLV_H_ */
