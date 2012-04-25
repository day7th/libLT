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
// File       : sharedMethods.c
// Author     : Pasquale Cataldi, Andrea Tomatis
// Company    :
// Last update: 2010/12/30
// Platform   : Ansi C
//*****************************************************************************
// Description: This file contains the functions
//				in common between encoder and decoder
//*****************************************************************************
// Revisions  :
// Date        Version  Author  Description
// 2008/07/17  0.1      PC,AT   First release
// 2008/07/17  0.2      PC,AT   Add getTime: number of clock
// 2008/10/25  0.3      PC      The degree distributions compute the theorical avg degree
//								The create_headers_p2p returns the avg degree for the
//									generated symbols
// 2008/11/11  0.4      PC      Fixed the error in the computation of the th avg degree
//									in Robust and Shok degree distribution
// 2009/04/24  0.5      PC      Modified the shifting "if" in create_headers_nw()
// 2010/12/30  0.6      PC      Moved the interleaving functions to utilInterleavers.h.
// 2010/12/30  0.7      PC      Moved the computeStdDevOnTheFly function to utilStats.h.
//*****************************************************************************

#include <math.h>
#include <string.h>

#include "sharedMethods.h"
#include "../random/randomPA.h"
#include "utilMem.h"
#include "utilErr.h"

double create_headers(Mgenerator *matrix) {

    unsigned int ii,jj;
    unsigned int offset = 0;
    double avgDegree = 0.0;
    // Create the header of each symbol
    unsigned int generated_degree = 0;
    unsigned int *selected_symbols = NULL;
    double sel;

    unsigned int w = matrix->conf.w;
    double *cumulative = matrix->cumulative;

    // set randoms
    randomPA myrandom;
    initRandom(&myrandom, matrix->seed);

    // Allocate at the beginning to save time
    selected_symbols = (unsigned int*) chk_malloc(w, sizeof(unsigned int));

    for (ii=0; ii<matrix->conf.N; ii++) {

        matrix->symbols[ii].header	= NULL;
        matrix->symbols[ii].deg		= 0;

        generated_degree = degree(cumulative, w, real(&myrandom));
        if (generated_degree > w)
            generated_degree = w;

        avgDegree += (double) generated_degree;

        if (generated_degree == w) {
            chooseAllInWindow(&(matrix->conf), offset, selected_symbols, generated_degree);
        } else {
            for (jj=0; jj<generated_degree; jj++) {
                sel	= real(&myrandom);
                selected_symbols[jj]	= rand2window(&(matrix->conf), offset, sel);

                if (verifySelectedSymbol(selected_symbols, jj) == False) {
                    jj--;		// Doesn't repeat encoding symbols
                }
            }
            sort(selected_symbols, 0, generated_degree-1);	// Sort the chosen encoding symbols' list
        }

        matrix->symbols[ii].header = (unsigned int *) chk_calloc(generated_degree, sizeof(unsigned int));
        memcpy(matrix->symbols[ii].header, selected_symbols, generated_degree*sizeof(unsigned int));
        matrix->symbols[ii].deg = generated_degree;

        /* For each encoded symbols choose a different window...
         * in this way, you will have the same amount of encoded symbols per each window.
         * The drawback is that when you have several windows, the overall performance will depend
         * from the worst window...
         */
        offset = ((offset + matrix->conf.s) % (matrix->conf.k));
    }
    // free and initialize the tmp symbol
    free(selected_symbols);
    selected_symbols = NULL;

    return (avgDegree/matrix->conf.N);
}

// nw is the number of ESs that are generated together in each window.
// Since there are Ns windows, the remaining N-Ns*nw (if any)
// will be generated one per window starting from the first
double create_headers_nw(Mgenerator *matrix, unsigned int nw, unsigned int Ns) {

    unsigned int ii,jj;
    unsigned int offset = 0;
    double avgDegree = 0.0;
    // Create the header of each symbol
    unsigned int generated_degree = 0;
    unsigned int *selected_symbols = NULL;
    double sel;

    unsigned int w = matrix->conf.w;
    double *cumulative = matrix->cumulative;

    unsigned int genPerWindow = 0;

    if (nw*Ns > matrix->conf.N) {
        print_error("(libLT_C:sharedMethods.c:create_headers_nw) "
                    "ERROR in the creation of the headers: nw is too large\n");
        exit(EXIT_FAILURE);
    }

    // Allocate at the beginning to save time
    selected_symbols = (unsigned int*) chk_malloc(w, sizeof(unsigned int));

    // set randoms
    randomPA myrandom;
    initRandom(&myrandom, matrix->seed);

    for (ii=0; ii<matrix->conf.N; ii++) {

        matrix->symbols[ii].header	= NULL;
        matrix->symbols[ii].deg		= 0;

        generated_degree = degree(cumulative, w, real(&myrandom));
        if (generated_degree > w)
            generated_degree = w;

        avgDegree += (double) generated_degree;

        if (generated_degree == w) {
            chooseAllInWindow(&(matrix->conf), offset, selected_symbols, generated_degree);
        } else {
            for (jj=0; jj<generated_degree; jj++) {
                sel	= real(&myrandom);
                selected_symbols[jj]	= rand2window(&(matrix->conf), offset, sel);

                if (verifySelectedSymbol(selected_symbols, jj) == False) {
                    jj--;		// Doesn't repeat encoding symbols
                }
            }
            sort(selected_symbols, 0, generated_degree-1);	// Sort the chosen encoding symbols' list
        }

        matrix->symbols[ii].header = (unsigned int *) chk_calloc(generated_degree, sizeof(unsigned int));
        memcpy(matrix->symbols[ii].header, selected_symbols, generated_degree*sizeof(unsigned int));
        matrix->symbols[ii].deg = generated_degree;

        // The following IF manages whether to move the window or not.
        genPerWindow++;
        if (genPerWindow==nw) {
            genPerWindow = 0;
            offset = ((offset + matrix->conf.s) % (matrix->conf.k));
        }
    }

    // free and initialize the tmp symbol
    free(selected_symbols);
    selected_symbols = NULL;

    return (avgDegree/matrix->conf.N);
}

int degree(double *prob_space, unsigned int w, double n) {
    unsigned int deg =0;
    unsigned int i;
    double max_ref = prob_space[w-1]*n;

    for (i=0; i<w; i++) {
        if (prob_space[i]>=max_ref) {
            deg = (i+1);
            break;
        }
    }
    return deg;
}

void sort(unsigned int *array, unsigned int begin, unsigned int end) {
    unsigned int temp;
    if (end > begin) {
        unsigned int pivot = array[begin];
        unsigned int l = begin + 1;
        unsigned int r = end+1;
        while (l < r) {
            if (array[l] < pivot) {
                l++;
            } else {
                r--;
                temp 		= array[l];
                array[l]	= array[r];
                array[r]	= temp;
            }
        }
        l--;
        temp			= array[begin];
        array[begin]	= array[l];
        array[l]		= temp;
        sort(array, begin, l);
        sort(array, r, end);
    }
    return;
}




Tbool verifySelectedSymbol(unsigned int *selected, unsigned int last) {
    unsigned int i;
    if (selected == NULL) {
        print_error("(libLT_C:sharedMethods.c:verifySelectedSymbol) "
                    "ERROR - NULL vector as a argument.\n");
        exit(EXIT_FAILURE);
    }
    unsigned int target = selected[last];
    for (i=0; (last>=0)&&(i<last); i++) {
        if (selected[i]==target) {
            return False;
        }
    }
    return True;
}



double SHOKcumulative(double *cumulative, unsigned int w, unsigned int s,
                      double epsShok) {

    double	m	= 0;					// parameter of the distribution
    double	*omega_shok=NULL;
    double	*omega_shok_norm=NULL;
    double	avgDeg = 0.0;
    unsigned int	i	= 0;					// index
    unsigned int	ref	= 0;
    unsigned int	wvirt = (2*w-s);

    unsigned int D = (unsigned int)(4.0*(1.0+epsShok)/epsShok + 0.5);
    m = epsShok*(2.0+epsShok)/4.0;

    if (wvirt>D) {
        ref = D;
    } else {
        ref = (double) wvirt;
    }

    omega_shok		= (double*) chk_calloc(wvirt, sizeof(double));
    omega_shok_norm	= (double*) chk_calloc(wvirt, sizeof(double));

    omega_shok[0] = ((double) m/(m+1.0));
    for (i = 1; i < (ref-1); i++) {
        omega_shok[i] = 1.0/(((i+1.0)*i)*(m+1.0));
    }
    omega_shok[ref] = 1.0/(ref*(m+1.0));

    double normfact = 0.0;
    for (i=0; i<w; i++) {
        normfact += omega_shok[i];
    }
    for (i=0; i<w; i++) {
        omega_shok_norm[i] = omega_shok[i]/normfact;
        avgDeg += omega_shok_norm[i]*(i+1);
    }

    cumulative[0] = omega_shok_norm[0];
    for (i=1; i<w; i++) {
        cumulative[i] = (double)(cumulative[i-1]+omega_shok_norm[i]);
    }

    free(omega_shok);
    free(omega_shok_norm);

    return avgDeg;
}

// for LTencoder and LTdecoder
double shokrollahi_distribution(Mgenerator *matrix) {
    unsigned int w = matrix->conf.w;
    unsigned int s = matrix->conf.s;
    double epsShok = matrix->conf.epsShok;

    return SHOKcumulative(matrix->cumulative, w, s, epsShok);
}


double RSDcumulative(double *cumulative, unsigned int w, unsigned int s,
                     double delta, double c) {

    unsigned int wvirt = (2*w-s);
    double lbound = sqrt(wvirt)/((log(wvirt/delta))*(wvirt-1));
    double ubound = sqrt(wvirt)/((log(wvirt/delta))*2.0);
    if ((c<lbound)||(c>ubound)) {
        print_error("(libLT_C:LTencoderFLY:RSDcumulative)\n"
                    "Error in setting the parameters of the Soliton distribution:\n"
                    "with 'k virt' = %d and 'delta' = %f\n"
                    "the allowed values of 'c' are between %f and %f\n"
                    "while 'c' is %f\n",
                    wvirt, delta, lbound, ubound, c);
        exit(EXIT_FAILURE);
    }

    if ((c<=0)||(delta<=0)) {
        print_error("(libLT_C:LTencoderFLY:RSDcumulative) Error: c and delta not defined!\n");
        exit(EXIT_FAILURE);
    }

    double	*rho= NULL;
    double	*tau= NULL;
    double	*mu	= NULL;
    double	*mu_norm = NULL;
    double	beta= 0.0;
    double	R	= 0.0;
    double	avgDeg = 0.0;
    unsigned int	step= 0;		// floor of R
    unsigned int	i	= 0;		// index for cycles
    unsigned int	j   = 0;

    // tmp vector allocations
    rho = (double*) chk_calloc(wvirt, sizeof(double));
    tau = (double*) chk_calloc(wvirt, sizeof(double));
    mu = (double*) chk_calloc(wvirt, sizeof(double));
    mu_norm = (double*) chk_calloc(w, sizeof(double));


    rho[0] = (double) 1/wvirt;
    beta += rho[0];
    for (i = 1; i < wvirt; i++) {
        rho[i] = (double) 1/(i*(i+1));
        beta += rho[i];
    }

    R = (double) c*log(wvirt/delta)*sqrt((double) wvirt);
    step = (int)((double)wvirt/R - 0.5) - 1;
    for (i = 0; i < step; i++) {
        tau[i] = (double) R/((i+1)*wvirt);
        beta += tau[i];
    }
    tau[step] = (double) R*log((double) R/delta)/((double)wvirt);
    beta += tau[step];

    for (i=0; i<wvirt; i++) {
        mu[i] = (double)(rho[i] + tau[i]);
    }

    for (i=0; i<wvirt; i++) {
        mu[i] = mu[i]/beta;
    }

    double normfact = 0.0;
    for (i=0; i<w; i++) {
        normfact += mu[i];
    }
    for (i=0; i<w; i++) {
        mu_norm[i] = mu[i]/normfact;
        avgDeg += mu_norm[i]*(i+1);
    }

    cumulative[0] = mu_norm[0];
    for (j=1; j<w; j++) {
        cumulative[j] = (double)(cumulative[j-1]+mu_norm[j]);
    }

    free(rho);
    free(tau);
    free(mu);
    free(mu_norm);

    return avgDeg;
}

// for LTencoder and LTdecoder
double rubust_soliton_distribution(Mgenerator *matrix) {

    unsigned int w = matrix->conf.w;
    unsigned int s = matrix->conf.s;
    double delta = matrix->conf.delta;
    double c = matrix->conf.c;

    return  RSDcumulative(matrix->cumulative, w, s, delta, c);
}

void chooseAllInWindowCore(unsigned int w, unsigned int offset, unsigned int* vect, unsigned int deg) {
    unsigned int i;
    if (deg!=w) {
        print_error("(libLT_C:sharedMethods.c:choose_all_in_windowCore) "
                    "ERROR choosing the symbols for the encoding process.\n");
        exit(EXIT_FAILURE);
    }
    for (i=0; i<w; i++)
        vect[i] = (offset+i);
    return;
}

void chooseAllInWindow(Configuration *conf, unsigned int offset, unsigned int* vect, unsigned int deg) {
    unsigned int i;
    chooseAllInWindowCore(conf->w, offset, vect, deg);
    for (i=0; i<conf->w; i++)
        vect[i] = (vect[i] % conf->k);
    return;
}

unsigned int rand2windowCore(unsigned int w,
                             unsigned int offset, double symb) {	// Uniform distribution
    if (symb>1.0) {
        print_error("(libLT_C:sharedMethods.c:rand2windowCore) "
                    "ERROR while selecting the encoding symbols: the target symbol is out of the current window!");
        exit(EXIT_FAILURE);
    }
    return ((unsigned int)(offset+(int)(symb*w)));

}

unsigned int rand2window(Configuration *conf, unsigned int offset, double symb) {	// Uniform distribution

    unsigned int chosen = rand2windowCore(conf->k, offset, symb);

    return (chosen %  conf->k);
}

// Read the computer's timer RDTSC
inline unsigned long long getTime(void) {
    unsigned long lo, hi;
    unsigned long long output;
    __asm__ __volatile__(
        "xorl %%eax, %%eax\n"
        "cpuid\n"
        "rdtsc\n"
    : "=a"(lo), "=d"(hi)
                :
                : "%ebx", "%ecx");

    output = (unsigned long long) hi << 32 | lo;
    return output;
}



// ===========================================================================
//					DECODER
// ===========================================================================

unsigned int reduceDegree(Symbol **current, char *decoded, char *decodingStatus,
                          unsigned int old_degree, int symbLen, unsigned int new_degree,
                          XorType type) {

    unsigned int check = 0;
    unsigned int undecoded = 0;
    unsigned int last_not_decoded = 0;
    unsigned int i, j;
    unsigned int currSymb = 0;

    char *new_info= NULL;
    int *new_header= NULL;

    new_info = (char *) chk_calloc(symbLen, sizeof(char));
    new_header = (int *) chk_calloc(new_degree, sizeof(unsigned int));

    memcpy(new_info, (*current)->info, symbLen*sizeof(char));

    for (i=0; i<old_degree; i++) {
        check = (*current)->header[i];
        currSymb = check * symbLen;
        if (decodingStatus[currSymb] != 0x00) {
            // EX-OR COMPUTATION
            if (type != NO) {
                calcXOR(&decoded[currSymb], new_info, symbLen, type);
            } else {
                for (j=0; j<symbLen; j++) {
                    new_info[j] ^= decoded[currSymb + j];
                }
            }
        } else {
            new_header[undecoded] = check;
            last_not_decoded = check;
            undecoded++;
        }
    }
    memcpy((*current)->info, new_info, symbLen*sizeof(char));
    free((*current)->header);
    (*current)->deg = new_degree;
    (*current)->header = (unsigned int *) chk_malloc(new_degree, sizeof(unsigned int));
    memcpy((*current)->header, new_header, new_degree*sizeof(unsigned int));

    if (new_info!=NULL)
        free(new_info);
    new_info = NULL;

    if (new_header!=NULL)
        free(new_header);
    new_header = NULL;

    return last_not_decoded;
}


void rmSymbol(Node **head, Node *node) {
    if (node == (*head))
        (*head) = node->next;

    if (node->data != NULL) {
        if (((Symbol *)node->data)->header != NULL)
            free(((Symbol *)node->data)->header);
        if (((Symbol *)node->data)->info != NULL)
            free(((Symbol *)node->data)->info);
        free(node->data);
        node->data = NULL;
    }
    rmNode(&node);
}

unsigned int computeRealDegree(Symbol *current, char *decodingStatus, int symbLen,
                               unsigned int old_degree, unsigned int k) {

    unsigned int new_degree = 0;
    unsigned int check = 0;
    unsigned int i = 0;

    if (current == NULL) {
        print_error("ERROR: compute_real_degree - current symbol does not exist.\n");
        exit(EXIT_FAILURE);
    }

    if (current->header == NULL) {
        print_error("ERROR: No header in compute_real_degree\n");
        exit(EXIT_FAILURE);
    }

    if (decodingStatus == NULL) {
        print_error("ERROR: No decoded vector in compute_real_degree\n");
        exit(EXIT_FAILURE);
    }

    for (i=0; i<old_degree; i++) {
        check = (unsigned int)((current->header)[i]);

        if ((check<0)||(check>k)) {
            print_error("ERROR: compute_real_degree: check out of bound\n");
            exit(EXIT_FAILURE);
        }

        if (isSymbolDecoded(decodingStatus, check*symbLen, symbLen) == False)
            new_degree++;
    }
    return new_degree;
}

Tbool isSymbolDecoded(char *decodingStatus, unsigned int startPos, int symbLen) {
    unsigned int i;
    for (i = startPos; i < startPos + symbLen; i++)
        if (decodingStatus[i] == 0x00)
            return False;
    return True;
}


// =============================================================================

