# libLT: a free fountain code library

# This file is part of libLT.

# libLT is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation version 3 of the License.

# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "../../libLT_C.h"

#include <stdlib.h>
#include <stdio.h>

from ctypes import *
import sys

libc = cdll.LoadLibrary("libc.so.6")
libLT = cdll.LoadLibrary("../../_libLT.so")


l=1       # Information Size of each symbol (bits) 
k=1000    # Number of input symbols
w=k       # Number of symbols in each window
s=w       # Number of symbols of movements
c=c_double(0.4)     # LT parameter c
delta=c_double(0.5) # LT parameter delta
seed=663  # Seed of the random generator for the encoding process
N=2*k	  # Maximum number of encoded symbols to be created

#sourceInfo = libLT.chk_calloc(k*l,sizeof(c_char))
#for i in xrange(0,k*l):
#    sourceInfo[i] = c_ubyte(i % 256)
SizeSourceInfo = c_ubyte * (k*l)
sourceInfo = SizeSourceInfo()
for i in xrange(0,k*l):
    sourceInfo[i] = c_ubyte(i % 256)

# Encoder
encoder = libLT.mallocEncoder()
libLT.initEncoder_LT(encoder,seed,l,k,w,s,delta,c,N)
#print 'DEBUG: Initializing done'
libLT.LTencode(encoder,sourceInfo,c_ulonglong(len(sourceInfo)))
#print 'DEBUG: Encoding done'
encodedInfo = libLT.getOutput(encoder)
print 'Encoding the source information completed.'

decoder = libLT.mallocDecoder()
mybucketid = libLT.addBucket(decoder,seed,l,k,w,s,delta,c,c_double(0.0),N,1)
numNewReceived = libLT.receiveSymb_BucketName(decoder,mybucketid,encodedInfo,N*l,0)
libLT.decodingProcess(decoder)
print '\nnumber of source symbols = %d' % k
print 'number of received symbols = %d' % numNewReceived
print 'TOT received symbols: %d' % libLT.getNumRecSymbols(decoder)
print 'TOT decoded symbols: %d' % libLT.getNumDecSymbols(decoder)

decoded = create_string_buffer(libLT.getDecoded(decoder))
#print 'sourceInfo:%s\n' % [s for s in sourceInfo]
print '%s' % ''.join(['_'+str(ord(d)) for d in decoded[0:len(sourceInfo)]])
#for i in xrange(9,k*l):
#    sys.stdout.write('_%d' % (decoded[i] - sourceInfo[i]))
#sys.stdout.write('\n')

#libc.free(sourceInfo)
libLT.freeEncoder(encoder)
libLT.freeDecoder(decoder)
