#include <string.h>
#include <ctype.h>
#define DoMethod _DoMethod
#include <clib/alib_protos.h>
#undef DoMethod
#include <dos/dos.h>
#include <proto/dos.h>
#include "YAM.h"

/* Not completely equivalent to the SAS/C version, as the returned value
   is not the same. But YAM only uses the return value as a boolean. */
int astcsma(const char *s, const char *p)
{
   char buf[256];
   if (ParsePatternNoCase(p, buf, sizeof(buf)) >= 0) {
      return MatchPatternNoCase(buf, s);
   }
}

int max(int x, int y)
{
   return x > y ? x : y;
}

size_t StrLen(const char *str)
{
   return strlen(str);
}

ULONG DoMethod(void *obj,ULONG a1,ULONG a2,ULONG a3,ULONG a4,ULONG a5,ULONG a6,ULONG a7,ULONG a8,ULONG a9,ULONG a10,ULONG a11,ULONG a12,ULONG a13,ULONG a14,ULONG a15)
{
    ULONG t[15];
    t[0]=a1;
    t[1]=a2;
    t[2]=a3;
    t[3]=a4;
    t[4]=a5;
    t[5]=a6;
    t[6]=a7;
    t[7]=a8;
    t[8]=a9;
    t[9]=a10;
    t[10]=a11;
    t[11]=a12;
    t[12]=a13;
    t[13]=a14;
    t[14]=a15;
    return DoMethodA(obj,(Msg)t);
}

void MD5Init(MD5_CTX *p)
{
   // Nothing counted, so count=0
   p->count[0] = 0;
   p->count[1] = 0;

   // Load magic initialization constants.
   p->state[0] = 0x67452301;
   p->state[1] = 0xefcdab89;
   p->state[2] = 0x98badcfe;
   p->state[3] = 0x10325476;
}

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

typedef unsigned char uint1;
typedef UINT4 uint4;

// Encodes input (UINT4) into output (unsigned char). Assumes len is
// a multiple of 4.
static void encode(uint1 *output, uint4 *input, uint4 len)
{
   unsigned int i, j;

   for (i = 0, j = 0; j < len; i++, j += 4)
   {
      output[j]     = (uint1)  (input[i] & 0xff);
      output[j + 1] = (uint1) ((input[i] >> 8) & 0xff);
      output[j + 2] = (uint1) ((input[i] >> 16) & 0xff);
      output[j + 3] = (uint1) ((input[i] >> 24) & 0xff);
   }
}

// Decodes input (unsigned char) into output (UINT4). Assumes len is
// a multiple of 4.
static void decode(uint4 *output, uint1 *input, uint4 len)
{
   unsigned int i, j;

   for (i = 0, j = 0; j < len; i++, j += 4)
      output[i] = ((uint4)input[j]) | (((uint4)input[j + 1]) << 8) |
	(((uint4)input[j + 2]) << 16) | (((uint4)input[j + 3]) << 24);
}

static inline unsigned int rotate_left(uint4 x, uint4 n)
{
   return (x << n) | (x >> (32 - n));
}

// F, G, H and I are basic MD5 functions.

static inline unsigned int MD5_F(uint4 x, uint4 y, uint4 z)
{
   return (x & y) | (~x & z);
}

static inline unsigned int MD5_G(uint4 x, uint4 y, uint4 z)
{
   return (x & z) | (y & ~z);
}

static inline unsigned int MD5_H(uint4 x, uint4 y, uint4 z)
{
   return x ^ y ^ z;
}

static inline unsigned int MD5_I(uint4 x, uint4 y, uint4 z)
{
   return y ^ (x | ~z);
}



// FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
// Rotation is separate from addition to prevent recomputation.

static inline void MD5_FF(uint4* a, uint4 b, uint4 c, uint4 d, uint4 x,
			  uint4 s, uint4 ac)
{
   *a += MD5_F(b, c, d) + x + ac;
   *a = rotate_left(*a, s) + b;
}

static inline void MD5_GG(uint4* a, uint4 b, uint4 c, uint4 d, uint4 x,
			  uint4 s, uint4 ac)
{
   *a += MD5_G(b, c, d) + x + ac;
   *a = rotate_left(*a, s) + b;
}

static inline void MD5_HH(uint4* a, uint4 b, uint4 c, uint4 d, uint4 x,
			  uint4 s, uint4 ac)
{
   *a += MD5_H(b, c, d) + x + ac;
   *a = rotate_left(*a, s) + b;
}

static inline void MD5_II(uint4* a, uint4 b, uint4 c, uint4 d, uint4 x,
			  uint4 s, uint4 ac)
{
   *a += MD5_I(b, c, d) + x + ac;
   *a = rotate_left(*a, s) + b;
}

// MD5 basic transformation. Transforms state based on block.
static void transform(MD5_CTX *p, uint1 block[64])
{
   uint4 a = p->state[0], b = p->state[1], c = p->state[2], d = p->state[3], x[16];

   decode (x, block, 64);

   /* Round 1 */
   MD5_FF(&a, b, c, d, x[ 0], S11, 0xd76aa478); /* 1 */
   MD5_FF(&d, a, b, c, x[ 1], S12, 0xe8c7b756); /* 2 */
   MD5_FF(&c, d, a, b, x[ 2], S13, 0x242070db); /* 3 */
   MD5_FF(&b, c, d, a, x[ 3], S14, 0xc1bdceee); /* 4 */
   MD5_FF(&a, b, c, d, x[ 4], S11, 0xf57c0faf); /* 5 */
   MD5_FF(&d, a, b, c, x[ 5], S12, 0x4787c62a); /* 6 */
   MD5_FF(&c, d, a, b, x[ 6], S13, 0xa8304613); /* 7 */
   MD5_FF(&b, c, d, a, x[ 7], S14, 0xfd469501); /* 8 */
   MD5_FF(&a, b, c, d, x[ 8], S11, 0x698098d8); /* 9 */
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

   // Zeroize sensitive information.
   memset ( (uint1 *) x, 0, sizeof(x));
}

void MD5Update(MD5_CTX *p, unsigned char *input, unsigned int input_length)
{
   uint4 input_index, buffer_index;
   uint4 buffer_space;                // how much space is left in buffer

   // Compute number of bytes mod 64
   buffer_index = (unsigned int)((p->count[0] >> 3) & 0x3F);

   // Update number of bits
   if ((p->count[0] += ((uint4)input_length << 3))<((uint4)input_length << 3))
      p->count[1]++;

   p->count[1] += ((uint4)input_length >> 29);

   buffer_space = 64 - buffer_index;  // how much space is left in buffer

   // Transform as many times as possible.
   if (input_length >= buffer_space)
   { // ie. we have enough to fill the buffer
      // fill the rest of the buffer and transform
      memcpy(p->buffer + buffer_index, input, buffer_space);
      transform(p, p->buffer);

      // now, transform each 64-byte piece of the input, bypassing the buffer
      for (input_index = buffer_space; input_index + 63 < input_length;
	   input_index += 64)
	 transform(p, input + input_index);

      buffer_index = 0;  // so we can buffer remaining
   }
   else
      input_index = 0;     // so we can buffer the whole input

   // and here we do the buffering:
   memcpy(p->buffer + buffer_index, input + input_index, input_length - input_index);
}

void MD5Final(unsigned char digest[16], MD5_CTX *p)
{
   unsigned char bits[8];
   unsigned int index, padLen;
   static uint1 PADDING[64]={
      0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
   };

   // Save number of bits
   encode(bits, p->count, 8);

   // Pad out to 56 mod 64.
   index = (uint4) ((p->count[0] >> 3) & 0x3f);
   padLen = (index < 56) ? (56 - index) : (120 - index);
   MD5Update(p, PADDING, padLen);

   // Append length (before padding)
   MD5Update(p, bits, 8);

   // Store state in digest
   encode(digest, p->state, 16);

   // Zeroize sensitive information
   memset(p->buffer, 0, sizeof(p->buffer));
}

struct PathNode {
   BPTR next;
   BPTR dir;
};

BPTR cloneWorkbenchPath(struct ExecBase *SysBase,
			struct DosLibrary *DOSBase,
			struct WBStartup *wbmsg)
{
   BPTR path = 0;

   Forbid();
   if (wbmsg->sm_Message.mn_ReplyPort)
   {
      if (((LONG)wbmsg->sm_Message.mn_ReplyPort->mp_Flags & PF_ACTION) == PA_SIGNAL)
      {
	 struct Process *wbproc = wbmsg->sm_Message.mn_ReplyPort->mp_SigTask;
	 if (wbproc->pr_Task.tc_Node.ln_Type == NT_PROCESS)
	 {
	    struct CommandLineInterface *cli = BADDR(wbproc->pr_CLI);
	    if (cli)
	    {
	       BPTR *p = &path;
	       BPTR dir = cli->cli_CommandDir;
	       while (dir)
	       {
		  BPTR dir2;
		  struct FileLock *lock = BADDR(dir);
		  struct PathNode *node;
		  dir = lock->fl_Link;
		  dir2 = DupLock(lock->fl_Key);
		  if (!dir2)
		     break;
		  node = AllocVec(8, MEMF_PUBLIC);
		  if (!node)
		  {
		     UnLock(dir2);
		     break;
		  }
		  node->next = NULL;
		  node->dir = dir2;
		  *p = MKBADDR(node);
		  p = &node->next; 
	       }
	    }
	 }
      }
   }
   Permit();

   return path;
}

void freeWorkbenchPath(struct ExecBase *SysBase,
		       struct DosLibrary *DOSBase,
		       BPTR path)
{
   while (path)
   {
      struct PathNode *node = BADDR(path);
      path = node->next;
      UnLock(node->dir);
      FreeVec(node);
   }
}

