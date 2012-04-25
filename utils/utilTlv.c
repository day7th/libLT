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
 * utilTlv.c
 *
 *  Created on: Jan 10, 2011
 *      Author: cataldi
 */

#include "utilTlv.h"
#include "utilErr.h"
#include "utilMem.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


////////////////////////////////////////////
//               SERIALIZE functions
////////////////////////////////////////////

unsigned int serializeSymbol(Symbol* encodedSymbol, char version, char** pTlvData) {
    unsigned int totLen = 0;
    unsigned int valueLen = 0;
    unsigned int i = 0;
    char* tlvData = NULL;

    Tbool explicit = (Tbool)(version & 0x01);

    if (encodedSymbol->info == NULL) {
        print_error("serializeSymbol: The symbol does not contain encoded information!");
        return 0;
    }
    if (encodedSymbol->deg == 0) {
        print_error("serializeSymbol: The symbol does not contain degree information!");
        return 0;
    }
    if (encodedSymbol->header == NULL) {
        print_error("serializeSymbol: The symbol does not contain information about the IDs of the generator symbols!");
        return 0;
    }

    // The length does not count the first two bytes, i.e. TLV_SYMBOL and the value of the total length of the serialized symbol.
    valueLen = 1 +                                           // TLV_DATA__INFO
               4 +                                           // TLV_DATA__INFO length
               encodedSymbol->symbLen;                       // number of bytes of information

    if (explicit == True) {
        valueLen += 1 +                                      // TLV_DATA__ENCODING_SYMBS
                    4 +                                      // TLV_DATA__ENCODING_SYMBS length
                    encodedSymbol->deg*sizeof(unsigned int); // number of bytes representing all the generators
    }

    totLen = 1 + // TLV_DATA__SYMBOL
             4 + // TLV_DATA__SYMBOL length
             valueLen;

    // Allocate the char array
    tlvData = (char*) chk_calloc(totLen, sizeof(char));

    // Write the data
    tlvData[i] = TLV_DATA__SYMBOL;
    i += sizeof(char);

    memcpy((char*)(tlvData+i), (unsigned int*) &(valueLen), sizeof(unsigned int));
    i += sizeof(unsigned int);
    tlvData[i] = TLV_DATA__INFO;
    i += sizeof(char);
    memcpy((char*)(tlvData+i), (unsigned int*) &(encodedSymbol->symbLen), sizeof(unsigned int));
    i += sizeof(unsigned int);
    memcpy((char*)(tlvData+i), encodedSymbol->info, encodedSymbol->symbLen*sizeof(char));
    i += encodedSymbol->symbLen*sizeof(char);

    if (explicit == True) {
        tlvData[i] = TLV_DATA__ENCODING_SYMBS;
        i += sizeof(char);
        memcpy((char*)(tlvData+i), (unsigned int*) &(encodedSymbol->deg), sizeof(unsigned int));
        i += sizeof(unsigned int);
        memcpy((char*)(tlvData+i), encodedSymbol->header, encodedSymbol->deg*sizeof(unsigned int));
        i += encodedSymbol->deg*sizeof(unsigned int);
    }

    // Make pTlvData point to the beginning of the generated data.
    *pTlvData = NULL;
    *pTlvData = tlvData;

    return totLen;
}


unsigned long long serializeSymbolList_explicit(Symbol* encodedSymbols, unsigned int N, char** pTlvData) {
    char* tlvData = NULL;
    char version = 0x01; // explicit

    short int valueHeaderLen = 0;
    unsigned int totHeaderLen = 0;
    unsigned int i = 0;

    if (encodedSymbols == NULL) {
        print_error("symbolList2TLV_explicit: The symbol list is empty!");
        return 0;
    }
    if (N == 0) {
        print_error("symbolList2TLV_explicit: The number of symbols to be converted to TLV format has to specified!");
        return 0;
    }

    // Compute header length
    valueHeaderLen = sizeof(char) +              // TLV_HEADER__VERSION
                     sizeof(char) +              // TLV_HEADER__VERSION value
                     sizeof(char) +              // TLV_HEADER__NUM_SYMBS
                     sizeof(unsigned int) +      // TLV_HEADER__NUM_SYMBS value
                     sizeof(char) +              // TLV_HEADER__DATA_LENGTH
                     sizeof(unsigned long long); // TLV_HEADER__DATA_LENGTH value

    totHeaderLen =   sizeof(char) +              // TLV_HEADER__LENGTH
                     sizeof(char) +              // TLV_HEADER__LENGTH value
                     valueHeaderLen;

    // Allocate buffer for header
    char* header_buff = (char*) chk_calloc(totHeaderLen, sizeof(char));

    // Create header
    header_buff[i] = TLV_HEADER__LENGTH;
    i += sizeof(char);
    header_buff[i] = (char) valueHeaderLen;
    i += sizeof(char);
    header_buff[i] = TLV_HEADER__VERSION;
    i += sizeof(char);
    header_buff[i] = version; // explicit
    i += sizeof(char);
    header_buff[i] = TLV_HEADER__NUM_SYMBS;
    i += sizeof(char);
    memcpy((char*)(header_buff+i), (unsigned int*) &N, sizeof(unsigned int));
    i += sizeof(unsigned int);
    header_buff[i] = TLV_HEADER__DATA_LENGTH;
    i += sizeof(char);
    // We still have to add the total length of the data!

    // Create the data_buff
    char* data_buff = NULL;
    unsigned long long data_buff_len = 0;
    unsigned int i_symb = 0;
    for (i_symb = 0; i_symb < N; i_symb++) {
        char *symb_buff = NULL;
        unsigned int tlvSymbLen = 0;
        // Serialize the current symbol
        tlvSymbLen = serializeSymbol(&encodedSymbols[i_symb], version, &symb_buff);
        // Reallocate the data_buff
        data_buff = (char*) chk_realloc(data_buff, data_buff_len+tlvSymbLen, sizeof(char));
        // Copy symb_buff at the end of the data_buff
        memcpy((char*)(data_buff + data_buff_len), symb_buff, tlvSymbLen*sizeof(char));
        // Update the length variable of the data_buff
        data_buff_len += tlvSymbLen;
        // free the symb_buff variable
        free(symb_buff);
    }

    // Now we have the value of the data_buff_len and we can update the field in the header
    memcpy((char*)(header_buff+i), &data_buff_len, sizeof(unsigned long long));
    i += sizeof(unsigned long long);

    // Create the tlv stream
    tlvData = (char*) chk_calloc(totHeaderLen+data_buff_len, sizeof(char));
    // Copy the header
    memcpy(tlvData, header_buff, totHeaderLen*sizeof(char));
    // Copy the serialized data
    memcpy((char*)(tlvData+totHeaderLen), data_buff, data_buff_len*sizeof(char));

    // Free temporary buffers
    free(header_buff);
    free(data_buff);

    // Make pTlvData point to the beginning of the generated data.
    *pTlvData = tlvData;

    return (unsigned long long) totHeaderLen+data_buff_len;
}


unsigned long serializeSymbolList_explicit_append(Symbol* encodedSymbols, unsigned int N, char** pTlvData) {

    char version = 0x01; // explicit

    if (encodedSymbols == NULL) {
        print_error("serializeSymbolList_explicit_append: The symbol list is empty!");
        return 0;
    }
    if (N == 0) {
        print_error("serializeSymbolList_explicit_append: The number of symbols to be converted to TLV format has to specified!");
        return 0;
    }
    // If the TLV-like buffer is empty, call serializeSymbolList_explicit()
    if (*pTlvData == NULL) {
        return serializeSymbolList_explicit(encodedSymbols, N, pTlvData);
    }

    char* tlvData = NULL;

    // ====== Read Header ======
    unsigned long long pos = 0;

    unsigned short headerLen;
    char bufferVersion;
    unsigned long long oldDataLen;
    unsigned int oldN;

    readSerializedBasicHeader(tlvData, &headerLen, &bufferVersion, &oldN, &oldDataLen, &pos);
    if (headerLen > pos) {
        // TODO
        readSerializedHeaderExtensions(tlvData, &pos);
    }

    // Check version
    if (bufferVersion != version) {
        print_error("serializeSymbolList_explicit_append: The TLV-like buffer is not explicit as required by this function.\n"
                    "New symbols not appended.\n");
        return (2+headerLen+oldDataLen);
    }

    // ====== Create the data_buff ======
    char* data_buff = NULL;
    unsigned long data_buff_len = 0;
    unsigned int i_symb = 0;
    for (i_symb = 0; i_symb < N; i_symb++) {
        char *symb_buff = NULL;
        // Serialize the current symbol
        unsigned int tlvSymbLen = serializeSymbol(&encodedSymbols[i_symb], version, &symb_buff);
        // Reallocate the data_buff
        data_buff = (char*) chk_realloc(data_buff, data_buff_len+tlvSymbLen, sizeof(char));
        // Copy symb_buff at the end of the data_buff
        memcpy((char*)(data_buff+data_buff_len), symb_buff, tlvSymbLen*sizeof(char));
        // Update the length variable of the data_buff
        data_buff_len += tlvSymbLen;
        // free the data_buff variable
        free(data_buff);
    }

    // Overwrite the total number of symbols that will be present in the new stream (this filed starts at the 6th byte of the header)
    unsigned int newN = oldN + N;
    memcpy((char*)(tlvData+5), &newN, sizeof(unsigned int));

    // Now we have the value of the data_buff_len and we can update the field in the header (that starts at the 18th byte of the header)
    unsigned long newDataLen = oldDataLen + data_buff_len;
    memcpy((char*)(tlvData+17), &newDataLen, sizeof(unsigned long));

    // Reallocate the TLV-like buffer
    tlvData = (char*) chk_realloc(tlvData, (2+headerLen)+newDataLen, sizeof(char));

    // copy the data_buff with the serialized new symbols
    memcpy((char*)(tlvData+(2+headerLen)+oldDataLen), data_buff, newDataLen*sizeof(char));

    // Make pTlvData point to the beginning of the generated data.
    *pTlvData = tlvData;

    return (2+headerLen)+newDataLen;
}


unsigned long serializeSymbolList_implicit(Symbol* encodedSymbols, unsigned int N, char** pTlvData, ...) {

    print_error("Sorry, serializeSymbolList_implicit() not implemented yet.\n");

    // TODO:

    return 0;
}


unsigned long serializeSymbolList_implicit_append(Symbol* encodedSymbols, unsigned int N, char** pTlvData) {

    print_error("Sorry, serializeSymbolList_implicit_append() not implemented yet.\n");

    // TODO:

    return 0;
}




////////////////////////////////////////////
//               DESERIALIZE functions
////////////////////////////////////////////

unsigned long deserializeSymbol(char* tlvData, unsigned long long startPos, Symbol* encodedSymbol) {

    Tbool isID = False;
    Tbool isINFO = False;
    Tbool isENCSYMBS = False;

    unsigned long long pos = startPos;

    char checkType = tlvData[startPos];
    if (checkType != TLV_DATA__SYMBOL) {
        print_error("deserializeSymbol: Impossible to read symbol at position %lu", pos);
        return 0;
    }
    pos += sizeof(char);

    // Read the length of the Symbol
    unsigned int length;
    memcpy(&length, (char*)(tlvData+pos), sizeof(unsigned int));
    pos += sizeof(unsigned int);

    while (pos-startPos < length) {

        char type = tlvData[pos];
        pos += sizeof(char);

        switch (type) {
        case TLV_DATA__ID: {
            // Read the id
            memcpy(&encodedSymbol->id, (char*)(tlvData+pos), sizeof(unsigned int));
            pos += sizeof(unsigned int);
            // Set the corresponding flag to true
            isID = True;
            break;
        }
        case TLV_DATA__INFO: {
            // Read symblen
            memcpy(&encodedSymbol->symbLen, (char*)(tlvData+pos), sizeof(int));
            pos += sizeof(int);
            // Read info
            encodedSymbol->info = (char*) chk_malloc(encodedSymbol->symbLen, sizeof(char));
            memcpy(encodedSymbol->info, (char*)(tlvData+pos), encodedSymbol->symbLen*sizeof(char));
            pos += encodedSymbol->symbLen*sizeof(char);
            // Set the corresponding flag to true
            isINFO = True;
            break;
        }
        case TLV_DATA__ENCODING_SYMBS: {
            // Read degree
            memcpy(&encodedSymbol->deg, (char*)(tlvData+pos), sizeof(unsigned int));
            pos += sizeof(unsigned int);
            // Read the encoding symbol
            encodedSymbol->header = (unsigned int*) chk_malloc(encodedSymbol->deg, sizeof(unsigned int));
            memcpy(encodedSymbol->header, (char*)(tlvData+pos), encodedSymbol->deg*sizeof(unsigned int));
            pos += encodedSymbol->deg*sizeof(unsigned int);
            // Set the corresponding flag to true
            isENCSYMBS = True;
            break;
        }
        default: {
            print_error("deserializeSymbol: Not recognized symbol type!");
            free(encodedSymbol);
            return 0;
        }
        } // end switch
    } // end while

    // Check that the symbol is compliant to either implicit or explicit
    if (!((isENCSYMBS && isINFO)||(isID && isINFO))) {
        print_error("deserializeSymbol: symbol is not compliant to either implicit nor explicit!");
        free(encodedSymbol);
        return 0;
    }

    return pos;
}

void deserializeSymbolList(char* tlvData, Symbol** pEncodedSymbolList, unsigned int* N) {

    if (tlvData == NULL) {
        print_error("Sorry, impossible to de-serialize the data buffer containing the symbol list because the buffer is empty!\n");
        return 0;
    }

    Symbol* encodedSymbols = NULL;
    unsigned long long pos = 0;

    // ====== Read Header ======

    unsigned short headerLen;
    char version;
    unsigned long long dataLen;

    readSerializedBasicHeader(tlvData, &headerLen, &version, N, &dataLen, &pos);
    if (headerLen +2 > pos) {
        // TODO
        readSerializedHeaderExtensions(tlvData, &pos);
    }

    // ====== Read symbols ======

    // Allocate an array of Symbol structures
    encodedSymbols = (Symbol*) chk_calloc(*N, sizeof(Symbol));

    // De-serialize one symbol at the time
    unsigned int numSymb = 0;
    for (numSymb = 0; numSymb < *N; numSymb++) {

        if (pos < (2+headerLen)+dataLen)
            pos = deserializeSymbol(tlvData, pos, &(encodedSymbols[numSymb]));
        else {
            print_error("tlv2symbolList: It seems that the TLV-like data buffer in not coherent. "
                        "The current position (%lu) has exceeded the expected maximum value (%lu)."
                        "\nNumber of de-serialized symbols: %d", pos, (2+headerLen)+dataLen, numSymb);
            break;
        }
    }

    // Make pEncodedSymbolList to point to the beginning of the generated data.
    *pEncodedSymbolList = NULL;
    *pEncodedSymbolList = encodedSymbols;

    return;
}





////////////////////////////////////////////
//               COMMON
////////////////////////////////////////////

void readSerializedBasicHeader(char* tlvData, unsigned short* headerLen, char* version, unsigned int* N, unsigned long long* dataLen, unsigned long long *pos) {

    if (tlvData == NULL) {
        print_error("readSerializedBasicHeader: TLV-like data buffer not provided.\n");
        return;
    }

    char type;

    // == Read type: TLV_HEADER__LENGTH
    type = tlvData[*pos];
    if (type != TLV_HEADER__LENGTH) {
        print_error("readSerializedBasicHeader: Expected TLV_HEADER__LENGTH type at this position of the TLV-like data buffer.\n", pos);
        return;
    }
    *pos += sizeof(char);
    // Read length: header
    *headerLen = (unsigned short) tlvData[*pos];
    *pos += sizeof(char);

    // == Read type: TLV_HEADER__VERSION
    type = tlvData[*pos];
    if (type != TLV_HEADER__VERSION) {
        print_error("readSerializedBasicHeader: Expected TLV_HEADER__VERSION type at this position (%lu-th byte) of the TLV-like data buffer.\n", pos);
        return;
    }
    *pos += sizeof(char);
    // Read value: TLV-like data version
    *version = tlvData[*pos];
    *pos += sizeof(char);

    // == Read type: TLV_HEADER__NUM_SYMBS
    type = tlvData[*pos];
    if (type != TLV_HEADER__NUM_SYMBS) {
        print_error("readSerializedBasicHeader: Expected TLV_HEADER__NUM_SYMBS type at this position (%lu-th byte) of the TLV-like data buffer.\n", pos);
        return;
    }
    *pos += sizeof(char);
    // Read value: Number of encoded symbols in the data buffer
    memcpy(N, (char*)(tlvData+*pos), sizeof(unsigned int));
    *pos += sizeof(unsigned int);

    // == Read type: TLV_HEADER__DATA_LENGTH
    type = tlvData[*pos];
    if (type != TLV_HEADER__DATA_LENGTH) {
        print_error("readSerializedBasicHeader: Expected TLV_HEADER__DATA_LENGTH type at this position (%lu-th byte) of the TLV-like data buffer.\n", pos);
        return;
    }
    *pos += sizeof(char);
    // Read value: Number of bytes contained the serialized symbols
    memcpy(dataLen, (char*)(tlvData+*pos), sizeof(unsigned long long));
    *pos += sizeof(unsigned long long);

}

void readSerializedHeaderExtensions(char* tlvData, unsigned long long *pos, ...) {

    print_error("Sorry, readSerializedHeaderExtensions() not implemented yet.\n");

    // TODO:

    return;
}
