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
// File       : utilErr.c
// Author     : Pasquale Cataldi
// Company    :
// Last update: 2010/12/30
// Platform   : Ansi C
//*****************************************************************************
// Description: This file contains the implementation of the function
//				for error printing
//*****************************************************************************
// Revisions  :
// Date        Version  Author  Description
// 2010/12/30  0.1      PC      First version.
//*****************************************************************************

#include <stdio.h>
#include <stdarg.h>

void print_error(char * format, ...) {
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsprintf(buffer,format, args);
    perror(buffer);
    va_end(args);
}

