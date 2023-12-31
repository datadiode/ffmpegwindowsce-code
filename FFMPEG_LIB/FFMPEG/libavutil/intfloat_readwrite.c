/*
 * portable IEEE float/double read/write functions
 *
 * Copyright (c) 2005 Michael Niedermayer <michaelni@gmx.at>
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file intfloat_readwrite.c
 * Portable IEEE float/double read/write functions.
 */

#include "common.h"
#include "intfloat_readwrite.h"
#include <assert.h>
#include <math.h>

double av_int2dbl(int64_t v){
	
#ifdef UNDER_CE
	if(v+v > (uint64_t)0xFFE<<52)
#else
	if(v+v > 0xFFEULL<<52)
#endif
	{
		union
		{
			double d;
			int64_t x;
		} v;

		v.x = 0x7FF80000;	// gcc returns this for 0.0 / 0.0
		
		return v.d;
	}
		
#ifdef UNDER_CE
	return ldexp(((v&(((int64_t)1<<52)-1)) + ((int64_t)1<<52)) * (v>>63|1), (v>>52&0x7FF)-1075);
#else
	return ldexp(((v&((1LL<<52)-1)) + (1LL<<52)) * (v>>63|1), (v>>52&0x7FF)-1075);
#endif
}

float av_int2flt(int32_t v){
    if(v+v > 0xFF000000U)
	{
		union
		{
			double d;
			int64_t x;
		} v;

		v.x = 0x7FF80000;	// gcc returns this for 0.0 / 0.0
		
		return v.d;
	}

	return ldexp(((v&0x7FFFFF) + (1<<23)) * (v>>31|1), (v>>23&0xFF)-150);
}

double av_ext2dbl(const AVExtFloat ext){
    uint64_t m = 0;
    int e, i;

    for (i = 0; i < 8; i++)
        m = (m<<8) + ext.mantissa[i];
    e = (((int)ext.exponent[0]&0x7f)<<8) | ext.exponent[1];
    if (e == 0x7fff && m)
	{
		union
		{
			double d;
			int64_t x;
		} v;

		v.x = 0x7FF80000;	// gcc returns this for 0.0 / 0.0
		
		return v.d;
	}

	e -= 16383 + 63;        /* In IEEE 80 bits, the whole (i.e. 1.xxxx)
                             * mantissa bit is written as opposed to the
                             * single and double precision formats */
    if (ext.exponent[0]&0x80)
        m= -m;
    return ldexp(m, e);
}

int64_t av_dbl2int(double d){
    int e;
    if     ( !d) return 0;
#ifdef UNDER_CE
    else if(d-d) return (int64_t)0x7FF0000000000000 + ((int64_t)(d<0)<<63) + (d!=d);
#else
    else if(d-d) return 0x7FF0000000000000LL + ((int64_t)(d<0)<<63) + (d!=d);
#endif
    d= frexp(d, &e);
#ifdef UNDER_CE
    return (int64_t)(d<0)<<63 | (e+(int64_t)1022)<<52 | (int64_t)((fabs(d)-0.5)*((int64_t)1<<53));
#else
    return (int64_t)(d<0)<<63 | (e+1022LL)<<52 | (int64_t)((fabs(d)-0.5)*(1LL<<53));
#endif
}

int32_t av_flt2int(float d){
    int e;
    if     ( !d) return 0;
    else if(d-d) return 0x7F800000 + ((d<0)<<31) + (d!=d);
    d= frexp(d, &e);
    return (d<0)<<31 | (e+126)<<23 | (int64_t)((fabs(d)-0.5)*(1<<24));
}

AVExtFloat av_dbl2ext(double d){
    struct AVExtFloat ext= {{0}};
    int e, i; double f; uint64_t m;

    f = fabs(frexp(d, &e));
    if (f >= 0.5 && f < 1) {
        e += 16382;
        ext.exponent[0] = e>>8;
        ext.exponent[1] = e;
        m = (uint64_t)ldexp(f, 64);
        for (i=0; i < 8; i++)
            ext.mantissa[i] = m>>(56-(i<<3));
    } else if (f != 0.0) {
        ext.exponent[0] = 0x7f; ext.exponent[1] = 0xff;
		assert(0);
		//if (f != 1/0.0)
        //    ext.mantissa[0] = ~0;
    }
    if (d < 0)
        ext.exponent[0] |= 0x80;
    return ext;
}

