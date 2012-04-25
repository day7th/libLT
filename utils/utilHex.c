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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utilHex.h"
#include "utilMem.h"

#define ASCIILEN 8


char *charToHex(char c) {

    char *res = (char*) chk_calloc(ASCIILEN,sizeof(char));

    int i;
    for (i=0; i<ASCIILEN; i++) {
        res[ASCIILEN-1-i] = (c >> i) & 0x01;
        //    printf("res[%d] = %x\n",ASCIILEN-1-i,res[ASCIILEN-1-i]);
    }
    //printf("%c => '%s'\n",c,res);

    return res;
}

char hexToChar(int len,char *h) {

    char res = 0x00;

    int i=0;
    for (; i<len; i++) {
        res |= (h[i] << (ASCIILEN-1-i));
    }

    return res;
}

int testHexChar() {
    int charHexIdentity = 1;
    int i = 0;
    for (; i<128; i++) {
        char *res = charToHex(i);
        charHexIdentity &= i == hexToChar(ASCIILEN,res);
        free(res);
    }

    return charHexIdentity;
}

char *strToHexstr(int len, char *s) {
    int newlen = len*ASCIILEN;
    char *res = (char*) chk_calloc(newlen,sizeof(char));

    int i;
    for (i=0; i<len; i++) {
        char* si_coded = charToHex(s[i]);
        int j;
        for (j=0; j<ASCIILEN; j++) {
            res[j+i*ASCIILEN] = si_coded[j];
        }
        free(si_coded);
    }

    return res;
}

char *hexstrToStr(int len, char *s) {
    int reslen = len/ASCIILEN;
    char *res = (char*) chk_calloc(reslen,sizeof(char));

    int i = 0;
    for (; i<reslen; i++) {
        res[i] = hexToChar(ASCIILEN,&s[i*ASCIILEN]);
//        printf("res[%d]=%c\n",i,res[i]);
    }

    return res;
}

int testHexStr(int len, char *s) {
    int res;

    char *hexstr = strToHexstr(len,s);
//    printf("hexstr='%s'\n",hexstr);
    char *ns = hexstrToStr(len*ASCIILEN,hexstr);
    res = (0 == strcmp(s,ns));
//    printf("\n\nstrcmp(%s,%s):%d\n\n",s,ns,strcmp(s,ns));
    free(hexstr);
    free(ns);

    return res;
}

void printHex(int len, char* s) {
    int i=0;
    for (; i<len; i++) {
        printf("%x",s[i]);
    }
}


//int main(int argn,char** argv) {
//
//    printf("testHexChar %s\n",testHexChar() == 0 ? "ERROR" : "works");
//    char *testString = "prova";
//    printf("testHexStr %s\n",testHexStr(5,testString) == 0 ? "ERROR" : "works");
//
//    if (argn < 2) {
//        printf("Usage: %s <file_name>\n",argv[0]);
//    }
//    else {
//        FILE *fp;
//        char c;
//        unsigned int size = 256,
//                     i    = 0;
//        char *fcontent = (char*) chk_calloc(size,sizeof(char));
//
//        fp = fopen(argv[1],"r");
//        while( (c = getc(fp)) != EOF ) {
//            fcontent[i] = c;
//            i+=1;
//            if (i == size) {
//                size *= 2;
//                char *fcontent_tmp = (char*) chk_calloc(size,sizeof(char));
//                strcpy(fcontent_tmp,fcontent);
//                free(fcontent);
//                fcontent = fcontent_tmp;
//            }
//        }
//        fclose(fp);
//
//        int clen = i*ASCIILEN;
//        printf("encoded:");
//        char *res = strToHexstr(i,fcontent);
//        printHex(clen,res);
//        printf("\n");
//        char *decres = hexstrToStr(clen,res);
//        printf("decoded:%s\n",decres);
//        printf("start==end:%s\n", strcmp(fcontent, decres) == 0 ? "True" : "False");
//        free(res);
//        free(decres);
//    }
//
//    return 0;
//}
