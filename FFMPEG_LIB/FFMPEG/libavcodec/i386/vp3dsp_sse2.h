/*
 * vp3dsp sse2 functions declarations
 * Copyright (c) 2007 Aurelien Jacobs <aurel@gnuage.org>
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

#ifndef VP3DSP_SSE2_H
#define VP3DSP_SSE2_H

#include "dsputil.h"

void ff_vp3_idct_sse2(int16_t *input_data);
void ff_vp3_idct_put_sse2(uint8_t *dest, int line_size, DCTELEM *block);
void ff_vp3_idct_add_sse2(uint8_t *dest, int line_size, DCTELEM *block);

#endif /* VP3DSP_SSE2_H */
