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
libc = cdll.LoadLibrary('libc.so.6')

if __name__=="__main__":

    # Simple initialization
    l = 1;
    k = 1000;
    w = k;
    s = w;
    c = 0.4;
    delta = 0.5;
    seed = 663;
    N = 2*k;
    code = 1;

    # Create the input source
    Type_sourceInfo = c_ubyte * (k*l)
    sourceInfo = Type_sourceInfo()
    for i in xrange(0,k*l):
        sourceInfo[i] = c_ubyte(i % 256)


    # Encoder FLY

    # Allocate the encoder data structure
    encoder = libLT.mallocEncoderFLY(seed)
    # Init the encoder data structure
    libLT.setEncoderFLY_LT(encoder,w,s,delta,c)

    # Allocate the space to store the encoded symbols
    # encodedInfo = libc.malloc(N*l*ctypes.sizeof(ctypes.c_char))

    # encode all the source symbols together (Only one window and it generates N encoded symbols)
    (encodedInfo,firstEncSymbol) = libLT.LTencodeInfoFLY(encoder, l, sourceInfo, 0, N)

    print 'Encoding the source information completed.'

    # Decoder FLY

    # Allocate the decoder data structure
    decoder = libLT.mallocDecoderFLY(N)
    # Add one bucket to the decoder (you need to add one per each seed or different parameters used. In practice, one per encoder)
    myBucket = libLT.addAndInitBucketFLY_LT(decoder,seed,l,w,s,delta,c)
    numNewReceived = libLT.receiveSymbFLY_BucketName(decoder,myBucket,encodedInfo,0,0)
    # Decode the symbols
    libLT.decodingProcessFLY(decoder)

    # See new decoded symbols
    print '\nnumber of source symbols = %s' % k
    print "number of received symbols = %d" % libLT.getNumReceived(decoder)
    
    # Count all decoded symbols
    print "TOT received symbols: %d" % libLT.getNumReceived(decoder)
    print "TOT decoded symbols: %d" % libLT.getNumDecoded(decoder)


    result = libLT.getDecodedInformation(decoder)

    for i in xrange(0,k*l):
        sys.stdout.write('_%s' % (c_ubyte.from_address(result+sizeof(c_ubyte)*i).value - sourceInfo[i]))
    sys.stdout.write('\n')
    
    
    # Finally, free the memory
    #libc.free(sourceInfo)
    
    libLT.freeEncoderFLY(encoder)
    libLT.freeDecoderFLY(decoder)
