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
#include <string.h>
#include "xor.h"

#define MMX_FLAG  0x00800000
#define SSE2_FLAG 0x04000000

//#define non_x86

XorType isSIMD(void) {

    unsigned int cpuflag  = 0x00000000;
    XorType ret = NO;

#ifndef non_x86

    asm volatile(
        "cpuid"
    : "=d"(cpuflag) : "a"(1)
            );

#endif

    if (cpuflag & MMX_FLAG) {
        ret=MMX;
    }

    if (cpuflag & SSE2_FLAG) {
        ret=SSE2;
    }

    printf(" ");

    return ret;
}

void calcXOR(void *src, void *dst, size_t n, XorType type) {

    unsigned int long_size, op_cnt;
    unsigned long t;
    char *src_p, *dst_p;
    unsigned long *src_lp, *dst_lp;
    void *aligned_dst;
    unsigned long gap;

    src_p=(char *)(src);
    dst_p=(char *)(dst);

#ifndef non_x86

    if (n>=32 && type==SSE2) {
        // SSE2$BL?Na$rMQ$$$F(B128bit$B$:$D(BXOR$B$r7W;;$9$k(B

        //printf("Using SSE2.\n");

        aligned_dst=(void *)(((unsigned int)dst+(16-1))&~(16-1));
        gap=(unsigned int)aligned_dst - (unsigned int)dst;

        //printf("%x, %x, %d\n", aligned_dst, dst, gap);

        for (t=0; t<gap; t++) {
            dst_p[t] ^= src_p[t];
        }

        op_cnt=(int)((n-gap)/16);

        asm volatile("\n"
                     "    mov      %2, %%eax\n"
                     "  calc1:\n"
                     "    movdqu   (%%esi),%%xmm0\n"
                     "    movdqa   (%%edi),%%xmm1\n"
                     "    pxor     %%xmm1,%%xmm0\n"
                     "    movdqa   %%xmm0,(%%edi)\n"

                     "    sub      $1, %%eax\n"
                     "    cmp      $0, %%eax\n"
                     "    je       end1\n"

                     "    add      $16, %%esi\n"
                     "    add      $16, %%edi\n"
                     "    jmp      calc1\n"

                     "  end1:\n"
                     "    emms" :: "S"((unsigned int)src+gap), "D"((unsigned int)dst+gap), "r"(op_cnt) : "%eax"
                            );

        // $B;D$C$?J,$r(B1$B%P%$%H$:$D7W;;(B
        for (t=gap+op_cnt*16; t<n; t++) {
            dst_p[t] ^= src_p[t];
        }

        return;
    }

    if (n>=8 && type==MMX) {

        // MMX$BL?Na$rMQ$$$F(B64bit$B$:$D(BXOR$B$r7W;;$9$k(B

        //printf("Using MMX.\n");

        op_cnt=(int)(n/8);

        asm volatile("\n"
                     "    mov    %2, %%eax\n"
                     "  calc2:\n"
                     "    movq   (%%esi),%%mm0\n"
                     "    movq   (%%edi),%%mm1\n"
                     "    pxor   %%mm1,%%mm0\n"
                     "    movq   %%mm0,(%%edi)\n"

                     "    sub    $1, %%eax\n"
                     "    cmp    $0, %%eax\n"
                     "    je     end2\n"

                     "    add    $8, %%esi\n"
                     "    add    $8, %%edi\n"
                     "    jmp    calc2\n"

                     "  end2:\n"
                     "    emms" :: "S"(src), "D"(dst), "r"(op_cnt) : "%eax"
                            );

        // $B;D$C$?J,$r(B1$B%P%$%H$:$D7W;;(B
        for (t=op_cnt*8; t<n; t++) {
            dst_p[t] ^= src_p[t];
        }

        return;
    }

#endif

    // long$B$G(BXOR$B$r7W;;$9$k(B

    // long$B$ND9$5$r<hF@(B
    long_size=sizeof(long);

    // pkt$B$H(Bparity$B$N@hF,%"%I%l%9$r(Blong$B$N%]%$%s%?$KJQ49(B
    src_lp=(unsigned long *)src_p;
    dst_lp=(unsigned long *)dst_p;

    // long$B$NC10L$G(BXOR$B$r7W;;(B
    op_cnt=(int)(n/long_size);
    for (t=0; t<op_cnt; t++) {
        *(dst_lp+t) ^= *(src_lp+t);
    }

    // $B;D$C$?J,$r(B1$B%P%$%H$:$D7W;;(B
    for (t=op_cnt*long_size; t<n; t++) {
        dst_p[t] ^= src_p[t];
    }
}



/*
  $B%"%;%s%V%i%3!<%I2r@b(B

  $B!&(BMMX

  asm volatile ("
      mov    %2, %%eax        // op_cnt$B$r(BEAX$B$KBeF~(B
    calc:
      movq   (%%esi),%%mm0    // src_lp$B$+$i(B64bit$BJ,$N%G!<%?$r(BMM0$B$KE>Aw(B(MMX$BL?Na(B)
      movq   (%%edi),%%mm1    // dst_lp$B$+$i(B64bit$BJ,$N%G!<%?$r(BMM1$B$KE>Aw(B(MMX$BL?Na(B)
      pxor   %%mm1,%%mm0      // MM1$B$H(BMM0$B$N(BXOR$B$r7W;;$9$k!#7k2L$O(BMM0$B!#(B(MMX$BL?Na(B)
      movq   %%mm0,(%%edi)    // MM0$B$NFbMF$r(BEDI(dst_lp)$B$KE>Aw(B        (MMX$BL?Na(B)

      sub    $1, %%eax        // EAX$B$+$i(B1$B$r0z$/(B
      cmp    $0, %%eax        // EAX$B$H(B0$B$rHf3S$7$F!"F1$8$J$i(BEFLAG?$B$,(BON$B$K$J$k(B
      je     end              // EFLAG$B$,(BON$B$J$i$P(Bend$B$K%8%c%s%W(B

      add    $8, %%esi        // ESI$B$N%"%I%l%9CM$K(B8$B$r2C;;$9$k(B
      add    $8, %%edi        // EDI$B$N%"%I%l%9CM$K(B8$B$r2C;;$9$k(B
      jmp    calc             // calc$B$K%8%c%s%W(B

    end:
      emms"
      :: "S" (src_p), "D" (dst_p), "r" (op_cnt)
      : "%eax"
  );


  $B!&(BSSE2

    MMX$B$H$[$H$s$IF1$8!#(BMMX$B%l%8%9%?(B(mm0..mm7)$B$NBe$o$j$K!"(B
    SSE$B%l%8%9%?(B(xmm0..xmm7)$B$r;H$&!#(Bpxor$B$O!";XDj$7$?%l%8%9%?$K$h$C$F(B
    $B<+F0E*$K(B64bit$B$+(B128bit$B$+$rH=JL$7$F7W;;$9$k$i$7$$!#(B

    asm volatile ("
        mov      %2, %%eax
      calc:
        movdqu   (%%esi),%%xmm0   // $B%G!<%?$r(B128bit$B0\F0!#(B
        movdqu   (%%edi),%%xmm1
        pxor     %%xmm1,%%xmm0    // SSE$B%l%8%9%?$N(BXOR$B$r7W;;(B
        movdqu   %%xmm0,(%%edi)

        sub      $1, %%eax
        cmp      $0, %%eax
        je       end

        add      $16, %%esi      // 16bytes(128bit)$BJ,%"%I%l%9$r2C;;(B
        add      $16, %%edi
        jmp      calc

      end:
        emms"
        :: "S" (src_p), "D" (dst_p), "r" (op_cnt)
        : "%eax"
    );


  $B!&%F%9%H7k2L(B

      30000bytes$B$N(BXOR$B$r(B200$BK|2s7W;;$7$?$H$3$m!"(B
        SSE2(128bit): 10$BIC(B
        MMX(64bit)  : 66$BIC(B
        long(32bit) : 146$BIC(B

      $B$H$$$&7k2L$,F@$i$l$?!#(B

*/
