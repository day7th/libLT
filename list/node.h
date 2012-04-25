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
// File       : node.h
// Author     : Pasquale Cataldi, Andrea Tomatis
// Company    :
// Last update: 2008/07/17
// Platform   : Ansi C
//*****************************************************************************
// Description: This file contains the declarations of the functions
// 				for the management of a list
//*****************************************************************************
// Revisions  :
// Date        Version  Author  Description
// 2008/07/17  0.1      PC,AT   First release
//*****************************************************************************

#ifndef NODE_H_
#define NODE_H_

typedef struct Node {
    struct Node *prev;
    struct Node *next;
    void *data;
} Node;

Node* addNode(Node* prev);
void rmNode(Node **ptr);



#endif /*NODE_H_*/
