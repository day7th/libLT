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



from ctypes import *


# FIXME: Operative System problem here...
libLT = cdll.LoadLibrary("./_libLT.so")

####################################
######### {{{ ENCODER ##############

def mallocEncoderFLY(seed):
    """ LTencoderFLY* mallocEncoderFLY(unsigned long seed) """
    return libLT.mallocEncoderFLY(c_int(seed))


def freeEncoderFLY(encoder_p):
    """ void freeEncoderFLY(LTencoderFLY *encoder) """
    libLT.freeEncoderFLY(encoder_p)


def setEncoderFLY(encoder_p, code, w, s, lt_delta, lt_c, raptor_eps):
    """
    void setEncoderFLY(LTencoderFLY *encoder, DegDistr code,
		unsigned int w, unsigned int s, 
        double lt_delta, double lt_c, double raptor_eps);
    """
    libLT.setEncoderFLY(encoder_p,
        c_int(code),
        c_uint(w),
        c_uint(s),
        c_double(lt_delta),
        c_double(lt_c),
        c_double(raptor_eps))


def setEncoderFLY_LT(encoder_p,w,s,lt_delta,lt_c):
    """
    void setEncoderFLY_LT(LTencoderFLY *encoder,
         unsigned int w, unsigned int s,
         double lt_delta, double lt_c);
    """
    libLT.setEncoderFLY_LT(encoder_p,
        c_uint(w),
        c_uint(s),
        c_double(lt_delta),
        c_double(lt_c))

# TODO setEncoderFLY_Raptor


def resetEncoderFLY(e, newSeed):
    """ void resetEncoderFLY(LTencoderFLY *encoder, unsigned long newSeed) """
    libLT.resetEncoderFLY(e,c_ulong(newSeed))


def reseedEncoderFLY(e,newSeed):
    """ void reseedEncoderFLY(LTencoderFLY *encoder, unsigned long newSeed) """
    libLT.reseedEncoderFLY(e,c_ulong(newSeed))


# MOD: don't need the pointer to the output string because i'm in Python


def LTencodeInfoFLY(e,symbLen,info,win_firstSymbId,N):
    """
    unsigned int LTencodeFLY(LTencoderFLY *encoder, nt symbLen,
	    char *info, unsigned long offset,
		unsigned int N, char *output);

    @return: a tuple of output_p pointer to the result string
        and the result code
    """
    output_p = create_string_buffer(N*symbLen)
    res = libLT.LTencodeInfoFLY(e,
                                c_int(symbLen),info,
                                win_firstSymbId,
                                c_uint(N),
                                output_p)
    return (output_p,res)


def getSeedFromConfig(e,w,s,lt_delta,lt_c,raptor_eps):
    """
    unsigned long getSeedFromConfig(LTencoderFLY *encoder,
			unsigned int w, unsigned int s,
			double lt_delta, double lt_c, double raptor_eps)
    """
    return libLT.getSeedFromConfig(e,
        c_uint(w),c_uint(s),c_double(lt_delta),
        c_double(lt_c),c_double(raptor_eps))


def getSeedFromCurrConfig(e):
    """ unsigned long getSeedFromCurrConfig(LTencoderFLY *encoder) """
    return libLT.getSeedFromCurrConfig(e)


def LTencodeSymbolFLY(encoder_p,symblen,info,win_firstsymbid,N,encodedSymbols):
    """
    unsigned int LTencodeSymbolFLY(LTencoderFLY *encoder, int symbLen,
                               char *info, unsigned long window_firstSymbolId,
                               unsigned int N, Symbol* encodedSymbols);
    """
    return libLT.LTencodeSymbolFLY(encoder_p,symblen,info,win_firstsymbid,N,encodedSymbols)


def getNumEncodedSymbols(encoder_p):
    """ unsigned int getNumEncodedSymbols(LTencoderFLY *encoder) """
    return libLT.getNumEncodedSymbols(encoder_p)


def LTencodeSymbolFLY_buffer(encoder_p,symblen,info,info_firstsymbid,encodedsymbs,N,win_firstsymbolid):
    """
    unsigned int LTencodeSymbolFLY_buffer(
            LTencoderFLY *encoder,
            int symbLen,
            char* info,
            unsigned long info_firstSymbolId,
            Symbol* encodedSymbols,
            unsigned int N,
            unsigned long window_firstSymbolId);
    """
    return libLT.LTencodeSymbolFLY_buffer(encoder_p,
                                          c_int(symblen),
                                          info,
                                          c_ulong(info_firstsymbid),
                                          encodedsymbs,
                                          c_uint(N),
                                          c_ulong(win_firstsymbolid))


############# ENCODER }}} ##########
####################################

def LTencodeFLY_toFile(encoder_p,N,l,sourceInfo,info_firstSymbolId,outfile):
    libLT.LTencodeFLY_toFile(encoder_p,
                             c_uint(N),
                             c_uint(l),
                             sourceInfo,
                             c_ulong(info_firstSymbolId),
                             outfile)

####################################
######### {{{ DECODER ##############


def mallocDecoderFLY(decodedSymbolsBufferSize):
    """ LTdecoderFLY *mallocDecoderFLY(unsigned int decodedSymbolsBufferSize) """
    return libLT.mallocDecoderFLY(c_uint(decodedSymbolsBufferSize))


def freeDecoderFLY(d):
    """ void freeDecoderFLY(LTdecoderFLY *decoder) """
    libLT.freeDecoderFLY(d)


def resetDecoderFLY(d):
    """ void resetDecoderFLY(LTdecoderFLY *decoder) """
    libLT.resetDecoderFLY(d)


def decodingProcessFLY(d):
    """ void decodingProcessFLY(LTdecoderFLY *decoder) """
    libLT.decodingProcessFLY(d)


def getDecodedInformation(decoder):
    """ char *getDecodedInformation(LTdecoderFLY *decoder) """
    return libLT.getDecodedInformation(decoder)


def getDecodingStatusInformation(d):
    """ char *getDecodingStatusInformation(LTdecoderFLY *decoder) """
    return libLT.getDecodingStatusInformation(d)


def getNumDecoded(decoder):
    """ int getNumDecoded(LTdecoderFLY *decoder) """
    return libLT.getNumDecoded(decoder)


def getNumReceived(decoder):
    """ int getNumReceived(LTdecoderFLY *decoder) """
    return libLT.getNumReceived(decoder)


def addBucketFLY(d):
    """ int addBucketFLY(LTdecoderFLY* decoder) """
    return libLT.addBucketFLY(d)


def addAndInitBucketFLY_LT(d,seed,symblen,w,s,delta,c,epsShok,code):
    """
    int addAndInitBucketFLY_LT(LTdecoderFLY *decoder, unsigned long seed,
		int symblen, unsigned int w, unsigned int s,
		double lt_delta, double lt_c)
    """
    return libLT.addAndInitBucketFLY(
        d,
        c_ulong(seed),
        c_int(symblen),
        c_uint(w),
        c_uint(s),
        c_double(delta),
        c_double(c),
        epsShok,code)


def initBucketFLY(b,seed,symblen,w,s,lt_delta,lt_c,epsShok,code):
    """
    void initBucketFLY(BucketFLY *bucket, unsigned long seed, int symblen,
         unsigned int w, unsigned int s,
         double lt_delta, double lt_c, double epsShok, DegDistr code)
    """
    libLT.initBucketFLY(b,
        c_ulong(seed),
        c_int(symblen),
        c_uint(w),
        c_uint(s),
        c_double(lt_delta),
        c_double(lt_c),
        c_double(epsShok),
        c_int(code))


def rmBucketFLY(d,node_p):
    """ void rmBucketFLY(LTdecoderFLY **decoder, Node *node) """
    libLT.rmBucketFLY(d,node_p)


def getBucketFLY(d,nameBucketInt):
    """ BucketFLY *getBucketFLY(LTdecoderFLY *decoder, int nameBucket) """
    return libLT.getBucketFLY(d,c_int(nameBucketInt))


def getNodeBucketFLY(d,nameBucketInt):
    """ Node *getNodeBucketFLY(LTdecoderFLY *decoder, int nameBucket) """
    return libLT.getNodeBucketFLY(d,c_int(nameBucketInt))


def getNameBucketFLY(d,seed,w,s,lt_delta,lt_c,epsShok):
    """
    int getNameBucketFLY(LTdecoderFLY *decoder, unsigned long seed, unsigned int w, unsigned int s,
		double lt_delta, double lt_c, double epsShok);
    """
    return libLT.getNameBucketFLY(d,
        c_ulong(seed),c_uint(w),c_uint(s),
        c_double(lt_delta),c_double(lt_c),c_double(epsShok))


def createHeadersFLY(b,newHeadersToGenerate,offset):
    """
    void createHeadersFly(BucketFLY *bucket,
         unsigned int newHeadersToGenerate,
         unsigned int offset);	// first symbol ID of the window
    """
    libLT.createHeadersFLY(b,c_uint(newHeadersToGenerate),c_uint(offset))


def receiveSymbFLY(d,b,encodedInfo,encodedInfoLen,IDfirstSymb,offset):
    """
    unsigned int receiveSymbFLY(LTdecoderFLY *decoder, BucketFLY *bucket,
         char *encodedInfo, unsigned int encodedInfoLength,
         unsigned int IDfirstSymb, unsigned long offset);
    """
    return libLT.receiveSymbFLY(d,b,encodedInfo,encodedInfoLength,c_uint(IDfirstSymb),c_ulong(offset))

def receiveSymbFLY_BucketName(decoder_p,
                              bucketName,
                              encodedInfo,
                              encodedInfoLength,
                              IDfirstSymb,
                              offset):
    """ unsigned int receiveSymbFLY_BucketName(LTdecoderFLY *decoder, int bucketName,char *encodedInfo, unsigned int encodedInfoLength,unsigned int IDfirstSymb, unsigned long offset);"""
    return libLT.receiveSymbFLY_BucketName(decoder_p,
                                           bucketName,
                                           encodedInfo,
                                           c_uint(encodedInfoLength),
                                           c_uint(IDfirstSymb),
                                           c_ulong(offset))

def receiveEncodedSymbFLY(decoder_p,encodedsymbs,numberencodedsymbs):
    """
    unsigned int receiveEncodedSymbFLY(LTdecoderFLY *decoder, Symbol* encodedSymbols,
            unsigned int numberEncodedSymbols);
    """
    return libLT.receiveEncodedSymbFLY(decoder_p,encodedsymbs,numberencodedsymbs)

def countUndecSymbInWindow(decoded,startIndex,winLengthByte,symbLen):
    """
    unsigned int countUndecSymbInWindow(char *decoded, unsigned int startIndex,
        unsigned int winLengthByte, int symbLen);
    """
    return libLT.countUndecSymbInWindow(decoded,c_uint(startIndex),
        c_uint(winLengthByte),c_int(symbLen))

def addAndInitBucketFLY_LT(d,seed,symblen,w,s,lt_delta,lt_c):
    """
    int addAndInitBucketFLY_LT(LTdecoderFLY *decoder, unsigned long seed,
         int symblen, unsigned int w, unsigned int s,
         double lt_delta, double lt_c);
    """
    return libLT.addAndInitBucketFLY_LT(d,
        c_ulong(seed),
        c_int(symblen),
        c_uint(w),
        c_uint(s),
        c_double(lt_delta),
        c_double(lt_c))




######### DECODER }}} ##############
####################################

def LTdecodeFLY_fromFile(decoder,infile):
    return libLT.LTdecodeFLY_fromFile(decoder,infile)

def LTdecodeFLY_fromFiles(decoder, infiles):
    arr = (c_char_p * len(infiles))()
    for i in xrange(len(infiles)):
        arr[i] = infiles[i]
    return libLT.LTdecodeFLY_fromFiles(decoder, len(infiles), arr)

####################################
######### {{{ CIOFILE ##############

def readFile(filename):
    """
    char *readFile(char* infile, unsigned long long* infilelen) 
    
    @return a tuple composed by the pointer to the C string
    that is the file content and is length
    """
    filelenbytes = c_ulonglong()
    filecontent = libLT.readFile(filename,byref(filelenbytes))
    return (filecontent,filelenbytes)

def readFileChunk(filename,startpos,chunklen):
    """ char *readFileChunk(char* infile, unsigned long long startpos, unsigned long long chunklen) """
    return libLT.readFileChunk(filename,
                               c_ulonglong(startpos),
                               c_ulonglong(chunklen))

def readSetOfChunksFromFile(filename,numchunks,chunkpositions,chunklen):
    """
    char *readSetOfChunksFromFile(char* infile, unsigned int numchuncks,
                                  unsigned long long* chunkpositions, 
                                  unsigned long long chunklen);
    """
    return libLT.readSetOfChunksFromFile(filename,
                                         c_uint(numchunks),
                                         chunkpositions,
                                         c_ulonglong(chunklen))

def getFileSize(filename):
    """ unsigned long long getFileSize(char* infile) """
    return libLT.getFileSize(filename)

def writeFile(outfilename,buff,bufflen):
    """ unsigned long long writeFile(char* outfile, char* buffer, unsigned long long bufflen) """
    return libLT.writeFile(outfilename,
                           buff,
                           c_ulonglong(bufflen))

######### CIOFILE }}} ##############
####################################

####################################
############# {{{ TLV ##############

# Serialization
def serializeSymbol(encodedsymbol,version,pTlvData):
    """ unsigned int serializeSymbol(Symbol* encodedSymbol, char version, char** pTlvData) """
    return libLT.serializeSymbol(encodedsymbol,version,pTlvData)

def serializeSymbolList_explicit(encodedSymbols,N,pTlvData):
    """  unsigned long long serializeSymbolList_explicit(Symbol* encodedSymbols, unsigned int N, char** pTlvData) """
    return libLT.serializeSymbolList_explicit(encodedSymbols,N,pTlvData)

def serializeSymbolList_explicit_append(encodedSymbols,N,pTlvData):
    """ unsigned long serializeSymbolList_explicit_append(Symbol* encodedSymbols, unsigned int N, char** pTlvData) """
    return libLT.serializeSymbolList_explicit_append(encodedSymbols,N,pTlvData)

# FIXME: prende un numero variabile di argomenti
def serializeSymbolList_implicit(encodedSymbols,N,pTlvData):
    """ unsigned long serializeSymbolList_implicit(Symbol* encodedSymbols, unsigned int N, char** pTlvData, ...) """
    return libLT.serializeSymbolList_implicit(encodedSymbols,N,pTlvData)

def serializeSymbolList_implicit_append(encodedsymbols,N,pTlvData): 
    """ unsigned long serializeSymbolList_implicit_append(Symbol* encodedSymbols, unsigned int N, char** pTlvData) """
    return libLT.serializeSymbolList_implicit_append(encodedsymbols,c_uint(N),pTlvData)

# deserialization


############# TLV }}} ##############
####################################

def deserializeSymbol(tlvData,startPos,encodedSymbol):
    """ unsigned long deserializeSymbol(char* tlvData,
                                        unsigned long long startPos,
                                        Symbol* encodedSymbol);
    """
    return libLT.deserializeSymbol(tlvData,
                                   c_ulonglong(startPos),
                                   encodedSymbol)

def deserializeSymbolList(tlvData,pEncodedSymbolList,N):
    """ void deserializeSymbolList(char* tlvData,
                                   Symbol** pEncodedSymbolList,
                                   unsigned int* N);
    """
    return libLT.deserializeSymbolList(tlvData,
                                       pEncodedSymbolList,
                                       N)



if __name__=="__main__":
    import sys
    if len(sys.argv) < 2:
        print "Usage: %s <to_encode>" % sys.argv[0]
        sys.exit(0)
    elif sys.argv[1] == "-file":
        if len(sys.argv) < 3:
            print "Usage: %s -file <file_name>"
            sys.exit(0)
        f = open(sys.argv[2])
        content = f.read()
        f.close()
    else:
        content = sys.argv[1]

#    print 'len(input):%d input:%s' % (strwrap.slen,strwrap.s)
    print 'len(content):%d' % len(content)

    ########## Configuration ###############
    symblen = 1      # num of bytes per symbol
    k = len(content) # num of input symbols
    w = k            # num of symbols for window
    s = w            # num of symbols for movements
    N = 2*k

    # LT coding parameters
    delta = 0.5
    c = 0.4

    seed = 663       # random seed for encoding process
    ########## /Configuration ##############

    # Encode
    enc = mallocEncoderFLY(seed)
    # setEncoderFLY_LT(encoder_p,w,s,lt_delta,lt_c):
    setEncoderFLY_LT(enc,w,s,delta,c)
    encinfo,_ = LTencodeFLY(enc,symblen,content,0,N)
    #print 'len(encoded):%d encoded:%s' % (len(encinfo),encinfo)
#    print 'len(encoded):%d' % len(encinfo)

    # Decode
    dec = mallocDecoderFLY(N)
    buckname = addAndInitBucketFLY_LT(dec,seed,symblen,w,s,delta,c)
    receiveSymbFLY_BucketName(dec,buckname,encinfo,0,0)
    decodingProcessFLY(dec)

    numRec = getNumReceived(dec)
    numDec = getNumDecoded(dec)
    decoderRes = getDecodedInformation(dec)
    dec_buff = c_char_p(decoderRes)

    print 'received:%d numdecoded:%s len(decoded):%d source==decoded:%s' % \
        (numRec,numDec,len(dec_buff.value),content[0:numDec] == dec_buff.value[0:numDec])

    print '\n'+'-'*10+'Input   content'+'-'*10+'\n'+content[0:numDec]+'\n'+'-'*35
    print '\n'+'-'*10+'Decoded content'+'-'*10+'\n'+dec_buff.value[0:numDec]+'\n'+'-'*35
