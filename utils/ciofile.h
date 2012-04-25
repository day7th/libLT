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


#ifndef CIOFILE_H_
#define CIOFILE_H_

/* Reads a file and store the content in a char buffer
 *
 * @param[in] infile Name of the file to read.
 * @param[out] *infilelen Length of the file in bytes.
 * @return The pointer to the buffer.
 */
char *readFile(char* infile, unsigned long long* infilelen);

/* Reads a chunk of data from a file and store the content in a char buffer
 *
 * @param[in] infile Name of the file to read.
 * @param[in] startpos Position of the first byte to be read.
 * @param[in] chunklen Length of the chunk to be read (in bytes).
 * @return The pointer to the buffer
 */
char *readFileChunk(char* infile, unsigned long long startpos, unsigned long long chunklen);

/* Given a certain file, it creates a buffer containing the chunks
 * of data of a given size and whose positions are specified as input parameter.
 *
 * @param[in] infile Name of the file to read.
 * @param[in] numchunks Number of data chunks to be read.
 * @param[in] chunkpositions Array containing the positions (in bytes from the beginning of the file) of the chunks to be read.
 * @param[in] chunklen Length of every chunk to be read (in bytes).
 * @return The pointer to the buffer
 */
char *readSetOfChunksFromFile(char* infile, unsigned int numchuncks, unsigned long long* chunkpositions, unsigned long long chunklen);

/* Given a certain file, it returns its size as number of bytes.
 *
 * @param[in] infile Name of the file to read.
 * @return File size.
 */
unsigned long long getFileSize(char* infile);

/* Writes a char buffer in a file
 *
 * @param[in] outfile Name of the file to written.
 * @param[in] buffer Buffer of data to be written.
 * @param[in] bufflen Length in bytes of the buffer.
 * @return Number of written bytes.
 */
unsigned long long writeFile(char* outfile, char* buffer, unsigned long long bufflen);


#endif /* CIOFILE_H_ */
