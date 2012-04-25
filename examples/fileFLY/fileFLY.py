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

import sys
sys.path.append('../../')
import libLT

from ctypes import *

if len(sys.argv) != 3:
    sys.exit('ERROR passing arguments!\n The first argument is the input file, the second is the decoded file.')

infile  = sys.argv[1]
outfile = sys.argv[2]

print('---Encoding file %s' % infile)
print('---Recevered Info in file %s' % outfile)

sourceInfo,infilelen = libLT.readFile(infile)

if sourceInfo == 0:
    sys.exit('ERROR source file not correctly read.')

l = 1
k = infilelen.value/l
w = k
s = w
c = 0.4
delta = 0.5
seed = 663
N = 2*k

print('number of input symbols = %d' % k)


# ENCODER FLY
encoder = libLT.mallocEncoderFLY(seed)
libLT.setEncoderFLY_LT(encoder,w,s,delta,c)
encodedInfo,_ = libLT.LTencodeInfoFLY(encoder,l,sourceInfo,0,N)
print('Encoding the source information completed.')

# DECODER FLY
decoder = libLT.mallocDecoderFLY(N)
myBucket = libLT.addAndInitBucketFLY_LT(decoder,seed,l,w,s,delta,c)
numNewReceived = libLT.receiveSymbFLY_BucketName(decoder,
                                                 myBucket,
                                                 encodedInfo,
                                                 N*l,
                                                 0,
                                                 0)
print('Decoding the received information.')
libLT.decodingProcessFLY(decoder)
print('\nnumber of received symbols = %d' % numNewReceived)
print('TOT received symbols: %d' % libLT.getNumReceived(decoder))
print('TOT decoded symbols: %d' % libLT.getNumDecoded(decoder))

outbuffer = decodedInfo = libLT.getDecodedInformation(decoder)
bytesToWrite = k*l

if (libLT.writeFile(outfile,outbuffer,bytesToWrite) == bytesToWrite):
    print('Output written correctly.')
else:
    print('Output NOT written correctly.')

libLT.freeEncoderFLY(encoder)
libLT.freeDecoderFLY(decoder)
