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
// File       : sharedMethods.h
// Author     : Pasquale Cataldi, Andrea Tomatis
// Company    :
// Last update: 2010/12/30
// Platform   : Ansi C
//*****************************************************************************
// Description: This file contains the declarations of the functions
//				in common between encoder and decoder
//*****************************************************************************
// Revisions  :
// Date        Version  Author  Description
// 2008/07/17  0.1      PC,AT   First release
// 2008/07/24  0.2      PC,AT   Add getTime: number of clock
// 2008/07/24  0.3      PC      Add " #include <math.h> "
// 2008/10/25  0.4      PC      The degree distributions compute the theorical avg degree
//								The create_headers_p2p returns the avg degree for the
//									generated symbols
// 2008/11/20  0.5      PC      Modified the name of "create_headers_p2p" to "create_headers"
//                              Added the function "create_headers_nw"
// 2009/02/19  0.5      PC      Modified the function "create_headers_nw" so that
//				                you can specify also Ns
// 2010/12/30  0.6      PC      Moved the (de-)interleaving functions to utilInterleavers.h.
// 2010/12/30  0.7      PC      Moved the computeStdDevOnTheFly function to utilStats.h.
//*****************************************************************************

#ifndef SHAREDMETHODS_H_
#define SHAREDMETHODS_H_

#include "LTstruct.h"
#include "../list/node.h"
#include "../xor/xor.h"

// Compute the Degree Distributions.
double SHOKcumulative(double *cumulative, unsigned int w, unsigned int s, double raptor_eps);
double RSDcumulative(double *cumulative, unsigned int w, unsigned int s, double delta, double c);

// Degree Distributions for LTencoder and LTdecoder. Return the theorical average encoded symbol degree
double rubust_soliton_distribution(Mgenerator *matrix);
double shokrollahi_distribution(Mgenerator *matrix);

// Both functions return the average generated degree of the encoded symbols
// Compute the ESs, moving the window after each generation
double create_headers(Mgenerator *matrix);
// Compute the ESs, generating nw symbols per each window. The remaining N-nw*Ns will
// be generated as the function above
double create_headers_nw(Mgenerator *matrix, unsigned int nw, unsigned int Ns);

inline unsigned long long getTime(void);

// Window
void chooseAllInWindow(Configuration *conf, unsigned int offset, unsigned int* vect, unsigned int deg);
void chooseAllInWindowCore(unsigned int w, unsigned int offset, unsigned int* vect, unsigned int deg);
unsigned int rand2window(Configuration *conf, unsigned int offset, double symb);
unsigned int rand2windowCore(unsigned int w, unsigned int offset, double symb);

int degree(double *prob_space, unsigned int w, double n);
void sort(unsigned int *array, unsigned int begin, unsigned int end);
Tbool verifySelectedSymbol(unsigned int *selected, unsigned int last);

// Decoder
unsigned int reduceDegree(Symbol **current, char *decoded, char *decodingStatus,
                          unsigned int old_degree, int symbLen, unsigned int new_degree,
                          XorType type);
void rmSymbol(Node **head, Node *node);

unsigned int computeRealDegree(Symbol *current, char *decodingStatus, int symbLen,
                               unsigned int old_degree, unsigned int k);

Tbool isSymbolDecoded(char *decodingStatus, unsigned int startPos, int symbLen);

#endif /*SHAREDMETHODS_H_*/
