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
// File       : utilMem.h
// Author     : Pasquale Cataldi
// Company    :
// Last update: 2010/12/30
// Platform   : Ansi C
//*****************************************************************************
// Description: This file contains the declarations of the functions
//				for memory allocation
//*****************************************************************************
// Revisions  :
// Date        Version  Author  Description
// 2010/12/30  0.1      PC      First version.
//*****************************************************************************


#ifndef UTILMEM_H_
#define UTILMEM_H_

/**
 * @brief Allocates initialized memory.
 * @param[in] n Number of elements to be allocated.
 * @param[in] size Size of each element, in bytes.
 * @return Pointer to the allocated memory.
 */
void *chk_calloc(unsigned n, unsigned size);

/**
 * @brief Allocates non-initialized memory.
 * @param[in] n Number of elements to be allocated.
 * @param[in] size Size of each element, in bytes.
 * @return Pointer to the allocated memory.
 */
void *chk_malloc(unsigned n, unsigned size);

/**
 * @brief Re-allocates memory.
 *
 * The function may move the memory block to a new location, in which case the new location is returned.
 *
 * @param[in] ptr Pointer to the original block to be reallocated.
 * @param[in] n Total number of elements to be allocated (old, if any, and new ones).
 * @param[in] size Size of each element, in bytes.
 * @return Pointer to the reallocated memory block, which may be either the same as the ptr argument or a new location.
 */
void *chk_realloc(void * ptr, unsigned n, unsigned size);


#endif /* UTILMEM_H_ */
