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


#ifndef _UTIL_BS_H_
#define _UTIL_BS_H_

/* Creates a 8 bytes string in which the n-th
 * byte is the hexadecimal rappresentation
 * of the n-th argument bit.
 *
 * @param the character to encode
 * @return a 8 bit string
 */
char *charToHex(char);

/* Convert the hexadecimal rappresentation
 * of the bits of a character in the
 * character itself.
 *
 * @param the hexadecimal string length
 * @param the hexadecimal string
 * @return the character
 */
char hexToChar(int,char*);

/* Apply charToHex to every character of
 * a string and return the result.
 *
 * @param the string length
 * @param the string
 * @return the hexadecimal string
 */
char *strToHexstr(int,char*);

/* Apply hexToChar to every group of 8
 * hexadecimal values to recreate the
 * original string.
 *
 * @param the hexadecimal string length
 * @param the hexadecimal string
 * @return the string decoded
 */
char *hexstrToStr(int,char*);

// Testing purpose
int testHexChar();
int testHexStr(int,char*);
void printHex(int,char*);

#endif //  _UTIL_BS_H_
