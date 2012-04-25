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
// File       : node.c
// Author     : Pasquale Cataldi, Andrea Tomatis
// Company    :
// Last update: 2010/12/30
// Platform   : Ansi C
//*****************************************************************************
// Description: This file contains functions for the management of a list
//*****************************************************************************
// Revisions  :
// Date        Version  Author  Description
// 2008/07/17  0.1      PC,AT   First release.
// 2010/12/30  0.2      PC      Added memory check to the addNode function.
//*****************************************************************************

#include <stdlib.h>

#include "node.h"
#include "../utils/utilMem.h"

Node* addNode(Node* prev) {
    Node *node = NULL;

    node = (Node*) chk_malloc(1, sizeof(Node));
    node->prev = prev;
    node->next = NULL;
    node->data = NULL;

    return node;
}

void rmNode(Node **ptr) {

    Node *current 	= (*ptr);
    Node *next 		= (*ptr)->next;
    Node *prev 		= (*ptr)->prev;


    if (prev != NULL) {
        if (next == NULL) {
            prev->next = NULL;
        } else {
            prev->next = next;
            if (next != NULL)
                next->prev = prev;
        }
    } else {
        if (next != NULL)
            next->prev = NULL;
    }

    free(current);
    current=NULL;
}

