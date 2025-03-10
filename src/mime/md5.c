/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2025 YAM Open Source Team

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
 YAM OpenSource project    :  https://github.com/jens-maus/yam/

***************************************************************************/

#include <string.h>
#include <stdio.h>

#include "mime/md5.h"

#include "SDI_compiler.h"

/***************************************************************************

 Original file description of md5 calculation part

 This code implements the MD5 message-digest algorithm.
 The algorithm is due to Ron Rivest.  This code was
 written by Colin Plumb in 1993, no copyright is claimed.
 This code is in the public domain; do with it what you wish.

 Equivalent code is available from RSA Data Security, Inc.
 This code has been tested against that, and is equivalent,
 except that you don't need to include two pages of legalese
 with every copy.

 To compute the message digest of a chunk of bytes, declare an
 MD5Context structure, pass it to md5init, call md5update as
 needed on buffers full of bytes, and then call md5final, which
 will fill a supplied 16-byte array with the digest.

***************************************************************************/

/* The four core functions - F1 is optimized somewhat */

/* #define F1(x, y, z) (x & y | ~x & z) */
#define F1(x, y, z) (z ^ (x & (y ^ z)))
#define F2(x, y, z) F1(z, x, y)
#define F3(x, y, z) (x ^ y ^ z)
#define F4(x, y, z) (y ^ (x | ~z))

/* This is the central step in the MD5 algorithm. */
#define MD5STEP(f, w, x, y, z, data, s) \
        ( w += f(x, y, z) + data,  w = w<<s | w>>(32-s),  w += x )

/**** MD5 message-digest routines ***/
/// byteReverse()
INLINE void byteReverse(unsigned char *buf, unsigned int longs)
{
  unsigned long t;

  do
  {
    t = (unsigned long)((unsigned)buf[3]<<8 | buf[2]) << 16 | ((unsigned)buf[1]<<8 | buf[0]);
    *(unsigned long *)buf = t;
    buf += 4;
  }
  while(--longs);
}
///
/// md5transform()
/*
 * The core of the MD5 algorithm, this alters an existing MD5 hash to
 * reflect the addition of 16 longwords of new data.  md5update blocks
 * the data and converts bytes into longwords for this routine.
 */
INLINE void md5transform(unsigned long buf[4], unsigned long const in[16])
{
  register unsigned long a, b, c, d;

  a = buf[0];
  b = buf[1];
  c = buf[2];
  d = buf[3];

  MD5STEP(F1, a, b, c, d, in[ 0]+0xd76aa478,  7);
  MD5STEP(F1, d, a, b, c, in[ 1]+0xe8c7b756, 12);
  MD5STEP(F1, c, d, a, b, in[ 2]+0x242070db, 17);
  MD5STEP(F1, b, c, d, a, in[ 3]+0xc1bdceee, 22);
  MD5STEP(F1, a, b, c, d, in[ 4]+0xf57c0faf,  7);
  MD5STEP(F1, d, a, b, c, in[ 5]+0x4787c62a, 12);
  MD5STEP(F1, c, d, a, b, in[ 6]+0xa8304613, 17);
  MD5STEP(F1, b, c, d, a, in[ 7]+0xfd469501, 22);
  MD5STEP(F1, a, b, c, d, in[ 8]+0x698098d8,  7);
  MD5STEP(F1, d, a, b, c, in[ 9]+0x8b44f7af, 12);
  MD5STEP(F1, c, d, a, b, in[10]+0xffff5bb1, 17);
  MD5STEP(F1, b, c, d, a, in[11]+0x895cd7be, 22);
  MD5STEP(F1, a, b, c, d, in[12]+0x6b901122,  7);
  MD5STEP(F1, d, a, b, c, in[13]+0xfd987193, 12);
  MD5STEP(F1, c, d, a, b, in[14]+0xa679438e, 17);
  MD5STEP(F1, b, c, d, a, in[15]+0x49b40821, 22);

  MD5STEP(F2, a, b, c, d, in[ 1]+0xf61e2562,  5);
  MD5STEP(F2, d, a, b, c, in[ 6]+0xc040b340,  9);
  MD5STEP(F2, c, d, a, b, in[11]+0x265e5a51, 14);
  MD5STEP(F2, b, c, d, a, in[ 0]+0xe9b6c7aa, 20);
  MD5STEP(F2, a, b, c, d, in[ 5]+0xd62f105d,  5);
  MD5STEP(F2, d, a, b, c, in[10]+0x02441453,  9);
  MD5STEP(F2, c, d, a, b, in[15]+0xd8a1e681, 14);
  MD5STEP(F2, b, c, d, a, in[ 4]+0xe7d3fbc8, 20);
  MD5STEP(F2, a, b, c, d, in[ 9]+0x21e1cde6,  5);
  MD5STEP(F2, d, a, b, c, in[14]+0xc33707d6,  9);
  MD5STEP(F2, c, d, a, b, in[ 3]+0xf4d50d87, 14);
  MD5STEP(F2, b, c, d, a, in[ 8]+0x455a14ed, 20);
  MD5STEP(F2, a, b, c, d, in[13]+0xa9e3e905,  5);
  MD5STEP(F2, d, a, b, c, in[ 2]+0xfcefa3f8,  9);
  MD5STEP(F2, c, d, a, b, in[ 7]+0x676f02d9, 14);
  MD5STEP(F2, b, c, d, a, in[12]+0x8d2a4c8a, 20);

  MD5STEP(F3, a, b, c, d, in[ 5]+0xfffa3942,  4);
  MD5STEP(F3, d, a, b, c, in[ 8]+0x8771f681, 11);
  MD5STEP(F3, c, d, a, b, in[11]+0x6d9d6122, 16);
  MD5STEP(F3, b, c, d, a, in[14]+0xfde5380c, 23);
  MD5STEP(F3, a, b, c, d, in[ 1]+0xa4beea44,  4);
  MD5STEP(F3, d, a, b, c, in[ 4]+0x4bdecfa9, 11);
  MD5STEP(F3, c, d, a, b, in[ 7]+0xf6bb4b60, 16);
  MD5STEP(F3, b, c, d, a, in[10]+0xbebfbc70, 23);
  MD5STEP(F3, a, b, c, d, in[13]+0x289b7ec6,  4);
  MD5STEP(F3, d, a, b, c, in[ 0]+0xeaa127fa, 11);
  MD5STEP(F3, c, d, a, b, in[ 3]+0xd4ef3085, 16);
  MD5STEP(F3, b, c, d, a, in[ 6]+0x04881d05, 23);
  MD5STEP(F3, a, b, c, d, in[ 9]+0xd9d4d039,  4);
  MD5STEP(F3, d, a, b, c, in[12]+0xe6db99e5, 11);
  MD5STEP(F3, c, d, a, b, in[15]+0x1fa27cf8, 16);
  MD5STEP(F3, b, c, d, a, in[ 2]+0xc4ac5665, 23);

  MD5STEP(F4, a, b, c, d, in[ 0]+0xf4292244,  6);
  MD5STEP(F4, d, a, b, c, in[ 7]+0x432aff97, 10);
  MD5STEP(F4, c, d, a, b, in[14]+0xab9423a7, 15);
  MD5STEP(F4, b, c, d, a, in[ 5]+0xfc93a039, 21);
  MD5STEP(F4, a, b, c, d, in[12]+0x655b59c3,  6);
  MD5STEP(F4, d, a, b, c, in[ 3]+0x8f0ccc92, 10);
  MD5STEP(F4, c, d, a, b, in[10]+0xffeff47d, 15);
  MD5STEP(F4, b, c, d, a, in[ 1]+0x85845dd1, 21);
  MD5STEP(F4, a, b, c, d, in[ 8]+0x6fa87e4f,  6);
  MD5STEP(F4, d, a, b, c, in[15]+0xfe2ce6e0, 10);
  MD5STEP(F4, c, d, a, b, in[ 6]+0xa3014314, 15);
  MD5STEP(F4, b, c, d, a, in[13]+0x4e0811a1, 21);
  MD5STEP(F4, a, b, c, d, in[ 4]+0xf7537e82,  6);
  MD5STEP(F4, d, a, b, c, in[11]+0xbd3af235, 10);
  MD5STEP(F4, c, d, a, b, in[ 2]+0x2ad7d2bb, 15);
  MD5STEP(F4, b, c, d, a, in[ 9]+0xeb86d391, 21);

  buf[0] += a;
  buf[1] += b;
  buf[2] += c;
  buf[3] += d;
}
///
/// md5init()
/*
 * Start MD5 accumulation.  Set bit count to 0 and buffer to mysterious
 * initialization constants.
 */
void md5init(struct MD5Context *ctx)
{
  ctx->state[0] = 0x67452301;
  ctx->state[1] = 0xefcdab89;
  ctx->state[2] = 0x98badcfe;
  ctx->state[3] = 0x10325476;

  ctx->count[0] = 0;
  ctx->count[1] = 0;
}
///
/// md5update()
/*
 * Update context to reflect the concatenation of another buffer full
 * of bytes.
 */
void md5update(struct MD5Context *ctx, const void *buf, unsigned int len)
{
  const char *cbuf = buf;
  unsigned int t;

  /* Update bitcount */
  t = ctx->count[0];
  if((ctx->count[0] = t + ((unsigned long)len << 3)) < t)
     ctx->count[1]++; /* Carry from low to high */
  ctx->count[1] += len >> 29;

  t = (t >> 3) & 0x3f;    /* Bytes already in shsInfo->data */

  /* Handle any leading odd-sized chunks */
  if(t)
  {
    unsigned char *p = &ctx->buffer.u8[t];

    t = 64-t;
    if(len < t)
    {
       memcpy(p, cbuf, len);
       return;
    }
    memcpy(p, cbuf, t);
    byteReverse(ctx->buffer.u8, 16);
    md5transform(ctx->state, ctx->buffer.u32);
    cbuf += t;
    len -= t;
  }

  /* Process data in 64-byte chunks */
  while(len >= 64)
  {
    memcpy(ctx->buffer.u8, cbuf, 64);
    byteReverse(ctx->buffer.u8, 16);
    md5transform(ctx->state, ctx->buffer.u32);
    cbuf += 64;
    len -= 64;
  }

  /* Handle any remaining bytes of data. */
  memcpy(ctx->buffer.u8, cbuf, len);
}
///
/// md5final()
/*
 * Final wrapup - pad to 64-byte boundary with the bit pattern
 * 1 0* (64-bit count of bits processed, MSB-first)
 */
void md5final(unsigned char digest[16], struct MD5Context *ctx)
{
  unsigned count;
  unsigned char *p;

  /* Compute number of bytes mod 64 */
  count = (ctx->count[0] >> 3) & 0x3F;

  /* Set the first char of padding to 0x80.  This is safe since there is
     always at least one byte free */
  p = &ctx->buffer.u8[count];
  *p++ = 0x80;

  /* Bytes of padding needed to make 64 bytes */
  count = 64 - 1 - count;

  /* Pad out to 56 mod 64 */
  if(count < 8)
  {
     /* Two lots of padding:  Pad the first block to 64 bytes */
     memset(p, 0, count);
     byteReverse(ctx->buffer.u8, 16);
     md5transform(ctx->state, ctx->buffer.u32);

     /* Now fill the next block with 56 bytes */
     memset(ctx->buffer.u8, 0, 56);
  }
  else
  {
     /* Pad block to 56 bytes */
     memset(p, 0, count-8);
  }
  byteReverse(ctx->buffer.u8, 14);

  /* Append length in bits and transform */
  ctx->buffer.u32[14] = ctx->count[0];
  ctx->buffer.u32[15] = ctx->count[1];

  md5transform(ctx->state, ctx->buffer.u32);
  byteReverse((unsigned char *)ctx->state, 4);
  memcpy(digest, ctx->state, 16);
  memset(ctx, 0, sizeof(*ctx));    /* In case it's sensitive */
}
///
/// md5hmac()
// text     pointer to data stream
// text_len length of data stream
// key      pointer to authentication key
// key_len  length of authentication key
// digest   caller digest to be filled in
void md5hmac(unsigned char * text, int text_len, unsigned char *key, int key_len, unsigned char digest[16])
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

    md5init(&tctx);
    md5update(&tctx, key, key_len);
    md5final(tk, &tctx);

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
  memcpy(k_ipad, key, key_len);
  memcpy(k_opad, key, key_len);

  /* XOR key with ipad and opad values */
  for (i=0; i<64; i++)
  {
     k_ipad[i] ^= 0x36;
     k_opad[i] ^= 0x5c;
  }
  /* perform inner MD5 */
  md5init(&context);                   /* init context for 1st pass */
  md5update(&context, k_ipad, 64);     /* start with inner pad */
  md5update(&context, text, text_len); /* then text of datagram */
  md5final(digest, &context);          /* finish up 1st pass */

  /* perform outer MD5 */
  md5init(&context);                   /* init context for 2nd pass */
  md5update(&context, k_opad, 64);     /* start with outer pad */
  md5update(&context, digest, 16);     /* then results of 1st hash */
  md5final(digest, &context);          /* finish up 2nd pass */
}
///
/// md5digestToHex
// convert an MD5 digest into a NUL-terminated hexdump for transmission
// NOTE: the hex buffer must have a size of at least 33 bytes!
void md5digestToHex(const unsigned char digest[16], char *hex)
{
  int i;

  for(i=0; i<16; i++)
    snprintf(&hex[i*2], 3, "%02x", digest[i]);
}

///
