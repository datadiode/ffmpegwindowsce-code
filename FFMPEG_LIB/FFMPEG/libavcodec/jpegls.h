/*
 * JPEG-LS common code
 * Copyright (c) 2003 Michael Niedermayer
 * Copyright (c) 2006 Konstantin Shishkov
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
 * @file jpegls.h
 * JPEG-LS common code.
 */

#ifndef JPEGLS_H
#define JPEGLS_H

#include "avcodec.h"

typedef struct JpeglsContext{
    AVCodecContext *avctx;
    AVFrame picture;
}JpeglsContext;

typedef struct JLSState{
    int T1, T2, T3;
    int A[367], B[367], C[365], N[367];
    int limit, reset, bpp, qbpp, maxval, range;
#ifdef UNDER_CE
    int Near,twonear;
#else
    int near,twonear;
#endif
    int run_index[3];
}JLSState;

extern const uint8_t ff_log2_run[32];

/**
 * Calculate initial JPEG-LS parameters
 */
void ff_jpegls_init_state(JLSState *state);

/**
 * Calculate quantized gradient value, used for context determination
 */
static inline int ff_jpegls_quantize(JLSState *s, int v){ //FIXME optimize
    if(v==0) return 0;
    if(v < 0){
        if(v <= -s->T3) return -4;
        if(v <= -s->T2) return -3;
        if(v <= -s->T1) return -2;
#ifdef UNDER_CE
        if(v <  -s->Near) return -1;
#else
        if(v <  -s->near) return -1;
#endif
        return 0;
    }else{
#ifdef UNDER_CE
        if(v <= s->Near) return 0;
#else
        if(v <= s->near) return 0;
#endif
        if(v <  s->T1) return 1;
        if(v <  s->T2) return 2;
        if(v <  s->T3) return 3;
        return 4;
    }
}

/**
 * Calculate JPEG-LS codec values
 */
void ff_jpegls_reset_coding_parameters(JLSState *s, int reset_all);


static inline void ff_jpegls_downscale_state(JLSState *state, int Q){
    if(state->N[Q] == state->reset){
        state->A[Q] >>=1;
        state->B[Q] >>=1;
        state->N[Q] >>=1;
    }
    state->N[Q]++;
}

static inline int ff_jpegls_update_state_regular(JLSState *state, int Q, int err){
    state->A[Q] += FFABS(err);
    err *= state->twonear;
    state->B[Q] += err;

    ff_jpegls_downscale_state(state, Q);

    if(state->B[Q] <= -state->N[Q]) {
        state->B[Q]= FFMAX(state->B[Q] + state->N[Q], 1-state->N[Q]);
        if(state->C[Q] > -128)
            state->C[Q]--;
    }else if(state->B[Q] > 0){
        state->B[Q]= FFMIN(state->B[Q] - state->N[Q], 0);
        if(state->C[Q] < 127)
            state->C[Q]++;
    }

    return err;
}

#define R(a, i   ) (bits == 8 ?  ((uint8_t*)(a))[i]    :  ((uint16_t*)(a))[i]  )
#define W(a, i, v) (bits == 8 ? (((uint8_t*)(a))[i]=v) : (((uint16_t*)(a))[i]=v))

#endif /* JPEGLS_H */
