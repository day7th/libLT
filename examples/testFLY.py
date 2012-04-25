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

import libLT

import ctypes
libc = ctypes.cdll.LoadLibrary('libc.so.6')

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
    sourceInfo = libc.calloc(k*l,ctypes.sizeof(ctypes.c_char))

    # Encoder FLY

    # Allocate the encoder data structure
    encoder = libLT.mallocEncoderFLY(seed)
    # Init the encoder data structure
    libLT.setEncoderFLY(encoder,code,w,s,delta,c,0.0)

	# Allocate the space to store the encoded symbols
    # encodedInfo = libc.malloc(N*l*ctypes.sizeof(ctypes.c_char))

	# encode all the source symbols together (Only one window and it generates N encoded symbols)
    (encodedInfo,firstEncSymbol) = libLT.LTencodeFLY(encoder, l, sourceInfo, 0, N)

    # Decoder FLY

	# Allocate the decoder data structure
    decoder = libLT.mallocDecoderFLY(N)
	# Add one bucket to the decoder (you need to add one per each seed or different parameters used. In practice, one per encoder)
    myBucket = libLT.addAndInitBucketFLY_LT(decoder,seed,l,w,s,delta,c)
    # In order to receive the encoded symbols in the correct bucket, it is necessary to get the pointer to the right bucket
    myBucketPtr = libLT.getBucketFLY(decoder,myBucket)
    # Receive the encoded symbols
    numNewReceived = libLT.receiveSymbFLY(decoder, myBucketPtr, encodedInfo, 0, 0)
    # Decode the symbols
    libLT.decodingProcessFLY(decoder)

    # See new decoded symbols
    print "number of received symbols = %d\n" % libLT.getNumReceived(decoder)
    
    # Count all decoded symbols
    print "TOT received symbols: %d\n" % libLT.getNumReceived(decoder)
    print "TOT decoded symbols: %d\n" % libLT.getNumDecoded(decoder)


    libc.printf("sourceInfo:'%s' decoded:'%s'\n",sourceInfo,libLT.getDecoded(decoder))
    
    
    # Finally, free the memory
    libc.free(sourceInfo)
    
    libLT.freeEncoderFLY(encoder)
    libLT.freeDecoderFLY(decoder)
