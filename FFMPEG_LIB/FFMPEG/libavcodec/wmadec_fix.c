/*
 * WMA compatible decoder
 * Copyright (c) 2002 The FFmpeg Project.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/**
 * @file wmadec.c
 * WMA compatible decoder.
 */

#include <inttypes.h>

#define TRACE
/* size of blocks */
#define BLOCK_MIN_BITS 7
#define BLOCK_MAX_BITS 11
#define BLOCK_MAX_SIZE (1 << BLOCK_MAX_BITS)

#define BLOCK_NB_SIZES (BLOCK_MAX_BITS - BLOCK_MIN_BITS + 1)

/* XXX: find exact max size */
#define HIGH_BAND_MAX_SIZE 16

#define NB_LSP_COEFS 10

/* XXX: is it a suitable value ? */
#define MAX_CODED_SUPERFRAME_SIZE 16384

#define M_PI    3.14159265358979323846

#define M_PI_F  0x3243f // in fixed 32 format
#define TWO_M_PI_F  0x6487f	//in fixed 32

#define MAX_CHANNELS 2

#define NOISE_TAB_SIZE 8192

#define LSP_POW_BITS 7

#define VLC_TYPE int16_t

#define PRECISION       16
#define PRECISION64     16

#define fixed32         int32_t
#define fixed64         int64_t

/* Inverse gain of circular cordic rotation in s0.31 format. */
static const long cordic_circular_gain = 0xb2458939; /* 0.607252929 */

typedef struct CoefVLCTable
{
    int n; /* total number of codes */
    const uint32_t *huffcodes; /* VLC bit values */
    const uint8_t *huffbits;   /* VLC bit size */
    const uint16_t *levels; /* table to build run/level tables */
}
CoefVLCTable;

#include "avcodec.h"
#include "wmadata_fix.h"


typedef fixed32 FFTSample;


static fixed64 IntTo64(int x)
{
    fixed64 res = 0;
    unsigned char *p = (unsigned char *)&res;

    p[2] = x & 0xff;
    p[3] = (x & 0xff00)>>8;
    p[4] = (x & 0xff0000)>>16;
    p[5] = (x & 0xff000000)>>24;
    return res;
}

static int IntFrom64(fixed64 x)
{
    int res = 0;
    unsigned char *p = (unsigned char *)&x;

    res = p[2] | (p[3]<<8) | (p[4]<<16) | (p[5]<<24);
    return res;
}

#define Fixed32From64(x) x & 0xFFFFFFFF
#define Fixed32To64(x) (fixed64)x
#define fixmul64byfixed(x,y) x*y

#define fixtof64(x)       (float)((float)(x) / (float)(1 << PRECISION64))		//does not work on int64_t!
#define ftofix32(x)       ((fixed32)((x) * (float)(1 << PRECISION) + ((x) < 0 ? -0.5 : 0.5)))
#define itofix64(x)       (IntTo64(x))
#define itofix32(x)       ((x) << PRECISION)
#define fixtoi32(x)       ((x) >> PRECISION)
#define fixtoi64(x)       (IntFrom64(x))


fixed32 fixmul32(fixed32 x, fixed32 y)
{
    fixed64 temp;
	float lol;
    lol= fixtof64(x) * fixtof64(y);

    temp = x;
    temp *= y;

    temp >>= PRECISION;

    return (fixed32)temp;
}

//mgg: special fixmul32 that does a 16.16 x 1.31 multiply that returns a 16.16 value.
//this is needed because the fft constants are all normalized to be less then 1 and can't fit into a 16 bit number

static fixed32 fixmul32b(fixed32 x, fixed32 y)
{
    fixed64 temp;

    temp = x;
    temp *= y;

    temp >>= 31;		//16+31-16 = 31 bits

    return (fixed32)temp;
}


fixed32 fixdiv32(fixed32 x, fixed32 y)
{
    fixed64 temp;

    if(x == 0)
        return 0;
    if(y == 0)
        return 0x7fffffff;
    temp = x;
    temp <<= PRECISION;
    return (fixed32)(temp / y);
}


static fixed64 fixdiv64(fixed64 x, fixed64 y)
{
    fixed64 temp;

    if(x == 0)
        return 0;
    if(y == 0)
        return 0x07ffffffffffffff;
    temp = x;
    temp <<= PRECISION64;
    return (fixed64)(temp / y);
}

static fixed32 fixsqrt32(fixed32 x)
{

    unsigned long r = 0, s, v = (unsigned long)x;

#define STEP(k) s = r + (1 << k * 2); r >>= 1; \
    if (s <= v) { v -= s; r |= (1 << k * 2); }

    STEP(15);
    STEP(14);
    STEP(13);
    STEP(12);
    STEP(11);
    STEP(10);
    STEP(9);
    STEP(8);
    STEP(7);
    STEP(6);
    STEP(5);
    STEP(4);
    STEP(3);
    STEP(2);
    STEP(1);
    STEP(0);

    return (fixed32)(r << (PRECISION / 2));
}


__inline fixed32 fixsin32(fixed32 x)
{

    fixed64 x2, temp;
    int     sign = 1;

    if(x < 0)
    {
        sign = -1;
        x = -x;
    }
    while (x > 0x19220)
    {
        x -= M_PI_F;
        sign = -sign;
    }
    if (x > 0x19220)
    {
        x = M_PI_F - x;
    }
    x2 = (fixed64)x * x;
    x2 >>= PRECISION;
    if(sign != 1)
    {
        x = -x;
    }
    /**
    temp = ftofix32(-.0000000239f) * x2;
    temp >>= PRECISION;
    **/
    temp = 0; // PJJ
    //temp = (temp + 0x0) * x2;	//MGG:  this can't possibly do anything?
    //temp >>= PRECISION;
    temp = (temp - 0xd) * x2;
    temp >>= PRECISION;
    temp = (temp + 0x222) * x2;
    temp >>= PRECISION;
    temp = (temp - 0x2aab) * x2;
    temp >>= PRECISION;
    temp += 0x10000;
    temp = temp * x;
    temp >>= PRECISION;

    return  (fixed32)(temp);
}

__inline fixed32 fixcos32(fixed32 x)
{
    return fixsin32(x - (M_PI_F>>1))*-1;
}


/**
 * Implements sin and cos using CORDIC rotation.
 *
 * @param phase has range from 0 to 0xffffffff, representing 0 and
 *        2*pi respectively.
 * @param cos return address for cos
 * @return sin of phase, value is a signed value from LONG_MIN to LONG_MAX,
 *         representing -1 and 1 respectively.
 *
 *		Gives at least 24 bits precision (last 2-8 bits or so are probably off)
 */
static long fsincos(unsigned long phase, fixed32 *cos)
{
    int32_t x, x1, y, y1;
    unsigned long z, z1;
    int i;

    /* Setup initial vector */
    x = cordic_circular_gain;
    y = 0;
    z = phase;

    /* The phase has to be somewhere between 0..pi for this to work right */
    if (z < 0xffffffff / 4) {
        /* z in first quadrant, z += pi/2 to correct */
        x = -x;
        z += 0xffffffff / 4;
    } else if (z < 3 * (0xffffffff / 4)) {
        /* z in third quadrant, z -= pi/2 to correct */
        z -= 0xffffffff / 4;
    } else {
        /* z in fourth quadrant, z -= 3pi/2 to correct */
        x = -x;
        z -= 3 * (0xffffffff / 4);
    }

    /* Each iteration adds roughly 1-bit of extra precision */
    for (i = 0; i < 31; i++) {
        x1 = x >> i;
        y1 = y >> i;
        z1 = atan_table[i];

        /* Decided which direction to rotate vector. Pivot point is pi/2 */
        if (z >= 0xffffffff / 4) {
            x -= y1;
            y += x1;
            z -= z1;
        } else {
            x += y1;
            y -= x1;
            z += z1;
        }
    }

    if (cos)
        *cos = x;

    return y;
}


#define ALT_BITSTREAM_READER

#define unaligned32(a) (*(uint32_t*)(a))

uint16_t bswap_16(uint16_t x)
{
    uint16_t hi = x & 0xff00;
    uint16_t lo = x & 0x00ff;
    return (hi >> 8) | (lo << 8);
}

uint32_t bswap_32(uint32_t x)
{
    uint32_t b1 = x & 0xff000000;
    uint32_t b2 = x & 0x00ff0000;
    uint32_t b3 = x & 0x0000ff00;
    uint32_t b4 = x & 0x000000ff;
    return (b1 >> 24) | (b2 >> 8) | (b3 << 8) | (b4 << 24);
}

#define CMUL(pre,\
          pim,\
          are,\
          aim,\
          bre,\
          bim)\
{\
    fixed32 _aref = are;\
    fixed32 _aimf = aim;\
    fixed32 _bref = bre;\
    fixed32 _bimf = bim;\
    fixed32 _r1 = fixmul32b(_bref, _aref);\
    fixed32 _r2 = fixmul32b(_bimf, _aimf);\
    fixed32 _r3 = fixmul32b(_bref, _aimf);\
    fixed32 _r4 = fixmul32b(_bimf, _aref);\
    *pre = _r1 - _r2;\
    *pim = _r3 + _r4;\
}\



#ifdef WORDS_BIGENDIAN
#define be2me_16(x) (x)
#define be2me_32(x) (x)
#define be2me_64(x) (x)
#define le2me_16(x) bswap_16(x)
#define le2me_32(x) bswap_32(x)
#define le2me_64(x) bswap_64(x)
#else
#define be2me_16(x) bswap_16(x)
#define be2me_32(x) bswap_32(x)
#define be2me_64(x) bswap_64(x)
#define le2me_16(x) (x)
#define le2me_32(x) (x)
#define le2me_64(x) (x)
#endif

#define NEG_SSR32(a,s) ((( int32_t)(a))>>(32-(s)))
#define NEG_USR32(a,s) (((uint32_t)(a))>>(32-(s)))


void av_free(void *ptr); // PJJ found below

static __inline int unaligned32_be(const void *v)
{
#ifdef CONFIG_ALIGN
    const uint8_t *p=v;
    return (((p[0]<<8) | p[1])<<16) | (p[2]<<8) | (p[3]);
#else

    return be2me_32( unaligned32(v)); //original
#endif
}

typedef struct GetBitContext
{
    const uint8_t *buffer, *buffer_end;
#ifdef ALT_BITSTREAM_READER

    int index;
#elif defined LIBMPEG2_BITSTREAM_READER

    uint8_t *buffer_ptr;
    uint32_t cache;
    int bit_count;
#elif defined A32_BITSTREAM_READER

    uint32_t *buffer_ptr;
    uint32_t cache0;
    uint32_t cache1;
    int bit_count;
#endif

    int size_in_bits;
}
GetBitContext;

typedef struct VLC
{
    int bits;
    VLC_TYPE (*table)[2]; ///< code, bits
    int table_size, table_allocated;
}
VLC;

typedef struct FFTComplex
{
    fixed32 re, im;
}
FFTComplex;

typedef struct FFTContext
{
    int nbits;
    int inverse;
    uint16_t *revtab;
    FFTComplex *exptab;
    FFTComplex *exptab1; /* only used by SSE code */
    int (*fft_calc)(struct FFTContext *s, FFTComplex *z);
}
FFTContext;

typedef struct MDCTContext
{
    int n;  /* size of MDCT (i.e. number of input data * 2) */
    int nbits; /* n = 2^nbits */
    /* pre/post rotation tables */
    fixed32 *tcos;
    fixed32 *tsin;
    FFTContext fft;
}
MDCTContext;

typedef struct WMADecodeContext
{
    GetBitContext gb;
    int sample_rate;
    int nb_channels;
    int bit_rate;
    int version; /* 1 = 0x160 (WMAV1), 2 = 0x161 (WMAV2) */
    int block_align;
    int use_bit_reservoir;
    int use_variable_block_len;
    int use_exp_vlc;  /* exponent coding: 0 = lsp, 1 = vlc + delta */
    int use_noise_coding; /* true if perceptual noise is added */
    int byte_offset_bits;
    VLC exp_vlc;
    int exponent_sizes[BLOCK_NB_SIZES];
    uint16_t exponent_bands[BLOCK_NB_SIZES][25];
    int high_band_start[BLOCK_NB_SIZES]; /* index of first coef in high band */
    int coefs_start;               /* first coded coef */
    int coefs_end[BLOCK_NB_SIZES]; /* max number of coded coefficients */
    int exponent_high_sizes[BLOCK_NB_SIZES];
    int exponent_high_bands[BLOCK_NB_SIZES][HIGH_BAND_MAX_SIZE];
    VLC hgain_vlc;

    /* there are two possible tables for spectral coefficients */
    VLC coef_vlc[2];
    uint16_t *run_table[2];
    uint16_t *level_table[2];
    /* frame info */
    int frame_len;       /* frame length in samples */
    int frame_len_bits;  /* frame_len = 1 << frame_len_bits */
    int nb_block_sizes;  /* number of block sizes */
    /* block info */
    int reset_block_lengths;
    int block_len_bits; /* log2 of current block length */
    int next_block_len_bits; /* log2 of next block length */
    int prev_block_len_bits; /* log2 of prev block length */
    int block_len; /* block length in samples */
    int block_num; /* block number in current frame */
    int block_pos; /* current position in frame */
    uint8_t ms_stereo; /* true if mid/side stereo mode */
    uint8_t channel_coded[MAX_CHANNELS]; /* true if channel is coded */
    fixed32 exponents[MAX_CHANNELS][BLOCK_MAX_SIZE];
    fixed32 max_exponent[MAX_CHANNELS];
    int16_t coefs1[MAX_CHANNELS][BLOCK_MAX_SIZE];
    fixed32 coefs[MAX_CHANNELS][BLOCK_MAX_SIZE];
    MDCTContext mdct_ctx[BLOCK_NB_SIZES];
    fixed32 *windows[BLOCK_NB_SIZES];
    FFTComplex mdct_tmp[BLOCK_MAX_SIZE]; /* temporary storage for imdct */
    /* output buffer for one frame and the last for IMDCT windowing */
    fixed32 frame_out[MAX_CHANNELS][BLOCK_MAX_SIZE * 2];
    /* last frame info */
    uint8_t last_superframe[MAX_CODED_SUPERFRAME_SIZE + 4]; /* padding added */
    int last_bitoffset;
    int last_superframe_len;
    fixed32 noise_table[NOISE_TAB_SIZE];
    int noise_index;
    fixed32 noise_mult; /* XXX: suppress that and integrate it in the noise array */
    /* lsp_to_curve tables */
    fixed32 lsp_cos_table[BLOCK_MAX_SIZE];
    fixed64 lsp_pow_e_table[256];
    fixed64 lsp_pow_m_table1[(1 << LSP_POW_BITS)];
    fixed64 lsp_pow_m_table2[(1 << LSP_POW_BITS)];
}
WMADecodeContext;

/* coded values in high bands */
int high_band_coded[MAX_CHANNELS][HIGH_BAND_MAX_SIZE];
int high_band_values[MAX_CHANNELS][HIGH_BAND_MAX_SIZE];

WMADecodeContext wmacontext;	//buffer for the struct above
typedef AVCodecContext CodecContext;

static void wma_lsp_to_curve_init(WMADecodeContext *s, int frame_len);
int fft_calc(FFTContext *s, FFTComplex *z);

//static variables that replace malloced stuff
fixed32 stat0[2048], stat1[1024], stat2[512], stat3[256], stat4[128];	//these are the MDCT reconstruction windows

fixed32 *tcosarray[5], *tsinarray[5];
fixed32 tcos0[1024], tcos1[512], tcos2[256], tcos3[128], tcos4[64];		//these are the sin and cos rotations used by the MDCT
fixed32 tsin0[1024], tsin1[512], tsin2[256], tsin3[128], tsin4[64];

FFTComplex *exparray[5];									//these are the fft lookup tables
uint16_t *revarray[5];
FFTComplex exptab0[512], exptab1[256], exptab2[128], exptab3[64], exptab4[32];
uint16_t revtab0[1024], revtab1[512], revtab2[256], revtab3[128], revtab4[64];

uint16_t *runtabarray[2], *levtabarray[2];										//these are VLC lookup tables

uint16_t runtab0[1336], runtab1[1336], levtab0[1336], levtab1[1336];				//these could be made smaller since only one can be 1336

char vlcbuf1[24576*10],  vlcbuf2[14336*10], vlcbuf3[1536*10];				//may also be too large by ~ 1KB each?
char vlcbuf4[1536*10];

static __inline int av_log2_wma(unsigned int v)
{
    int n;

    n = 0;
    if (v & 0xffff0000)
    {
        v >>= 16;
        n += 16;
    }
    if (v & 0xff00)
    {
        v >>= 8;
        n += 8;
    }
    n += ff_log2_tab_vma[v];

    return n;
}

#ifdef ALT_BITSTREAM_READER
#   define MIN_CACHE_BITS 25

#   define OPEN_READER(name, gb)\
        int name##_index= (gb)->index;\
        int name##_cache= 0;\

#   define CLOSE_READER(name, gb)\
        (gb)->index= name##_index;\

#   define UPDATE_CACHE(name, gb)\
        name##_cache= unaligned32_be( ((uint8_t *)(gb)->buffer)+(name##_index>>3) ) << (name##_index&0x07);\

#   define SKIP_CACHE(name, gb, num)\
        name##_cache <<= (num);\

// FIXME name?
#   define SKIP_COUNTER(name, gb, num)\
        name##_index += (num);\

#   define SKIP_BITS(name, gb, num)\
        {\
            SKIP_CACHE(name, gb, num)\
            SKIP_COUNTER(name, gb, num)\
        }\

#   define LAST_SKIP_BITS(name, gb, num) SKIP_COUNTER(name, gb, num)
#   define LAST_SKIP_CACHE(name, gb, num) ;

#   define SHOW_UBITS(name, gb, num)\
        NEG_USR32(name##_cache, num)

#   define SHOW_SBITS(name, gb, num)\
        NEG_SSR32(name##_cache, num)

#   define GET_CACHE(name, gb)\
        ((uint32_t)name##_cache)

static __inline int get_bits_count(GetBitContext *s)
{
    return s->index;
}
#elif defined LIBMPEG2_BITSTREAM_READER
//libmpeg2 like reader

#   define MIN_CACHE_BITS 17

#   define OPEN_READER(name, gb)\
        int name##_bit_count=(gb)->bit_count;\
        int name##_cache= (gb)->cache;\
        uint8_t * name##_buffer_ptr=(gb)->buffer_ptr;\

#   define CLOSE_READER(name, gb)\
        (gb)->bit_count= name##_bit_count;\
        (gb)->cache= name##_cache;\
        (gb)->buffer_ptr= name##_buffer_ptr;\

#ifdef LIBMPEG2_BITSTREAM_READER_HACK

#   define UPDATE_CACHE(name, gb)\
    if(name##_bit_count >= 0){\
        name##_cache+= (int)be2me_16(*(uint16_t*)name##_buffer_ptr) << name##_bit_count;\
        ((uint16_t*)name##_buffer_ptr)++;\
        name##_bit_count-= 16;\
    }\

#else

#   define UPDATE_CACHE(name, gb)\
    if(name##_bit_count >= 0){\
        name##_cache+= ((name##_buffer_ptr[0]<<8) + name##_buffer_ptr[1]) << name##_bit_count;\
        name##_buffer_ptr+=2;\
        name##_bit_count-= 16;\
    }\

#endif

#   define SKIP_CACHE(name, gb, num)\
        name##_cache <<= (num);\

#   define SKIP_COUNTER(name, gb, num)\
        name##_bit_count += (num);\

#   define SKIP_BITS(name, gb, num)\
        {\
            SKIP_CACHE(name, gb, num)\
            SKIP_COUNTER(name, gb, num)\
        }\

#   define LAST_SKIP_BITS(name, gb, num) SKIP_BITS(name, gb, num)
#   define LAST_SKIP_CACHE(name, gb, num) SKIP_CACHE(name, gb, num)

#   define SHOW_UBITS(name, gb, num)\
        NEG_USR32(name##_cache, num)

#   define SHOW_SBITS(name, gb, num)\
        NEG_SSR32(name##_cache, num)

#   define GET_CACHE(name, gb)\
        ((uint32_t)name##_cache)

static __inline int get_bits_count(GetBitContext *s)
{
    return (s->buffer_ptr - s->buffer)*8 - 16 + s->bit_count;
}

#elif defined A32_BITSTREAM_READER

#   define MIN_CACHE_BITS 32

#   define OPEN_READER(name, gb)\
        int name##_bit_count=(gb)->bit_count;\
        uint32_t name##_cache0= (gb)->cache0;\
        uint32_t name##_cache1= (gb)->cache1;\
        uint32_t * name##_buffer_ptr=(gb)->buffer_ptr;\

#   define CLOSE_READER(name, gb)\
        (gb)->bit_count= name##_bit_count;\
        (gb)->cache0= name##_cache0;\
        (gb)->cache1= name##_cache1;\
        (gb)->buffer_ptr= name##_buffer_ptr;\

#   define UPDATE_CACHE(name, gb)\
    if(name##_bit_count > 0){\
        const uint32_t next= be2me_32( *name##_buffer_ptr );\
        name##_cache0 |= NEG_USR32(next,name##_bit_count);\
        name##_cache1 |= next<<name##_bit_count;\
        name##_buffer_ptr++;\
        name##_bit_count-= 32;\
    }\

#ifdef ARCH_X86
#   define SKIP_CACHE(name, gb, num)\
        asm(\
            "shldl %2, %1, %0  \n\t"\
            "shll %2, %1  \n\t"\
            : "+r" (name##_cache0), "+r" (name##_cache1)\
            : "Ic" ((uint8_t)num)\
           );
#else
#   define SKIP_CACHE(name, gb, num)\
        name##_cache0 <<= (num);\
        name##_cache0 |= NEG_USR32(name##_cache1,num);\
        name##_cache1 <<= (num);
#endif

#   define SKIP_COUNTER(name, gb, num)\
        name##_bit_count += (num);\

#   define SKIP_BITS(name, gb, num)\
        {\
            SKIP_CACHE(name, gb, num)\
            SKIP_COUNTER(name, gb, num)\
        }\

#   define LAST_SKIP_BITS(name, gb, num) SKIP_BITS(name, gb, num)
#   define LAST_SKIP_CACHE(name, gb, num) SKIP_CACHE(name, gb, num)

#   define SHOW_UBITS(name, gb, num)\
        NEG_USR32(name##_cache0, num)

#   define SHOW_SBITS(name, gb, num)\
        NEG_SSR32(name##_cache0, num)

#   define GET_CACHE(name, gb)\
        (name##_cache0)

static __inline int get_bits_count(GetBitContext *s)
{
    return ((uint8_t*)s->buffer_ptr - s->buffer)*8 - 32 + s->bit_count;
}

#endif

/**
 * read mpeg1 dc style vlc (sign bit + mantisse with no MSB).
 * if MSB not set it is negative
 * @param n length in bits
 * @author BERO
 */
static __inline int get_xbits(GetBitContext *s, int n)
{
    register int tmp;
    register int32_t cache;
    OPEN_READER(re, s)
    UPDATE_CACHE(re, s)
    cache = GET_CACHE(re,s);
    if ((int32_t)cache<0)
    { //MSB=1
        tmp = NEG_USR32(cache,n);
    }
    else
    {
        tmp = - NEG_USR32(~cache,n);
    }
    LAST_SKIP_BITS(re, s, n)
    CLOSE_READER(re, s)
    return tmp;
}

static __inline int get_sbits(GetBitContext *s, int n)
{
    register int tmp;
    OPEN_READER(re, s)
    UPDATE_CACHE(re, s)
    tmp= SHOW_SBITS(re, s, n);
    LAST_SKIP_BITS(re, s, n)
    CLOSE_READER(re, s)
    return tmp;
}

/**
 * reads 0-17 bits.
 * Note, the alt bitstream reader can read upto 25 bits, but the libmpeg2 reader cant
 */
static __inline unsigned int get_bits(GetBitContext *s, int n)
{
    register int tmp;
    OPEN_READER(re, s)
    UPDATE_CACHE(re, s)
    tmp= SHOW_UBITS(re, s, n);
    LAST_SKIP_BITS(re, s, n)
    CLOSE_READER(re, s)
    return tmp;
}


/**
 *
 * if the vlc code is invalid and max_depth=1 than no bits will be removed
 * if the vlc code is invalid and max_depth>1 than the number of bits removed
 * is undefined
 */
#define GET_VLC(code, name, gb, table, bits, max_depth)\
{\
    int n, index, nb_bits;\
\
    index= SHOW_UBITS(name, gb, bits);\
    code = table[index][0];\
    n    = table[index][1];\
\
    if(max_depth > 1 && n < 0){\
        LAST_SKIP_BITS(name, gb, bits)\
        UPDATE_CACHE(name, gb)\
\
        nb_bits = -n;\
\
        index= SHOW_UBITS(name, gb, nb_bits) + code;\
        code = table[index][0];\
        n    = table[index][1];\
        if(max_depth > 2 && n < 0){\
            LAST_SKIP_BITS(name, gb, nb_bits)\
            UPDATE_CACHE(name, gb)\
\
            nb_bits = -n;\
\
            index= SHOW_UBITS(name, gb, nb_bits) + code;\
            code = table[index][0];\
            n    = table[index][1];\
        }\
    }\
    SKIP_BITS(name, gb, n)\
}

static __inline void skip_bits(GetBitContext *s, int n)
{
    //Note gcc seems to optimize this to s->index+=n for the ALT_READER :))
    OPEN_READER(re, s)
    UPDATE_CACHE(re, s)
    LAST_SKIP_BITS(re, s, n)
    CLOSE_READER(re, s)
}

#define align_get_bits(s)\
{\
    int n= (-get_bits_count(s)) & 7;\
    if(n)\
        skip_bits(s, n);\
}\

/**
 * init GetBitContext.
 * @param buffer bitstream buffer, must be FF_INPUT_BUFFER_PADDING_SIZE bytes larger then the actual read bits
 * because some optimized bitstream readers read 32 or 64 bit at once and could read over the end
 * @param bit_size the size of the buffer in bits
 */
void init_get_bits(GetBitContext *s,
                   const uint8_t *buffer, int bit_size)
{
    int buffer_size= (bit_size+7)>>3;

    if (buffer_size < 0 || bit_size < 0)
    {
        buffer_size = bit_size = 0;
        buffer = 0;
    }
    s->buffer= buffer;
    s->size_in_bits= bit_size;
    s->buffer_end= buffer + buffer_size;
#ifdef ALT_BITSTREAM_READER

    s->index=0;
#elif defined LIBMPEG2_BITSTREAM_READER
#ifdef LIBMPEG2_BITSTREAM_READER_HACK

    if ((int)buffer&1)
    {
        /* word alignment */
        s->cache = (*buffer++)<<24;
        s->buffer_ptr = buffer;
        s->bit_count = 16-8;
    }
    else
#endif

    {
        s->buffer_ptr = buffer;
        s->bit_count = 16;
        s->cache = 0;
    }
#elif defined A32_BITSTREAM_READER
    s->buffer_ptr = (uint32_t*)buffer;
    s->bit_count = 32;
    s->cache0 = 0;
    s->cache1 = 0;
#endif

    {
        OPEN_READER(re, s)
        UPDATE_CACHE(re, s)
        UPDATE_CACHE(re, s)
        CLOSE_READER(re, s)
    }
#ifdef A32_BITSTREAM_READER
    s->cache1 = 0;
#endif

}

/* cannot call it directly because of 'void **' casting is not automatic */
void __av_freepy(void **ptr)
{
    av_free(*ptr);
    *ptr = NULL;
}
#define av_freep(p) __av_freepy((void **)(p))

/**
 * The size of the FFT is 2^nbits. If inverse is TRUE, inverse FFT is
 * done
 */
int fft_inits(FFTContext *s, int nbits, int inverse)
{
    int i, j, m, n;
    fixed32 alpha, c1, s1, ct, st;
    int s2;

    s->nbits = nbits;
    n = 1 << nbits;

    s->exptab = exparray[10-nbits];
    s->revtab = revarray[10-nbits];
    s->inverse = inverse;

    s2 = inverse ? 1 : -1;

    for(i=0;i<(n/2);++i)
    {
	    //we're going to redo this in CORDIC fixed format!     Hold onto your butts

	    /*
	    input to cordic is from 0 ->2pi with 0->0 and 2^32-1 ->2pi
	    output, which is what we'll store the variables as is
	    	-1->-2^31 and 1->2^31-1

	    */

        fixed32 ifix = itofix32(i);
        fixed32 nfix = itofix32(n);
        fixed32 res = fixdiv32(ifix,nfix);		//this is really bad here since nfix can be as large as 1024 ! also, make this a shift
        alpha = fixmul32(TWO_M_PI_F, res);
        ct = fixcos32(alpha);					//need to correct alpha for 0->2pi scale
        st = fixsin32(alpha);// * s2;

        s1 = fsincos(res<<16, &c1);		//does sin and cos in one pass!

	//I really have my doubts about the correctness of the alpha to cordic mapping here, but it seems to work well enough
	//double check this later!

        s->exptab[i].re = c1;
        s->exptab[i].im = s1*s2;
    }

   // s->fft_calc = fft_calc;
    s->exptab1 = NULL;


    /* compute bit reverse table */

    for(i=0;i<n;i++)
    {
        m=0;
        for(j=0;j<nbits;j++)
        {
            m |= ((i >> j) & 1) << (nbits-j-1);

        }

        s->revtab[i]=m;
    }
    return 0;
}

/* butter fly op */
#define BF(pre, pim, qre, qim, pre1, pim1, qre1, qim1) \
{\
  fixed32 ax, ay, bx, by;\
  bx=pre1;\
  by=pim1;\
  ax=qre1;\
  ay=qim1;\
  pre = (bx + ax);\
  pim = (by + ay);\
  qre = (bx - ax);\
  qim = (by - ay);\
}

int fft_calc_unscaled(FFTContext *s, FFTComplex *z)
{
    int ln = s->nbits;
    int j, np, np2;
    int nblocks, nloops;
    register FFTComplex *p, *q;
    FFTComplex *exptab = s->exptab;
    int l;
    fixed32 tmp_re, tmp_im;

    np = 1 << ln;


    /* pass 0 */

    p=&z[0];
    j=(np >> 1);
    do
    {
        BF(p[0].re, p[0].im, p[1].re, p[1].im,
           p[0].re, p[0].im, p[1].re, p[1].im);
        p+=2;
    }
    while (--j != 0);

    /* pass 1 */


    p=&z[0];
    j=np >> 2;
    if (s->inverse)
    {
        do
        {
            BF(p[0].re, p[0].im, p[2].re, p[2].im,
               p[0].re, p[0].im, p[2].re, p[2].im);
            BF(p[1].re, p[1].im, p[3].re, p[3].im,
               p[1].re, p[1].im, -p[3].im, p[3].re);
            p+=4;
        }
        while (--j != 0);
    }
    else
    {
        do
        {
            BF(p[0].re, p[0].im, p[2].re, p[2].im,
               p[0].re, p[0].im, p[2].re, p[2].im);
            BF(p[1].re, p[1].im, p[3].re, p[3].im,
               p[1].re, p[1].im, p[3].im, -p[3].re);
            p+=4;
        }
        while (--j != 0);
    }
    /* pass 2 .. ln-1 */

    nblocks = np >> 3;
    nloops = 1 << 2;
    np2 = np >> 1;
    do
    {
        p = z;
        q = z + nloops;
        for (j = 0; j < nblocks; ++j)
        {
            BF(p->re, p->im, q->re, q->im,
               p->re, p->im, q->re, q->im);

            p++;
            q++;
            for(l = nblocks; l < np2; l += nblocks)
            {
                CMUL(&tmp_re, &tmp_im, exptab[l].re, exptab[l].im, q->re, q->im);
                BF(p->re, p->im, q->re, q->im,
                   p->re, p->im, tmp_re, tmp_im);
                p++;
                q++;
            }

            p += nloops;
            q += nloops;
        }
        nblocks = nblocks >> 1;
        nloops = nloops << 1;
    }
    while (nblocks != 0);
	return 0;
}



#define GET_DATA(v, table, i, wrap, size) \
{\
    const uint8_t *ptr = (const uint8_t *)table + i * wrap;\
    switch(size) {\
    case 1:\
        v = *(const uint8_t *)ptr;\
        break;\
    case 2:\
        v = *(const uint16_t *)ptr;\
        break;\
    default:\
        v = *(const uint32_t *)ptr;\
        break;\
    }\
}

// deprecated, dont use get_vlc for new code, use get_vlc2 instead or use GET_VLC directly
static __inline int get_vlc(GetBitContext *s, VLC *vlc)
{
    int code;
    VLC_TYPE (*table)[2]= vlc->table;

    OPEN_READER(re, s)
    UPDATE_CACHE(re, s)

    GET_VLC(code, re, s, table, vlc->bits, 3)

    CLOSE_READER(re, s)
    return code;
}

static int alloc_table(VLC *vlc, int size)
{
    int index = 0;
    index = vlc->table_size;
    vlc->table_size += size;
    if (vlc->table_size > vlc->table_allocated)
    {
        vlc->table_allocated += (1 << vlc->bits);
        vlc->table = av_realloc(vlc->table,
                                sizeof(VLC_TYPE) * 2 * vlc->table_allocated);
        if (!vlc->table)
            return -1;
    }
    return index;
}

static int build_table(VLC *vlc, int table_nb_bits,
                       int nb_codes,
                       const void *bits, int bits_wrap, int bits_size,
                       const void *codes, int codes_wrap, int codes_size,
                       uint32_t code_prefix, int n_prefix)
{
    int i, j, k, n, table_size, table_index, nb, n1, index;
    uint32_t code;
    VLC_TYPE (*table)[2];

    table_size = 1 << table_nb_bits;
    table_index = alloc_table(vlc, table_size);
    if (table_index < 0)
        return -1;
    table = &vlc->table[table_index];

    for(i=0;i<table_size;i++)
    {
        table[i][1] = 0; //bits
        table[i][0] = -1; //codes
    }

    /* first pass: map codes and compute auxillary table sizes */
    for(i=0;i<nb_codes;i++)
    {
        GET_DATA(n, bits, i, bits_wrap, bits_size);
        GET_DATA(code, codes, i, codes_wrap, codes_size);
        /* we accept tables with holes */
        if (n <= 0)
            continue;
        /* if code matches the prefix, it is in the table */
        n -= n_prefix;
        if (n > 0 && (code >> n) == code_prefix)
        {
            if (n <= table_nb_bits)
            {
                /* no need to add another table */
                j = (code << (table_nb_bits - n)) & (table_size - 1);
                nb = 1 << (table_nb_bits - n);
                for(k=0;k<nb;k++)
                {
                    if (table[j][1] /*bits*/ != 0)
                    {
                        // PJJ exit(-1);
                    }
                    table[j][1] = n; //bits
                    table[j][0] = i; //code
                    j++;
                }
            }
            else
            {
                n -= table_nb_bits;
                j = (code >> n) & ((1 << table_nb_bits) - 1);
                /* compute table size */
                n1 = -table[j][1]; //bits
                if (n > n1)
                    n1 = n;
                table[j][1] = -n1; //bits
            }
        }
    }

    /* second pass : fill auxillary tables recursively */
    for(i=0;i<table_size;i++)
    {
        n = table[i][1]; //bits
        if (n < 0)
        {
            n = -n;
            if (n > table_nb_bits)
            {
                n = table_nb_bits;
                table[i][1] = -n; //bits
            }
            index = build_table(vlc, n, nb_codes,
                                bits, bits_wrap, bits_size,
                                codes, codes_wrap, codes_size,
                                (code_prefix << table_nb_bits) | i,
                                n_prefix + table_nb_bits);
            if (index < 0)
                return -1;
            /* note: realloc has been done, so reload tables */
            table = &vlc->table[table_index];
            table[i][0] = index; //code
        }
    }
    return table_index;
}

/* Build VLC decoding tables suitable for use with get_vlc().

   'nb_bits' set thee decoding table size (2^nb_bits) entries. The
   bigger it is, the faster is the decoding. But it should not be too
   big to save memory and L1 cache. '9' is a good compromise.

   'nb_codes' : number of vlcs codes

   'bits' : table which gives the size (in bits) of each vlc code.

   'codes' : table which gives the bit pattern of of each vlc code.

   'xxx_wrap' : give the number of bytes between each entry of the
   'bits' or 'codes' tables.

   'xxx_size' : gives the number of bytes of each entry of the 'bits'
   or 'codes' tables.

   'wrap' and 'size' allows to use any memory configuration and types
   (byte/word/long) to store the 'bits' and 'codes' tables.
*/
int init_vlc(VLC *vlc, int nb_bits, int nb_codes,
             const void *bits, int bits_wrap, int bits_size,
             const void *codes, int codes_wrap, int codes_size)
{
    vlc->bits = nb_bits;
   // vlc->table = NULL;
   // vlc->table_allocated = 0;
    vlc->table_size = 0;

    if (build_table(vlc, nb_bits, nb_codes,
                    bits, bits_wrap, bits_size,
                    codes, codes_wrap, codes_size,
                    0, 0) < 0)
    {
       // av_free(vlc->table);
        return -1;
    }
    return 0;
}

/**
 * init MDCT or IMDCT computation.
 */
int ff_mdct_init_wma(MDCTContext *s, int nbits, int inverse)
{
    int n, n4, i;
    memset(s, 0, sizeof(*s));
    n = 1 << nbits;			//nbits ranges from 12 to 8 inclusive
    s->nbits = nbits;
    s->n = n;
    n4 = n >> 2;
    s->tcos = tcosarray[12-nbits];
    s->tsin = tsinarray[12-nbits];
    for(i=0;i<n4;i++)
    {
        fixed32 ip = itofix32(i) + 0x2000;
        ip = ip >> nbits;

	s->tsin[i] = - fsincos(ip<<16, &(s->tcos[i]));			//I can't remember why this works, but it seems to agree for ~24 bits, maybe more!
	s->tcos[i] *=-1;
  }
    if (fft_inits(&s->fft, s->nbits - 2, inverse) < 0)
        goto fail;
    return 0;
fail:
    return -1;
}

/**
 * Compute inverse MDCT of size N = 2^nbits
 * @param output N samples
 * @param input N/2 samples
 * @param tmp N/2 samples
 */
void ff_imdct_calc_wma(MDCTContext *s,
                   fixed32 *output,
                   const fixed32 *input,
                   FFTComplex *tmp)
{
    int k, n8, n4, n2, n, j,scale;
    const uint16_t *revtab = s->fft.revtab;
    const fixed32 *tcos = s->tcos;
    const fixed32 *tsin = s->tsin;
    const fixed32 *in1, *in2;
    FFTComplex *z = (FFTComplex *)tmp;

    n = 1 << s->nbits;
    n2 = n >> 1;
    n4 = n >> 2;
    n8 = n >> 3;

    /* pre rotation */
    in1 = input;
    in2 = input + n2 - 1;

    for(k = 0; k < n4; k++)
    {
        j=revtab[k];
        CMUL(&z[j].re, &z[j].im, *in2, *in1, tcos[k], tsin[k]);
        in1 += 2;
        in2 -= 2;
    }

    for(k = 0; k < n4; k++){
	    z[k].re >>=1;
	    z[k].im >>=1;
    }


   	 scale = fft_calc_unscaled(&s->fft, z);
   	//	scale = fft_calc(&s->fft, z);

    /* post rotation + reordering */

    for(k = 0; k < n4; k++)
    {
        CMUL(&z[k].re, &z[k].im, (z[k].re), (z[k].im), tcos[k], tsin[k]);
    }

    for(k = 0; k < n8; k++)
    {
        fixed32 r1,r2,r3,r4,r1n,r2n,r3n;

        r1 = z[n8 + k].im;
        r1n = r1 * -1;
        r2 = z[n8-1-k].re;
        r2n = r2 * -1;
        r3 = z[k+n8].re;
        r3n = r3 * -1;
        r4 = z[n8-k-1].im;

        output[2*k] = r1n;
        output[n2-1-2*k] = r1;

        output[2*k+1] = r2;
        output[n2-1-2*k-1] = r2n;

        output[n2 + 2*k]= r3n;
        output[n-1- 2*k]= r3n;

        output[n2 + 2*k+1]= r4;
        output[n-2 - 2 * k] = r4;
    }
}

/* XXX: use same run/length optimization as mpeg decoders */
static void init_coef_vlc(VLC *vlc,
                          uint16_t **prun_table, uint16_t **plevel_table,
                          const CoefVLCTable *vlc_table, int tab)
{
    int n = vlc_table->n;
    const uint8_t *table_bits = vlc_table->huffbits;
    const uint32_t *table_codes = vlc_table->huffcodes;
    const uint16_t *levels_table = vlc_table->levels;
    uint16_t *run_table, *level_table;
    const uint16_t *p;
    int i, l, j, level;


    init_vlc(vlc, 9, n, table_bits, 1, 1, table_codes, 4, 4);

	run_table = runtabarray[tab];
	level_table= levtabarray[tab];
    p = levels_table;
    i = 2;
    level = 1;
    while (i < n)
    {
        l = *p++;
        for(j=0;j<l;++j)
        {
            run_table[i] = j;
            level_table[i] = level;
            ++i;
        }
        ++level;
    }
    *prun_table = run_table;
    *plevel_table = level_table;
}

static int wma_decode_init_fixed(CodecContext * avctx)
{
    WMADecodeContext *s = avctx->priv_data;
    int i, flags1, flags2;
    fixed32 *window;
    uint8_t *extradata;
    fixed64 bps1;
    fixed32 high_freq;
    fixed64 bps;
    int sample_rate1;
    int coef_vlc_table;
	    	fixed32 *temp[5];

    s->sample_rate = avctx->sample_rate;
    s->nb_channels = avctx->channels;
    s->bit_rate = avctx->bit_rate;
    s->block_align = avctx->block_align;

    if (avctx->codec->id == CODEC_ID_WMAV1)
    {
        s->version = 1;
    }
    else
    {
        s->version = 2;
    }

    /* extract flag infos */
    flags1 = 0;
    flags2 = 0;
    extradata = avctx->extradata;
    if (s->version == 1 && avctx->extradata_size >= 4)
    {
        flags1 = extradata[0] | (extradata[1] << 8);
        flags2 = extradata[2] | (extradata[3] << 8);
    }
    else if (s->version == 2 && avctx->extradata_size >= 6)
    {
        flags1 = extradata[0] | (extradata[1] << 8) |
                 (extradata[2] << 16) | (extradata[3] << 24);
        flags2 = extradata[4] | (extradata[5] << 8);
    }
    s->use_exp_vlc = flags2 & 0x0001;
    s->use_bit_reservoir = flags2 & 0x0002;
    s->use_variable_block_len = flags2 & 0x0004;

    /* compute MDCT block size */
    if (s->sample_rate <= 16000)
    {
        s->frame_len_bits = 9;
    }
    else if (s->sample_rate <= 22050 ||
             (s->sample_rate <= 32000 && s->version == 1))
    {
        s->frame_len_bits = 10;
    }
    else
    {
        s->frame_len_bits = 11;
    }
    s->frame_len = 1 << s->frame_len_bits;
    if (s-> use_variable_block_len)
    {
        int nb_max, nb;
        nb = ((flags2 >> 3) & 3) + 1;
        if ((s->bit_rate / s->nb_channels) >= 32000)
        {
            nb += 2;
        }
        nb_max = s->frame_len_bits - BLOCK_MIN_BITS;		//max is 11-7
        if (nb > nb_max)
            nb = nb_max;
        s->nb_block_sizes = nb + 1;
    }
    else
    {
        s->nb_block_sizes = 1;
    }

    /* init rate dependant parameters */
    s->use_noise_coding = 1;
    high_freq = fixmul64byfixed(itofix64(s->sample_rate), 0x8000);


    /* if version 2, then the rates are normalized */
    sample_rate1 = s->sample_rate;
    if (s->version == 2)
    {
        if (sample_rate1 >= 44100)
            sample_rate1 = 44100;
        else if (sample_rate1 >= 22050)
            sample_rate1 = 22050;
        else if (sample_rate1 >= 16000)
            sample_rate1 = 16000;
        else if (sample_rate1 >= 11025)
            sample_rate1 = 11025;
        else if (sample_rate1 >= 8000)
            sample_rate1 = 8000;
    }

	{
		fixed64 tim , tmpi;
		fixed64 tmp = itofix64(s->bit_rate);
		fixed64 tmp2 = itofix64(s->nb_channels * s->sample_rate);
		bps = fixdiv64(tmp, tmp2);
		tim = fixmul64byfixed(bps, s->frame_len);
		tmpi = fixdiv64(tim,itofix64(8));
		s->byte_offset_bits = av_log2_wma(fixtoi64(tmpi)) + 2;
	}

    /* compute high frequency value and choose if noise coding should
       be activated */
    bps1 = bps;
    if (s->nb_channels == 2)
        bps1 = fixmul32(bps,0x1999a);
    if (sample_rate1 == 44100)
    {
        if (bps1 >= 0x9c29)
            s->use_noise_coding = 0;
        else
            high_freq = fixmul64byfixed(high_freq,0x6666);
    }
    else if (sample_rate1 == 22050)
    {
        if (bps1 >= 0x128f6)
            s->use_noise_coding = 0;
        else if (bps1 >= 0xb852)
            high_freq = fixmul64byfixed(high_freq,0xb333);
        else
            high_freq = fixmul64byfixed(high_freq,0x999a);
    }
    else if (sample_rate1 == 16000)
    {
        if (bps > 0x8000)
            high_freq = fixmul64byfixed(high_freq,0x8000);
        else
            high_freq = fixmul64byfixed(high_freq,0x4ccd);
    }
    else if (sample_rate1 == 11025)
    {
        high_freq = fixmul64byfixed(high_freq,0xb3333);
    }
    else if (sample_rate1 == 8000)
    {
        if (bps <= 0xa000)
        {
            high_freq = fixmul64byfixed(high_freq,0x8000);
        }
        else if (bps > 0xc000)
        {
            s->use_noise_coding = 0;
        }
        else
        {
            high_freq = fixmul64byfixed(high_freq,0xa666);
        }
    }
    else
    {
        if (bps >= 0xcccd)
        {
            high_freq = fixmul64byfixed(high_freq,0xc000);
        }
        else if (bps >= 0x999a)
        {
            high_freq = fixmul64byfixed(high_freq,0x999a);
        }
        else
        {
            high_freq = fixmul64byfixed(high_freq,0x8000);
        }
    }

    /* compute the scale factor band sizes for each MDCT block size */
    {
        int a, b, pos, lpos, k, block_len, i, j, n;
        const uint8_t *table;

        if (s->version == 1)
        {
            s->coefs_start = 3;
        }
        else
        {
            s->coefs_start = 0;
        }
        for(k = 0; k < s->nb_block_sizes; ++k)
        {
            block_len = s->frame_len >> k;

            if (s->version == 1)
            {
                lpos = 0;
                for(i=0;i<25;++i)
                {
                    a = wma_critical_freqs[i];
                    b = s->sample_rate;
                    pos = ((block_len * 2 * a)  + (b >> 1)) / b;
                    if (pos > block_len)
                        pos = block_len;
                    s->exponent_bands[0][i] = pos - lpos;
                    if (pos >= block_len)
                    {
                        ++i;
                        break;
                    }
                    lpos = pos;
                }
                s->exponent_sizes[0] = i;
            }
            else
            {
                /* hardcoded tables */
                table = NULL;
                a = s->frame_len_bits - BLOCK_MIN_BITS - k;
                if (a < 3)
                {
                    if (s->sample_rate >= 44100)
                        table = exponent_band_44100[a];
                    else if (s->sample_rate >= 32000)
                        table = exponent_band_32000[a];
                    else if (s->sample_rate >= 22050)
                        table = exponent_band_22050[a];
                }
                if (table)
                {
                    n = *table++;
                    for(i=0;i<n;++i)
                        s->exponent_bands[k][i] = table[i];
                    s->exponent_sizes[k] = n;
                }
                else
                {
                    j = 0;
                    lpos = 0;
                    for(i=0;i<25;++i)
                    {
                        a = wma_critical_freqs[i];
                        b = s->sample_rate;
                        pos = ((block_len * 2 * a)  + (b << 1)) / (4 * b);
                        pos <<= 2;
                        if (pos > block_len)
                            pos = block_len;
                        if (pos > lpos)
                            s->exponent_bands[k][j++] = pos - lpos;
                        if (pos >= block_len)
                            break;
                        lpos = pos;
                    }
                    s->exponent_sizes[k] = j;
                }
            }

            /* max number of coefs */
            s->coefs_end[k] = (s->frame_len - ((s->frame_len * 9) / 100)) >> k;
            /* high freq computation */
			{
				fixed64 tmp2;
				fixed64 tmp = itofix64(block_len<<2);

				tmp = fixmul64byfixed(tmp,high_freq);
				tmp2 = itofix64(s->sample_rate);
				tmp2 += 0x8000;
				s->high_band_start[k] = fixtoi64(fixdiv64(tmp,tmp2));
			}

            n = s->exponent_sizes[k];
            j = 0;
            pos = 0;
            for(i=0;i<n;++i)
            {
                int start, end;
                start = pos;
                pos += s->exponent_bands[k][i];
                end = pos;
                if (start < s->high_band_start[k])
                    start = s->high_band_start[k];
                if (end > s->coefs_end[k])
                    end = s->coefs_end[k];
                if (end > start)
                    s->exponent_high_bands[k][j++] = end - start;
            }
            s->exponent_high_sizes[k] = j;
        }
    }

    /* init MDCT */
    tcosarray[0] = tcos0; tcosarray[1] = tcos1; tcosarray[2] = tcos2; tcosarray[3] = tcos3;tcosarray[4] = tcos4;
    tsinarray[0] = tsin0; tsinarray[1] = tsin1; tsinarray[2] = tsin2; tsinarray[3] = tsin3;tsinarray[4] = tsin4;

    exparray[0] = exptab0; exparray[1] = exptab1; exparray[2] = exptab2; exparray[3] = exptab3; exparray[4] = exptab4;
    revarray[0]=revtab0; revarray[1]=revtab1; revarray[2]=revtab2; revarray[3]=revtab3; revarray[4]=revtab4;

    for(i = 0; i < s->nb_block_sizes; ++i)
    {
        ff_mdct_init_wma(&s->mdct_ctx[i], s->frame_len_bits - i + 1, 1);
    }

    /*ffmpeg uses malloc to only allocate as many window sizes as needed.  However, we're really only interested in the worst case memory usage.
    * In the worst case you can have 5 window sizes, 128 doubling up 2048
    * Smaller windows are handled differently.
    * Since we don't have malloc, just statically allocate this
    */
	temp[0] = stat0;
	temp[1] = stat1;
	temp[2] = stat2;
	temp[3] = stat3;
	temp[4] = stat4;

    /* init MDCT windows : simple sinus window */
    for(i = 0; i < s->nb_block_sizes; i++)
    {
        int n, j;
        fixed32 alpha;
        n = 1 << (s->frame_len_bits - i);
        window = temp[i];

        alpha = M_PI_F>>(s->frame_len_bits - i+1);
        for(j=0;j<n;++j)
        {
            fixed32 j2 = itofix32(j) + 0x8000;
            window[n - j - 1] = fixsin32(fixmul32(j2,alpha));		//alpha between 0 and pi/2

        }
        s->windows[i] = window;
    }

    s->reset_block_lengths = 1;

    if (s->use_noise_coding)
    {
        /* init the noise generator */
        if (s->use_exp_vlc)
        {
            s->noise_mult = 0x51f;
        }
        else
        {
            s->noise_mult = 0xa3d;
        }


        {
            unsigned int seed;
            fixed32 norm;
            seed = 1;
            norm = 0;   // PJJ: near as makes any diff to 0!
            for (i=0;i<NOISE_TAB_SIZE;++i)
            {
                seed = seed * 314159 + 1;
                s->noise_table[i] = itofix32((int)seed) * norm;
            }
        }

	    s->exp_vlc.table = vlcbuf4;
	    s->exp_vlc.table_allocated = 15360/2;
        init_vlc(&s->hgain_vlc, 9, sizeof(hgain_huffbits),
                 hgain_huffbits, 1, 1,
                 hgain_huffcodes, 2, 2);
    }

    if (s->use_exp_vlc)
    {
	    s->exp_vlc.table = vlcbuf3;
	    s->exp_vlc.table_allocated = 15360/4;
        init_vlc(&s->exp_vlc, 9, sizeof(scale_huffbits),
                 scale_huffbits, 1, 1,
                 scale_huffcodes, 4, 4);
    }
    else
    {
        wma_lsp_to_curve_init(s, s->frame_len);
    }

    /* choose the VLC tables for the coefficients */
    coef_vlc_table = 2;
    if (s->sample_rate >= 32000)
    {
        if (bps1 < 0xb852)
            coef_vlc_table = 0;
        else if (bps1 < 0x128f6)
            coef_vlc_table = 1;
    }

    runtabarray[0] = runtab0; runtabarray[1] = runtab1;
    levtabarray[0] = levtab0; levtabarray[1] = levtab1;

	s->coef_vlc[0].table = vlcbuf1;
	s->coef_vlc[0].table_allocated = (24576*10)/4;
	s->coef_vlc[1].table = vlcbuf2;
	s->coef_vlc[1].table_allocated = (14336*10)/4;

    init_coef_vlc(&s->coef_vlc[0], &s->run_table[0], &s->level_table[0],
                  &coef_vlcs[coef_vlc_table * 2], 0);
    init_coef_vlc(&s->coef_vlc[1], &s->run_table[1], &s->level_table[1],
                  &coef_vlcs[coef_vlc_table * 2 + 1], 1);

    return 0;
}

/* interpolate values for a bigger or smaller block. The block must
   have multiple sizes */
static void interpolate_array(fixed32 *scale, int old_size, int new_size)
{
    int i, j, jincr, k;
    fixed32 v;

    if (new_size > old_size)
    {
        jincr = new_size / old_size;
        j = new_size;
        for(i = old_size - 1; i >=0; --i)
        {
            v = scale[i];
            k = jincr;
            do
            {
                scale[--j] = v;
            }
            while (--k);
        }
    }
    else if (new_size < old_size)
    {
        j = 0;
        jincr = old_size / new_size;
        for(i = 0; i < new_size; ++i)
        {
            scale[i] = scale[j];
            j += jincr;
        }
    }
}

/* compute x^-0.25 with an exponent and mantissa table. We use linear
   interpolation to reduce the mantissa table size at a small speed
   expense (linear interpolation approximately doubles the number of
   bits of precision). */
static __inline fixed32 pow_m1_4(WMADecodeContext *s, fixed32 x)
{
    union {
        fixed64 f;
        unsigned int v;
    } u, t;
    unsigned int e, m;
    fixed64 a, b;

    u.f = x;
    e = u.v >> 23;
    m = (u.v >> (23 - LSP_POW_BITS)) & ((1 << LSP_POW_BITS) - 1);
    /* build interpolation scale: 1 <= t < 2. */
    t.v = ((u.v << LSP_POW_BITS) & ((1 << 23) - 1)) | (127 << 23);
    a = s->lsp_pow_m_table1[m];
    b = s->lsp_pow_m_table2[m];
    return lsp_pow_e_table[e] * (a + b * t.f);
}

static void wma_lsp_to_curve_init(WMADecodeContext *s, int frame_len)
{
    fixed32 wdel, a, b;
    int i, m;

    wdel = fixdiv32(M_PI_F, itofix32(frame_len));
    for (i=0; i<frame_len; ++i)
    {
        s->lsp_cos_table[i] = 0x20000 * fixcos32(wdel * i);	//wdel*i between 0 and pi

    }
    /* NOTE: these two tables are needed to avoid two operations in
       pow_m1_4 */
    b = itofix32(1);
	{
    int ix = 0;
    for(i=(1 << LSP_POW_BITS) - 1;i>=0;i--)
    {
        m = (1 << LSP_POW_BITS) + i;
        a = m * (0x8000 / (1 << LSP_POW_BITS)); //PJJ
        a = pow_a_table[ix++];  // PJJ : further refinement
        s->lsp_pow_m_table1[i] = 2 * a - b;
        s->lsp_pow_m_table2[i] = b - a;
        b = a;
    }
	}
}

/* NOTE: We use the same code as Vorbis here */
/* XXX: optimize it further with SSE/3Dnow */
static void wma_lsp_to_curve(WMADecodeContext *s,
                             fixed32 *out,
                             fixed32 *val_max_ptr,
                             int n,
                             fixed32 *lsp)
{
    int i, j;
    fixed32 p, q, w, v, val_max;

    val_max = 0;
    for(i=0;i<n;++i)
    {
        p = 0x8000;
        q = 0x8000;
        w = s->lsp_cos_table[i];
        for (j=1;j<NB_LSP_COEFS;j+=2)
        {
            q *= w - lsp[j - 1];
            p *= w - lsp[j];
        }
        p *= p * (0x20000 - w);
        q *= q * (0x20000 + w);
        v = p + q;
        v = pow_m1_4(s, v); // PJJ
        if (v > val_max)
            val_max = v;
        out[i] = v;
    }
    *val_max_ptr = val_max;
}

/* decode exponents coded with LSP coefficients (same idea as Vorbis) */
static void decode_exp_lsp(WMADecodeContext *s, int ch)
{
    fixed32 lsp_coefs[NB_LSP_COEFS];
    int val, i;

    for (i = 0; i < NB_LSP_COEFS; ++i)
    {
        if (i == 0 || i >= 8)
            val = get_bits(&s->gb, 3);
        else
            val = get_bits(&s->gb, 4);
        lsp_coefs[i] = lsp_codebook[i][val];
    }

    wma_lsp_to_curve(s,
                     s->exponents[ch],
                     &s->max_exponent[ch],
                     s->block_len,
                     lsp_coefs);
}

/* decode exponents coded with VLC codes */
static int decode_exp_vlc(WMADecodeContext *s, int ch)
{
    int last_exp, n, code;
    const uint16_t *ptr, *band_ptr;
    fixed32 v, max_scale;
    fixed32 *q,*q_end;

    band_ptr = s->exponent_bands[s->frame_len_bits - s->block_len_bits];
    ptr = band_ptr;
    q = s->exponents[ch];
    q_end = q + s->block_len;
    max_scale = 0;


    if (s->version == 1)		//wmav1 only
    {
        last_exp = get_bits(&s->gb, 5) + 10;
        /* XXX: use a table */
        v = pow_10_to_yover16[last_exp];
        max_scale = v;
        n = *ptr++;
        do
        {
            *q++ = v;
        }
        while (--n);
    }
    last_exp = 36;

    while (q < q_end)
    {
        code = get_vlc(&s->gb, &s->exp_vlc);
        if (code < 0)
        {
            return -1;
        }
        /* NOTE: this offset is the same as MPEG4 AAC ! */
        last_exp += code - 60;
        /* XXX: use a table */
        v = pow_10_to_yover16[last_exp];
        if (v > max_scale)
        {
            max_scale = v;
        }
        n = *ptr++;
        do
        {
            *q++ = v;

        }
        while (--n);
    }

    s->max_exponent[ch] = max_scale;
    return 0;
}

/* return 0 if OK. return 1 if last block of frame. return -1 if
   unrecorrable error. */
static int wma_decode_block(WMADecodeContext *s)
{
    int n, v, a, ch, code, bsize;
    int coef_nb_bits, total_gain, parse_exponents;
    fixed32 window[BLOCK_MAX_SIZE * 2];
    int nb_coefs[MAX_CHANNELS];
    fixed32 mdct_norm;
    /* compute current block length */
    if (s->use_variable_block_len)
    {
        n = av_log2_wma(s->nb_block_sizes - 1) + 1;

        if (s->reset_block_lengths)
        {
            s->reset_block_lengths = 0;
            v = get_bits(&s->gb, n);
            if (v >= s->nb_block_sizes)
            {
                return -1;
            }
            s->prev_block_len_bits = s->frame_len_bits - v;
            v = get_bits(&s->gb, n);
            if (v >= s->nb_block_sizes)
            {
                return -1;
            }
            s->block_len_bits = s->frame_len_bits - v;
        }
        else
        {
            /* update block lengths */
            s->prev_block_len_bits = s->block_len_bits;
            s->block_len_bits = s->next_block_len_bits;
        }
        v = get_bits(&s->gb, n);

        if (v >= s->nb_block_sizes)
        {
            return -1;
        }
        s->next_block_len_bits = s->frame_len_bits - v;
    }
    else
    {
        /* fixed block len */
        s->next_block_len_bits = s->frame_len_bits;
        s->prev_block_len_bits = s->frame_len_bits;
        s->block_len_bits = s->frame_len_bits;
    }
    /* now check if the block length is coherent with the frame length */
    s->block_len = 1 << s->block_len_bits;

    if ((s->block_pos + s->block_len) > s->frame_len)
    {
        return -1;
    }

    if (s->nb_channels == 2)
    {
        s->ms_stereo = get_bits(&s->gb, 1);
    }
    v = 0;
    for (ch = 0; ch < s->nb_channels; ++ch)
    {
        a = get_bits(&s->gb, 1);
        s->channel_coded[ch] = a;
        v |= a;
    }
    /* if no channel coded, no need to go further */
    /* XXX: fix potential framing problems */
    if (!v)
    {
        goto next;
    }

    bsize = s->frame_len_bits - s->block_len_bits;

    /* read total gain and extract corresponding number of bits for
       coef escape coding */
    total_gain = 1;
    for(;;)
    {
        a = get_bits(&s->gb, 7);
        total_gain += a;
        if (a != 127)
        {
            break;
        }
    }

    if (total_gain < 15)
        coef_nb_bits = 13;
    else if (total_gain < 32)
        coef_nb_bits = 12;
    else if (total_gain < 40)
        coef_nb_bits = 11;
    else if (total_gain < 45)
        coef_nb_bits = 10;
    else
        coef_nb_bits = 9;
    /* compute number of coefficients */
    n = s->coefs_end[bsize] - s->coefs_start;

    for(ch = 0; ch < s->nb_channels; ++ch)
    {
        nb_coefs[ch] = n;
    }
    /* complex coding */

    if (s->use_noise_coding)
    {

        for(ch = 0; ch < s->nb_channels; ++ch)
        {
            if (s->channel_coded[ch])
            {
                int i, n, a;
                n = s->exponent_high_sizes[bsize];
                for(i=0;i<n;++i)
                {
                    a = get_bits(&s->gb, 1);
                    high_band_coded[ch][i] = a;
                    /* if noise coding, the coefficients are not transmitted */
                    if (a)
                        nb_coefs[ch] -= s->exponent_high_bands[bsize][i];
                }
            }
        }
        for(ch = 0; ch < s->nb_channels; ++ch)
        {
            if (s->channel_coded[ch])
            {
                int i, n, val, code;

                n = s->exponent_high_sizes[bsize];
                val = (int)0x80000000;
                for(i=0;i<n;++i)
                {
                    if (high_band_coded[ch][i])
                    {
                        if (val == (int)0x80000000)
                        {
                            val = get_bits(&s->gb, 7) - 19;
                        }
                        else
                        {
                            code = get_vlc(&s->gb, &s->hgain_vlc);
                            if (code < 0)
                            {
                                return -1;
                            }
                            val += code - 18;
                        }
                        high_band_values[ch][i] = val;
                    }
                }
            }
        }
    }

    /* exposant can be interpolated in short blocks. */
    parse_exponents = 1;
    if (s->block_len_bits != s->frame_len_bits)
    {
        parse_exponents = get_bits(&s->gb, 1);
    }

    if (parse_exponents)
    {

        for(ch = 0; ch < s->nb_channels; ++ch)
        {
            if (s->channel_coded[ch])
            {
                if (s->use_exp_vlc)
                {
                    if (decode_exp_vlc(s, ch) < 0)
                    {
                        return -1;
                    }
                }
                else
                {
                    decode_exp_lsp(s, ch);
                }
            }
        }
    }
    else
    {
        for(ch = 0; ch < s->nb_channels; ++ch)
        {
            if (s->channel_coded[ch])
            {
                interpolate_array(s->exponents[ch],
                                  1 << s->prev_block_len_bits,
                                  s->block_len);
            }
        }
    }

    /* parse spectral coefficients : just RLE encoding */
    for(ch = 0; ch < s->nb_channels; ++ch)
    {
        if (s->channel_coded[ch])
        {
            VLC *coef_vlc;
            int level, run, sign, tindex;
            int16_t *ptr, *eptr;
            const int16_t *level_table, *run_table;

            /* special VLC tables are used for ms stereo because
               there is potentially less energy there */
            tindex = (ch == 1 && s->ms_stereo);
            coef_vlc = &s->coef_vlc[tindex];
            run_table = s->run_table[tindex];
            level_table = s->level_table[tindex];
            /* XXX: optimize */
            ptr = &s->coefs1[ch][0];
            eptr = ptr + nb_coefs[ch];
            memset(ptr, 0, s->block_len * sizeof(int16_t));

            for(;;)
            {
                code = get_vlc(&s->gb, coef_vlc);
                if (code < 0)
                {
                    return -1;
                }
                if (code == 1)
                {
                    /* EOB */
                    break;
                }
                else if (code == 0)
                {
                    /* escape */
                    level = get_bits(&s->gb, coef_nb_bits);
                    /* NOTE: this is rather suboptimal. reading
                       block_len_bits would be better */
                    run = get_bits(&s->gb, s->frame_len_bits);
                }
                else
                {
                    /* normal code */
                    run = run_table[code];
                    level = level_table[code];
                }
                sign = get_bits(&s->gb, 1);
                if (!sign)
                    level = -level;
                ptr += run;
                if (ptr >= eptr)
                {
                    return -1;
                }
                *ptr++ = level;


                /* NOTE: EOB can be omitted */
                if (ptr >= eptr)
                    break;
            }
        }
        if (s->version == 1 && s->nb_channels >= 2)
        {
            align_get_bits(&s->gb);
        }
    }

    {
        int n4 = s->block_len >> 1;
        mdct_norm = 0x10000>>(s->block_len_bits-1);		//theres no reason to do a divide by two in fixed precision ...

        if (s->version == 1)
        {
            fixed32 tmp = fixsqrt32(itofix32(n4));
            mdct_norm *= tmp; // PJJ : exercise this path
        }
    }

    /* finally compute the MDCT coefficients */
    for(ch = 0; ch < s->nb_channels; ++ch)
    {
        if (s->channel_coded[ch])
        {
            int16_t *coefs1;
            fixed32 *exponents, *exp_ptr;
            fixed32 *coefs, atemp;
            fixed64 mult;
            fixed64 mult1;
            fixed32 noise;
            int i, j, n, n1, last_high_band;
            static fixed32 exp_power[HIGH_BAND_MAX_SIZE];

			//total_gain, coefs1, mdctnorm are lossless
            coefs1 = s->coefs1[ch];
            exponents = s->exponents[ch];
            mult = fixdiv64(pow_table[total_gain],Fixed32To64(s->max_exponent[ch]));
            mult = fixmul64byfixed(mult, mdct_norm);//This is actually fixed64*2^16!
            coefs = s->coefs[ch];					//VLC exponenents are used to get MDCT coef here!
			n=0;

            if (s->use_noise_coding)
            {
                mult1 = mult;

                /* very low freqs : noise */
                for(i = 0;i < s->coefs_start; ++i)
                {
                    *coefs++ = fixmul32(fixmul32(s->noise_table[s->noise_index],(*exponents++)),Fixed32From64(mult1));
                    s->noise_index = (s->noise_index + 1) & (NOISE_TAB_SIZE - 1);
                }

                n1 = s->exponent_high_sizes[bsize];

                /* compute power of high bands */
                exp_ptr = exponents +
                          s->high_band_start[bsize] -
                          s->coefs_start;
                last_high_band = 0; /* avoid warning */
                for (j=0;j<n1;++j)
                {
                    n = s->exponent_high_bands[s->frame_len_bits -
                                               s->block_len_bits][j];
                    if (high_band_coded[ch][j])
                    {
                        fixed32 e2, v;
                        e2 = 0;
                        for(i = 0;i < n; ++i)
                        {
                            v = exp_ptr[i];
                            e2 += v * v;
                        }
                        exp_power[j] = fixdiv32(e2,n);
                        last_high_band = j;
                    }
                    exp_ptr += n;
                }

                /* main freqs and high freqs */
                for(j=-1;j<n1;++j)
                {
                    if (j < 0)
                    {
                        n = s->high_band_start[bsize] -
                            s->coefs_start;
                    }
                    else
                    {
                        n = s->exponent_high_bands[s->frame_len_bits -
                                                   s->block_len_bits][j];
                    }
                    if (j >= 0 && high_band_coded[ch][j])
                    {
                        /* use noise with specified power */
                        fixed32 tmp = fixdiv32(exp_power[j],exp_power[last_high_band]);
                        mult1 = (fixed64)fixsqrt32(tmp);
                        /* XXX: use a table */
                        mult1 = mult1 * pow_table[high_band_values[ch][j]];
                        mult1 = fixdiv64(mult1,fixmul32(s->max_exponent[ch],s->noise_mult));
                        mult1 = fixmul64byfixed(mult1,mdct_norm);
                        for(i = 0;i < n; ++i)
                        {
                            noise = s->noise_table[s->noise_index];
                            s->noise_index = (s->noise_index + 1) & (NOISE_TAB_SIZE - 1);
                            *coefs++ = fixmul32(fixmul32(*exponents,noise),Fixed32From64(mult1));
                            ++exponents;
                        }
                    }
                    else
                    {
                        /* coded values + small noise */
                        for(i = 0;i < n; ++i)
                        {
                            // PJJ: check code path
                            noise = s->noise_table[s->noise_index];
                            s->noise_index = (s->noise_index + 1) & (NOISE_TAB_SIZE - 1);
                            *coefs++ = fixmul32(fixmul32(((*coefs1++) + noise),*exponents),mult);
                            ++exponents;
                        }
                    }
                }

                /* very high freqs : noise */
                n = s->block_len - s->coefs_end[bsize];
                mult1 = fixmul32(mult,exponents[-1]);
                for (i = 0; i < n; ++i)
                {
                    *coefs++ = fixmul32(s->noise_table[s->noise_index],Fixed32From64(mult1));
                    s->noise_index = (s->noise_index + 1) & (NOISE_TAB_SIZE - 1);
                }
            }
            else
            {

                /* XXX: optimize more */
                for(i = 0;i < s->coefs_start; ++i)
                    *coefs++ = 0;			//why do we do this step?!
                n = nb_coefs[ch];

                for(i = 0;i < n; ++i)
                {
					atemp = (fixed32)(coefs1[i]*mult>>16);
					*coefs++=fixmul32(atemp,exponents[i]);  //this does not work
                }//coefs1 could underflow?
                n = s->block_len - s->coefs_end[bsize];
                for(i = 0;i < n; ++i)
                    *coefs++ = 0;
            }
        }
    }



    if (s->ms_stereo && s->channel_coded[1])
    {
        fixed32 a, b;
        int i;

        /* nominal case for ms stereo: we do it before mdct */
        /* no need to optimize this case because it should almost
           never happen */
        if (!s->channel_coded[0])
        {
            memset(s->coefs[0], 0, sizeof(fixed32) * s->block_len);
            s->channel_coded[0] = 1;
        }

        for(i = 0; i < s->block_len; ++i)
        {
            a = s->coefs[0][i];
            b = s->coefs[1][i];
            s->coefs[0][i] = a + b;
            s->coefs[1][i] = a - b;
        }
    }

    /* build the window : we ensure that when the windows overlap
       their squared sum is always 1 (MDCT reconstruction rule) */
    /* XXX: merge with output */
    {
        int i, next_block_len, block_len, prev_block_len, n;
        fixed32 *wptr;

        block_len = s->block_len;
        prev_block_len = 1 << s->prev_block_len_bits;
        next_block_len = 1 << s->next_block_len_bits;

        /* right part */
        wptr = window + block_len;
        if (block_len <= next_block_len)
        {
            for(i=0;i<block_len;++i)
                *wptr++ = s->windows[bsize][i];
        }
        else
        {
            /* overlap */
            n = (block_len / 2) - (next_block_len / 2);
            for(i=0;i<n;++i)
                *wptr++ = itofix32(1);
            for(i=0;i<next_block_len;++i)
                *wptr++ = s->windows[s->frame_len_bits - s->next_block_len_bits][i];
            for(i=0;i<n;++i)
                *wptr++ = 0;
        }

        /* left part */
        wptr = window + block_len;
        if (block_len <= prev_block_len)
        {
            for(i=0;i<block_len;++i)
                *--wptr = s->windows[bsize][i];
        }
        else
        {
            /* overlap */
            n = (block_len / 2) - (prev_block_len / 2);
            for(i=0;i<n;++i)
                *--wptr = itofix32(1);
            for(i=0;i<prev_block_len;++i)
                *--wptr = s->windows[s->frame_len_bits - s->prev_block_len_bits][i];
            for(i=0;i<n;++i)
                *--wptr = 0;
        }
    }


    for(ch = 0; ch < s->nb_channels; ++ch)
    {
        if (s->channel_coded[ch])
        {
            fixed32 output[BLOCK_MAX_SIZE * 2];
            fixed32 *ptr;
            int i, n4, index, n;

            n = s->block_len;
            n4 = s->block_len >>1;

            ff_imdct_calc_wma(&s->mdct_ctx[bsize],
                          output,
                          s->coefs[ch],
                          s->mdct_tmp);

            /* XXX: optimize all that by build the window and
               multipying/adding at the same time */
            /* multiply by the window */
            for(i=0;i<n * 2;++i)
            {
                output[i] = fixmul32(output[i], window[i]);
                //output[i] *= window[i];
            }

            /* add in the frame */
            index = (s->frame_len / 2) + s->block_pos - n4;
            ptr = &s->frame_out[ch][index];

            for(i=0;i<n * 2;++i)
            {
                *ptr += output[i];
                ++ptr;
            }

            /* specific fast case for ms-stereo : add to second
               channel if it is not coded */
            if (s->ms_stereo && !s->channel_coded[1])
            {
                ptr = &s->frame_out[1][index];
                for(i=0;i<n * 2;++i)
                {
                    *ptr += output[i];
                    ++ptr;
                }
            }
        }
    }
next:
    /* update block number */
    ++s->block_num;
    s->block_pos += s->block_len;
    if (s->block_pos >= s->frame_len)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/* decode a frame of frame_len samples */
static int wma_decode_frame(WMADecodeContext *s, int16_t *samples)
{
    int ret, i, n, a, ch, incr;
    int16_t *ptr;
    fixed32 *iptr;
    /* read each block */
    s->block_num = 0;
    s->block_pos = 0;
    for(;;)
    {
        ret = wma_decode_block(s);
        if (ret < 0)
        {
            return -1;
        }
        if (ret)
        {
            break;
        }
    }

    /* convert frame to integer */
    n = s->frame_len;
    incr = s->nb_channels;
    for(ch = 0; ch < s->nb_channels; ++ch)
    {
        ptr = samples + ch;
        iptr = s->frame_out[ch];

        for (i=0;i<n;++i)
        {
            a = fixtoi32(*iptr++)<<1;		//ugly but good enough for now

            if (a > 32767)
            {
                a = 32767;
            }
            else if (a < -32768)
            {
                a = -32768;
            }
            *ptr = a;
            ptr += incr;
        }
        /* prepare for next block */
        memmove(&s->frame_out[ch][0], &s->frame_out[ch][s->frame_len],
                s->frame_len * sizeof(fixed32));
        /* XXX: suppress this */
        memset(&s->frame_out[ch][s->frame_len], 0,
               s->frame_len * sizeof(fixed32));
    }

    return 0;
}

static int wma_decode_superframe(CodecContext *avctx,
                                 void *data,
                                 int *data_size,
                                 uint8_t *buf,
                                 int buf_size)
{
    WMADecodeContext *s = avctx->priv_data;
    int nb_frames, bit_offset, i, pos, len;
    uint8_t *q;
    int16_t *samples;
    if (buf_size==0)
    {
        s->last_superframe_len = 0;
        return 0;
    }



    samples = data;
    init_get_bits(&s->gb, buf, buf_size*8);
    if (s->use_bit_reservoir)
    {
        /* read super frame header */
        get_bits(&s->gb, 4); /* super frame index */
        nb_frames = get_bits(&s->gb, 4) - 1;

        bit_offset = get_bits(&s->gb, s->byte_offset_bits + 3);
        if (s->last_superframe_len > 0)
        {
            /* add bit_offset bits to last frame */
            if ((s->last_superframe_len + ((bit_offset + 7) >> 3)) >
                    MAX_CODED_SUPERFRAME_SIZE)
            {
                goto fail;
            }
            q = s->last_superframe + s->last_superframe_len;
            len = bit_offset;
            while (len > 0)
            {
                *q++ = (get_bits)(&s->gb, 8);
                len -= 8;
            }
            if (len > 0)
            {
                *q++ = (get_bits)(&s->gb, len) << (8 - len);
            }

            /* XXX: bit_offset bits into last frame */
            init_get_bits(&s->gb, s->last_superframe, MAX_CODED_SUPERFRAME_SIZE*8);
            /* skip unused bits */
            if (s->last_bitoffset > 0)
                skip_bits(&s->gb, s->last_bitoffset);
            /* this frame is stored in the last superframe and in the
               current one */
            if (wma_decode_frame(s, samples) < 0)
            {
                goto fail;
            }
            samples += s->nb_channels * s->frame_len;
        }

        /* read each frame starting from bit_offset */
        pos = bit_offset + 4 + 4 + s->byte_offset_bits + 3;
        init_get_bits(&s->gb, buf + (pos >> 3), (MAX_CODED_SUPERFRAME_SIZE - (pos >> 3))*8);
        len = pos & 7;
        if (len > 0)
            skip_bits(&s->gb, len);

        s->reset_block_lengths = 1;
        for(i=0;i<nb_frames;++i)
        {
            if (wma_decode_frame(s, samples) < 0)
            {
                goto fail;
            }
            samples += s->nb_channels * s->frame_len;
        }

        /* we copy the end of the frame in the last frame buffer */
        pos = get_bits_count(&s->gb) + ((bit_offset + 4 + 4 + s->byte_offset_bits + 3) & ~7);
        s->last_bitoffset = pos & 7;
        pos >>= 3;
        len = buf_size - pos;
        if (len > MAX_CODED_SUPERFRAME_SIZE || len < 0)
        {
            goto fail;
        }
        s->last_superframe_len = len;
        memcpy(s->last_superframe, buf + pos, len);
    }
    else
    {
        /* single frame decode */
        if (wma_decode_frame(s, samples) < 0)
        {
            goto fail;
        }
        samples += s->nb_channels * s->frame_len;
    }
    *data_size = (int8_t *)samples - (int8_t *)data;
    return s->block_align;
fail:
    /* when error, we reset the bit reservoir */
    s->last_superframe_len = 0;
    return -1;
}


static int wma_decode_end(CodecContext *avctx)
{
    return 0;
}

AVCodec wmav1_decoder =
    {
        "wmav1",
        CODEC_TYPE_AUDIO,
        CODEC_ID_WMAV1,
        sizeof(WMADecodeContext),
        wma_decode_init_fixed,
        NULL,
        wma_decode_end,
        wma_decode_superframe,
        0,
        0,
        0,
        0,
        0
    };

AVCodec wmav2_decoder =
    {
        "wmav2",
        CODEC_TYPE_AUDIO,
        CODEC_ID_WMAV2,
        sizeof(WMADecodeContext),
        wma_decode_init_fixed,
        NULL,
        wma_decode_end,
        wma_decode_superframe,
        0,
        0,
        0,
        0,
        0
    };
