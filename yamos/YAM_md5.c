/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2001 by YAM Open Source Team

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 YAM Official Support Site :  http://www.yam.ch
 YAM OpenSource project    :  http://sourceforge.net/projects/yamos/

 $Id$

***************************************************************************/

#include <string.h>
#include "SDI_compiler.h"
#include "YAM_md5.h"

/// MD5Init
void MD5Init(struct MD5Context *p)
{
   /* Load magic initialization constants. */
   p->state[0] = 0x67452301;
   p->state[1] = 0xefcdab89;
   p->state[2] = 0x98badcfe;
   p->state[3] = 0x10325476;

   /* Nothing counted, so count=0 */
   p->count[0] = 0;
   p->count[1] = 0;
}

///

#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

/// encode()
/* Encodes input (unsigned long) into output (unsigned char). Assumes len is
   a multiple of 4. */
INLINE void encode(unsigned char *output, unsigned long *input, unsigned long len)
{
   unsigned int i, j;

   for (i = 0, j = 0; j < len; i++, j += 4)
   {
      output[j]     = (unsigned char)  (input[i] & 0xff);
      output[j + 1] = (unsigned char) ((input[i] >> 8) & 0xff);
      output[j + 2] = (unsigned char) ((input[i] >> 16) & 0xff);
      output[j + 3] = (unsigned char) ((input[i] >> 24) & 0xff);
   }
}

///
/// decode()
/* Decodes input (unsigned char) into output (unsigned long). Assumes len is
   a multiple of 4. */
INLINE void decode(unsigned long *output, unsigned char *input, unsigned long len)
{
   unsigned int i, j;

   for (i = 0, j = 0; j < len; i++, j += 4)
      output[i] = ((unsigned long)input[j]) | (((unsigned long)input[j + 1]) << 8) |
  (((unsigned long)input[j + 2]) << 16) | (((unsigned long)input[j + 3]) << 24);
}

///
/// rotate_left()
INLINE unsigned int rotate_left(unsigned long x, unsigned long n)
{
   return (x << n) | (x >> (32 - n));
}

///
/// MD5_F/G/H/I
/* F, G, H and I are basic MD5 functions. */

INLINE unsigned int MD5_F(unsigned long x, unsigned long y, unsigned long z)
{
   return (x & y) | (~x & z);
}

INLINE unsigned int MD5_G(unsigned long x, unsigned long y, unsigned long z)
{
   return (x & z) | (y & ~z);
}

INLINE unsigned int MD5_H(unsigned long x, unsigned long y, unsigned long z)
{
   return x ^ y ^ z;
}

INLINE unsigned int MD5_I(unsigned long x, unsigned long y, unsigned long z)
{
   return y ^ (x | ~z);
}

///
/// MD5_FF/GG/HH/II
/* FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
   Rotation is separate from addition to prevent recomputation. */

INLINE void MD5_FF(unsigned long* a, unsigned long b, unsigned long c, unsigned long d, unsigned long x,
          unsigned long s, unsigned long ac)
{
   *a += MD5_F(b, c, d) + x + ac;
   *a = rotate_left(*a, s) + b;
}

INLINE void MD5_GG(unsigned long* a, unsigned long b, unsigned long c, unsigned long d, unsigned long x,
          unsigned long s, unsigned long ac)
{
   *a += MD5_G(b, c, d) + x + ac;
   *a = rotate_left(*a, s) + b;
}

INLINE void MD5_HH(unsigned long* a, unsigned long b, unsigned long c, unsigned long d, unsigned long x,
          unsigned long s, unsigned long ac)
{
   *a += MD5_H(b, c, d) + x + ac;
   *a = rotate_left(*a, s) + b;
}

INLINE void MD5_II(unsigned long* a, unsigned long b, unsigned long c, unsigned long d, unsigned long x,
          unsigned long s, unsigned long ac)
{
   *a += MD5_I(b, c, d) + x + ac;
   *a = rotate_left(*a, s) + b;
}

///
/// transform()
/* MD5 basic transformation. Transforms state based on block. */
static void transform(struct MD5Context *p, unsigned char block[64])
{
   unsigned long a = p->state[0], b = p->state[1], c = p->state[2], d = p->state[3], x[16];

   decode (x, block, 64);

   /* Round 1 */
   MD5_FF(&a, b, c, d, x[ 0], S11, 0xd76aa478); /*  1 */
   MD5_FF(&d, a, b, c, x[ 1], S12, 0xe8c7b756); /*  2 */
   MD5_FF(&c, d, a, b, x[ 2], S13, 0x242070db); /*  3 */
   MD5_FF(&b, c, d, a, x[ 3], S14, 0xc1bdceee); /*  4 */
   MD5_FF(&a, b, c, d, x[ 4], S11, 0xf57c0faf); /*  5 */
   MD5_FF(&d, a, b, c, x[ 5], S12, 0x4787c62a); /*  6 */
   MD5_FF(&c, d, a, b, x[ 6], S13, 0xa8304613); /*  7 */
   MD5_FF(&b, c, d, a, x[ 7], S14, 0xfd469501); /*  8 */
   MD5_FF(&a, b, c, d, x[ 8], S11, 0x698098d8); /*  9 */
   MD5_FF(&d, a, b, c, x[ 9], S12, 0x8b44f7af); /* 10 */
   MD5_FF(&c, d, a, b, x[10], S13, 0xffff5bb1); /* 11 */
   MD5_FF(&b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
   MD5_FF(&a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
   MD5_FF(&d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
   MD5_FF(&c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
   MD5_FF(&b, c, d, a, x[15], S14, 0x49b40821); /* 16 */

   /* Round 2 */
   MD5_GG(&a, b, c, d, x[ 1], S21, 0xf61e2562); /* 17 */
   MD5_GG(&d, a, b, c, x[ 6], S22, 0xc040b340); /* 18 */
   MD5_GG(&c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
   MD5_GG(&b, c, d, a, x[ 0], S24, 0xe9b6c7aa); /* 20 */
   MD5_GG(&a, b, c, d, x[ 5], S21, 0xd62f105d); /* 21 */
   MD5_GG(&d, a, b, c, x[10], S22,  0x2441453); /* 22 */
   MD5_GG(&c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
   MD5_GG(&b, c, d, a, x[ 4], S24, 0xe7d3fbc8); /* 24 */
   MD5_GG(&a, b, c, d, x[ 9], S21, 0x21e1cde6); /* 25 */
   MD5_GG(&d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
   MD5_GG(&c, d, a, b, x[ 3], S23, 0xf4d50d87); /* 27 */
   MD5_GG(&b, c, d, a, x[ 8], S24, 0x455a14ed); /* 28 */
   MD5_GG(&a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
   MD5_GG(&d, a, b, c, x[ 2], S22, 0xfcefa3f8); /* 30 */
   MD5_GG(&c, d, a, b, x[ 7], S23, 0x676f02d9); /* 31 */
   MD5_GG(&b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */

   /* Round 3 */
   MD5_HH(&a, b, c, d, x[ 5], S31, 0xfffa3942); /* 33 */
   MD5_HH(&d, a, b, c, x[ 8], S32, 0x8771f681); /* 34 */
   MD5_HH(&c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
   MD5_HH(&b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
   MD5_HH(&a, b, c, d, x[ 1], S31, 0xa4beea44); /* 37 */
   MD5_HH(&d, a, b, c, x[ 4], S32, 0x4bdecfa9); /* 38 */
   MD5_HH(&c, d, a, b, x[ 7], S33, 0xf6bb4b60); /* 39 */
   MD5_HH(&b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
   MD5_HH(&a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
   MD5_HH(&d, a, b, c, x[ 0], S32, 0xeaa127fa); /* 42 */
   MD5_HH(&c, d, a, b, x[ 3], S33, 0xd4ef3085); /* 43 */
   MD5_HH(&b, c, d, a, x[ 6], S34,  0x4881d05); /* 44 */
   MD5_HH(&a, b, c, d, x[ 9], S31, 0xd9d4d039); /* 45 */
   MD5_HH(&d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
   MD5_HH(&c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
   MD5_HH(&b, c, d, a, x[ 2], S34, 0xc4ac5665); /* 48 */

   /* Round 4 */
   MD5_II(&a, b, c, d, x[ 0], S41, 0xf4292244); /* 49 */
   MD5_II(&d, a, b, c, x[ 7], S42, 0x432aff97); /* 50 */
   MD5_II(&c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
   MD5_II(&b, c, d, a, x[ 5], S44, 0xfc93a039); /* 52 */
   MD5_II(&a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
   MD5_II(&d, a, b, c, x[ 3], S42, 0x8f0ccc92); /* 54 */
   MD5_II(&c, d, a, b, x[10], S43, 0xffeff47d); /* 55 */
   MD5_II(&b, c, d, a, x[ 1], S44, 0x85845dd1); /* 56 */
   MD5_II(&a, b, c, d, x[ 8], S41, 0x6fa87e4f); /* 57 */
   MD5_II(&d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
   MD5_II(&c, d, a, b, x[ 6], S43, 0xa3014314); /* 59 */
   MD5_II(&b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
   MD5_II(&a, b, c, d, x[ 4], S41, 0xf7537e82); /* 61 */
   MD5_II(&d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
   MD5_II(&c, d, a, b, x[ 2], S43, 0x2ad7d2bb); /* 63 */
   MD5_II(&b, c, d, a, x[ 9], S44, 0xeb86d391); /* 64 */

   p->state[0] += a;
   p->state[1] += b;
   p->state[2] += c;
   p->state[3] += d;

   /* Zeroize sensitive information. */
   memset ( (unsigned char *) x, 0, sizeof(x));
}

///
/// MD5Update()
void MD5Update(struct MD5Context *p, unsigned char *input, unsigned int input_length)
{
   unsigned long input_index, buffer_index;
   unsigned long buffer_space;                /* how much space is left in buffer */

   /* Compute number of bytes mod 64 */
   buffer_index = (unsigned int)((p->count[0] >> 3) & 0x3F);

   /* Update number of bits */
   if ((p->count[0] += ((unsigned long)input_length << 3))<((unsigned long)input_length << 3))
      p->count[1]++;

   p->count[1] += ((unsigned long)input_length >> 29);

   buffer_space = 64 - buffer_index;  /* how much space is left in buffer */

   /* Transform as many times as possible. */
   if (input_length >= buffer_space)
   {  /* ie. we have enough to fill the buffer */
      /* fill the rest of the buffer and transform */
      memcpy(p->buffer + buffer_index, input, buffer_space);
      transform(p, p->buffer);

      /* now, transform each 64-byte piece of the input, bypassing the buffer */
      for (input_index = buffer_space; input_index + 63 < input_length;
     input_index += 64)
   transform(p, input + input_index);

      buffer_index = 0;  /* so we can buffer remaining */
   }
   else
      input_index = 0;     /* so we can buffer the whole input */

   /* and here we do the buffering: */
   memcpy(p->buffer + buffer_index, input + input_index, input_length - input_index);
}

///
/// MD5Final()
void MD5Final(unsigned char digest[16], struct MD5Context *p)
{
   static unsigned char PADDING[64]={
      0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
   };
   unsigned int index, padLen;
   unsigned char bits[8];

   /* Save number of bits */
   encode(bits, p->count, 8);

   /* Pad out to 56 mod 64. */
   index = (unsigned long) ((p->count[0] >> 3) & 0x3f);
   padLen = (index < 56) ? (56 - index) : (120 - index);
   MD5Update(p, PADDING, padLen);

   /* Append length (before padding) */
   MD5Update(p, bits, 8);

   /* Store state in digest */
   encode(digest, p->state, 16);

   /* Zeroize sensitive information */
   memset(p->buffer, 0, sizeof(p->buffer));
}

///
/// hmac_md5()
void hmac_md5(unsigned char * text, int text_len, unsigned char *key, int key_len, unsigned char digest[16])
/* text     pointer to data stream */
/* text_len length of data stream */
/* key      pointer to authentication key */
/* key_len  length of authentication key */
/* digest   caller digest to be filled in */
{
   struct MD5Context context;
   unsigned char k_ipad[65];    /* inner padding - key XORd with ipad */
   unsigned char k_opad[65];    /* outer padding - key XORd with opad */
   unsigned char tk[16];
   int i;
   /* if key is longer than 64 bytes reset it to key=MD5(key) */
   if(key_len > 64)
   {
      struct MD5Context tctx;

      MD5Init(&tctx);
      MD5Update(&tctx, key, key_len);
      MD5Final(tk, &tctx);

      key = tk;
      key_len = 16;
   }

   /*
    * the HMAC_MD5 transform looks like:
    *
    * MD5(K XOR opad, MD5(K XOR ipad, text))
    *
    * where K is an n byte key
    * ipad is the byte 0x36 repeated 64 times
    * opad is the byte 0x5c repeated 64 times
    * and text is the data being protected
    */

    /* start out by storing key in pads */
    memset(k_ipad, 0, sizeof(k_ipad));
    memset(k_opad, 0, sizeof(k_opad));
    memcpy(key, k_ipad, key_len);
    memcpy(key, k_opad, key_len);

    /* XOR key with ipad and opad values */
    for (i=0; i<64; i++)
    {
       k_ipad[i] ^= 0x36;
       k_opad[i] ^= 0x5c;
    }
    /* perform inner MD5 */
    MD5Init(&context);                   /* init context for 1st pass */
    MD5Update(&context, k_ipad, 64);     /* start with inner pad */
    MD5Update(&context, text, text_len); /* then text of datagram */
    MD5Final(digest, &context);          /* finish up 1st pass */

    /* perform outer MD5 */
    MD5Init(&context);                   /* init context for 2nd pass */
    MD5Update(&context, k_opad, 64);     /* start with outer pad */
    MD5Update(&context, digest, 16);     /* then results of 1st hash */
    MD5Final(digest, &context);          /* finish up 2nd pass */
}
///
