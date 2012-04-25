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

if len(sys.argv) != 4:
    sys.exit('ERROR passing arguments!\n'+
             'The first argument is the input file, '+\
             'the second is the decoded file and the '+\
             'third the encoded file.')

infile  = sys.argv[1]
outfile = sys.argv[2]
encfile = sys.argv[3]

print("---Encoding file %s" % infile);
print("---Recovered Info in file %s" % outfile);
print("---Encoded file %s" % encfile);

sourceInfo,infilelen = libLT.readFile(infile)
if sourceInfo == 0:
    sys.exit('ERROR source file not correctly read')

l = 1;
k = infilelen.value/l;
w = k;
s = w;
c = 0.4;
delta = 0.5;
seed = 663;
N = 2*k;

print('l:%s w:%s s:%s c:%s delta:%s seed:%s N:%s' % (l,w,s,c,delta,seed,N))

print('number of input symbols = %d' % k)

from ctypes import *
class Symbol(Structure):
    _fields_ = [('id',c_uint),
                ('deg',c_uint),
                ('header',POINTER(c_uint)),
                ('info',c_char_p),
                ('symbLen',c_int)]

# ENCODER FLY
encoder = libLT.mallocEncoderFLY(seed)
libLT.setEncoderFLY_LT(encoder,w,s,delta,c)

libLT.LTencodeFLY_toFile(encoder,N,l,sourceInfo,0,encfile)

# DECODER FLY
decoder = libLT.mallocDecoderFLY(N)
decodedInfo = libLT.LTdecodeFLY_fromFile(decoder,encfile)

if (libLT.writeFile(outfile, decodedInfo, k*l) == k*l):
    print("Output written correctly.")
else:
    sys.exit("Output NOT written correctly.")
