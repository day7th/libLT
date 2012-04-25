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

#include "ciofile.h"
#include "utilMem.h"
#include "utilErr.h"

char *readFile(char* infile, unsigned long long* infilelen) {
    FILE *file = NULL;
    char *buffer = NULL;

    //Open file
    file = fopen(infile, "rb");
    if (!file) {
        print_error("Unable to open file %s", infile);
        return NULL;
    }

    //Get file length
    fseek(file, 0, SEEK_END);
    *infilelen=ftell(file);
    fseek(file, 0, SEEK_SET);

    //Allocate memory
    buffer=(char *) chk_malloc(*infilelen+1, sizeof(char));

    //Read file contents into buffer
    unsigned long long readBytes = fread(buffer, 1, *infilelen, file);
    if (readBytes != *infilelen)
        print_error("ERROR reading from %s\n", infile);
    fclose(file);

    return buffer;
}

char *readFileChunk(char* infile, unsigned long long startpos, unsigned long long chunklen) {
    FILE *file = NULL;
    char *buffer = NULL;

    //Open file
    file = fopen(infile, "rb");
    if (!file) {
        print_error("Unable to open file %s", infile);
        return NULL;
    }

    //Get file length
    fseek(file, 0, SEEK_END);
    unsigned long long infilelen=ftell(file);

    if (infilelen < startpos+chunklen) {
        print_error("Impossible to read %ul bytes starting from byte %lu.\n"
                    "From that position only %ul bytes can be read.\n"
                    "For this reason, the buffer was not loaded.", chunklen, startpos, startpos+chunklen-infilelen);
        return 0;
    }

    //Allocate memory
    buffer=(char *) chk_malloc(chunklen, sizeof(char));

    // Find the start position of the buffer to be read
    fseek(file, startpos, SEEK_SET);

    //Read file contents into buffer
    unsigned long long readBytes = fread(buffer, 1, chunklen, file);
    if (readBytes != chunklen)
        print_error("ERROR reading from %s\n", infile);
    fclose(file);

    return buffer;
}

char *readSetOfChunksFromFile(char* infile, unsigned int numchuncks, unsigned long long* chunkpositions, unsigned long long chunklen) {
    FILE *file = NULL;
    char *buffer = NULL;

    if (chunkpositions == NULL) {
        print_error("Impossible to read the chunks from the file since the positions were not set.\n");
        return 0;
    }

    //Open file
    file = fopen(infile, "rb");
    if (!file) {
        print_error("Unable to open file %s", infile);
        return NULL;
    }

    //Get file length
    fseek(file, 0, SEEK_END);
    unsigned long long infilelen=ftell(file);

    //Allocate memory
    buffer=(char *) chk_malloc(numchuncks, chunklen*sizeof(char));

    unsigned int i;
    for (i = 0; i < numchuncks; i++) {

        // Find the start position of the buffer to be read
        fseek(file, chunkpositions[i], SEEK_SET);

        // Check that the chunk can be safely read
        if (chunkpositions[i]+chunklen > infilelen) {
            print_error("The chunk %lu cannot be read because exceeds the file size. The buffer will be freed.\n", i);
            free(buffer);
            return 0;
        }

        //Read file contents into buffer
        unsigned long long readBytes = fread(buffer+(chunklen*i), 1, chunklen, file);
        if (readBytes != chunklen)
            print_error("ERROR reading from %s. Chunk %lu not read.\n", infile, i);
    }
    fclose(file);

    return buffer;
}

unsigned long long getFileSize(char* infile) {
    FILE *file = NULL;

    //Open file
    file = fopen(infile, "rb");
    if (!file) {
        print_error("Unable to open file %s", infile);
        return 0;
    }

    //Get file length
    fseek(file, 0, SEEK_END);
    unsigned long long infilelen=ftell(file);

    //Close file
    fclose(file);

    return infilelen;
}

unsigned long long writeFile(char* outfile, char* buffer, unsigned long long bufflen) {
    FILE *file = NULL;

    //Open file
    file = fopen(outfile, "wb");
    if (!file) {
        print_error("Unable to find or create file %s", outfile);
        return 0;
    }

    //Write buffer contents into file
    unsigned long long count = fwrite(buffer, sizeof(char), bufflen, file);
    if (ferror(file))
        fprintf(stderr, "ERROR writing to %s\n", outfile);
    fclose(file);

    return count;
}
