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
        // SSE2命令を用いて128bitずつXORを計算する

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

        // 残った分を1バイトずつ計算
        for (t=gap+op_cnt*16; t<n; t++) {
            dst_p[t] ^= src_p[t];
        }

        return;
    }

    if (n>=8 && type==MMX) {

        // MMX命令を用いて64bitずつXORを計算する

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

        // 残った分を1バイトずつ計算
        for (t=op_cnt*8; t<n; t++) {
            dst_p[t] ^= src_p[t];
        }

        return;
    }

#endif

    // longでXORを計算する

    // longの長さを取得
    long_size=sizeof(long);

    // pktとparityの先頭アドレスをlongのポインタに変換
    src_lp=(unsigned long *)src_p;
    dst_lp=(unsigned long *)dst_p;

    // longの単位でXORを計算
    op_cnt=(int)(n/long_size);
    for (t=0; t<op_cnt; t++) {
        *(dst_lp+t) ^= *(src_lp+t);
    }

    // 残った分を1バイトずつ計算
    for (t=op_cnt*long_size; t<n; t++) {
        dst_p[t] ^= src_p[t];
    }
}



/*
  アセンブラコード解説

  ・MMX

  asm volatile ("
      mov    %2, %%eax        // op_cntをEAXに代入
    calc:
      movq   (%%esi),%%mm0    // src_lpから64bit分のデータをMM0に転送(MMX命令)
      movq   (%%edi),%%mm1    // dst_lpから64bit分のデータをMM1に転送(MMX命令)
      pxor   %%mm1,%%mm0      // MM1とMM0のXORを計算する。結果はMM0。(MMX命令)
      movq   %%mm0,(%%edi)    // MM0の内容をEDI(dst_lp)に転送        (MMX命令)

      sub    $1, %%eax        // EAXから1を引く
      cmp    $0, %%eax        // EAXと0を比較して、同じならEFLAG?がONになる
      je     end              // EFLAGがONならばendにジャンプ

      add    $8, %%esi        // ESIのアドレス値に8を加算する
      add    $8, %%edi        // EDIのアドレス値に8を加算する
      jmp    calc             // calcにジャンプ

    end:
      emms"
      :: "S" (src_p), "D" (dst_p), "r" (op_cnt)
      : "%eax"
  );


  ・SSE2

    MMXとほとんど同じ。MMXレジスタ(mm0..mm7)の代わりに、
    SSEレジスタ(xmm0..xmm7)を使う。pxorは、指定したレジスタによって
    自動的に64bitか128bitかを判別して計算するらしい。

    asm volatile ("
        mov      %2, %%eax
      calc:
        movdqu   (%%esi),%%xmm0   // データを128bit移動。
        movdqu   (%%edi),%%xmm1
        pxor     %%xmm1,%%xmm0    // SSEレジスタのXORを計算
        movdqu   %%xmm0,(%%edi)

        sub      $1, %%eax
        cmp      $0, %%eax
        je       end

        add      $16, %%esi      // 16bytes(128bit)分アドレスを加算
        add      $16, %%edi
        jmp      calc

      end:
        emms"
        :: "S" (src_p), "D" (dst_p), "r" (op_cnt)
        : "%eax"
    );


  ・テスト結果

      30000bytesのXORを200万回計算したところ、
        SSE2(128bit): 10秒
        MMX(64bit)  : 66秒
        long(32bit) : 146秒

      という結果が得られた。

*/
