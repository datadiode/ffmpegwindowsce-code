/*
 * Intel Indeo 2 codec
 * Copyright (c) 2005 Konstantin Shishkov
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
 *
 */

/**
 * @file indeo2.c
 * Intel Indeo 2 decoder.
 */
#define ALT_BITSTREAM_READER_LE
#include "avcodec.h"
#include "bitstream.h"
#include "indeo2data.h"

typedef struct Ir2Context{
    AVCodecContext *avctx;
    AVFrame picture;
    GetBitContext gb;
    int decode_delta;
} Ir2Context;

#define CODE_VLC_BITS 14
static VLC ir2_vlc;

/* Indeo 2 codes are in range 0x01..0x7F and 0x81..0x90 */
static inline int ir2_get_code(GetBitContext *gb)
{
    return get_vlc2(gb, ir2_vlc.table, CODE_VLC_BITS, 1) + 1;
}

static int ir2_decode_plane(Ir2Context *ctx, int width, int height, uint8_t *dst, int stride,
                             const uint8_t *table)
{
    int i;
    int j;
    int out = 0;
    int c;
    int t;

    if(width&1)
        return -1;

    /* first line contain absolute values, other lines contain deltas */
    while (out < width){
        c = ir2_get_code(&ctx->gb);
        if(c >= 0x80) { /* we have a run */
            c -= 0x7F;
            if(out + c*2 > width)
                return -1;
            for (i = 0; i < c * 2; i++)
                dst[out++] = 0x80;
        } else { /* copy two values from table */
            dst[out++] = table[c * 2];
            dst[out++] = table[(c * 2) + 1];
        }
    }
    dst += stride;

    for (j = 1; j < height; j++){
        out = 0;
        while (out < width){
            c = ir2_get_code(&ctx->gb);
            if(c >= 0x80) { /* we have a skip */
                c -= 0x7F;
                if(out + c*2 > width)
                    return -1;
                for (i = 0; i < c * 2; i++) {
                    dst[out] = dst[out - stride];
                    out++;
                }
            } else { /* add two deltas from table */
                t = dst[out - stride] + (table[c * 2] - 128);
                t= av_clip_uint8(t);
                dst[out] = t;
                out++;
                t = dst[out - stride] + (table[(c * 2) + 1] - 128);
                t= av_clip_uint8(t);
                dst[out] = t;
                out++;
            }
        }
        dst += stride;
    }
    return 0;
}

static int ir2_decode_plane_inter(Ir2Context *ctx, int width, int height, uint8_t *dst, int stride,
                             const uint8_t *table)
{
    int j;
    int out = 0;
    int c;
    int t;

    if(width&1)
        return -1;

    for (j = 0; j < height; j++){
        out = 0;
        while (out < width){
            c = ir2_get_code(&ctx->gb);
            if(c >= 0x80) { /* we have a skip */
                c -= 0x7F;
                out += c * 2;
            } else { /* add two deltas from table */
                t = dst[out] + (((table[c * 2] - 128)*3) >> 2);
                t= av_clip_uint8(t);
                dst[out] = t;
                out++;
                t = dst[out] + (((table[(c * 2) + 1] - 128)*3) >> 2);
                t= av_clip_uint8(t);
                dst[out] = t;
                out++;
            }
        }
        dst += stride;
    }
    return 0;
}

static int ir2_decode_frame(AVCodecContext *avctx,
                        void *data, int *data_size,
                        uint8_t *buf, int buf_size)
{
    Ir2Context * const s = avctx->priv_data;
    AVFrame *picture = data;
    AVFrame * const p= (AVFrame*)&s->picture;
    int start;

    if(p->data[0])
        avctx->release_buffer(avctx, p);

    p->reference = 1;
    p->buffer_hints = FF_BUFFER_HINTS_VALID | FF_BUFFER_HINTS_PRESERVE | FF_BUFFER_HINTS_REUSABLE;
    if (avctx->reget_buffer(avctx, p)) {
        av_log(s->avctx, AV_LOG_ERROR, "reget_buffer() failed\n");
        return -1;
    }

    s->decode_delta = buf[18];

    /* decide whether frame uses deltas or not */
#ifndef ALT_BITSTREAM_READER_LE
    for (i = 0; i < buf_size; i++)
        buf[i] = ff_reverse[buf[i]];
#endif
    start = 48; /* hardcoded for now */

    init_get_bits(&s->gb, buf + start, buf_size - start);

    if (s->decode_delta) { /* intraframe */
        ir2_decode_plane(s, avctx->width, avctx->height,
                         s->picture.data[0], s->picture.linesize[0], ir2_luma_table);
        /* swapped U and V */
        ir2_decode_plane(s, avctx->width >> 2, avctx->height >> 2,
                         s->picture.data[2], s->picture.linesize[2], ir2_luma_table);
        ir2_decode_plane(s, avctx->width >> 2, avctx->height >> 2,
                         s->picture.data[1], s->picture.linesize[1], ir2_luma_table);
    } else { /* interframe */
        ir2_decode_plane_inter(s, avctx->width, avctx->height,
                         s->picture.data[0], s->picture.linesize[0], ir2_luma_table);
        /* swapped U and V */
        ir2_decode_plane_inter(s, avctx->width >> 2, avctx->height >> 2,
                         s->picture.data[2], s->picture.linesize[2], ir2_luma_table);
        ir2_decode_plane_inter(s, avctx->width >> 2, avctx->height >> 2,
                         s->picture.data[1], s->picture.linesize[1], ir2_luma_table);
    }

    *picture= *(AVFrame*)&s->picture;
    *data_size = sizeof(AVPicture);

    return buf_size;
}

static int ir2_decode_init(AVCodecContext *avctx){
    Ir2Context * const ic = avctx->priv_data;

    ic->avctx = avctx;

    avctx->pix_fmt= PIX_FMT_YUV410P;

    if (!ir2_vlc.table)
        init_vlc(&ir2_vlc, CODE_VLC_BITS, IR2_CODES,
                 &ir2_codes[0][1], 4, 2,
//#ifdef ALT_BITSTREAM_READER_LE
                 &ir2_codes[0][0], 4, 2, INIT_VLC_USE_STATIC | INIT_VLC_LE);
//#else
                 //&ir2_codes[0][0], 4, 2, INIT_VLC_USE_STATIC);
//#endif

    return 0;
}

AVCodec indeo2_decoder = {
    "indeo2",
    CODEC_TYPE_VIDEO,
    CODEC_ID_INDEO2,
    sizeof(Ir2Context),
    ir2_decode_init,
    NULL,
    NULL,
    ir2_decode_frame,
    CODEC_CAP_DR1,
};
