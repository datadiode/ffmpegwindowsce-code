/*
 * Interplay C93 video decoder
 * Copyright (c) 2007 Anssi Hannula <anssi.hannula@gmail.com>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#include "avcodec.h"
#include "bytestream.h"
#include <stddef.h>

typedef struct {
    AVFrame pictures[2];
    int currentpic;
} C93DecoderContext;

typedef enum {
    C93_8X8_FROM_PREV  = 0x02,
    C93_4X4_FROM_PREV  = 0x06,
    C93_4X4_FROM_CURR  = 0x07,
    C93_8X8_2COLOR     = 0x08,
    C93_4X4_2COLOR     = 0x0A,
    C93_4X4_4COLOR_GRP = 0x0B,
    C93_4X4_4COLOR     = 0x0D,
    C93_NOOP           = 0x0E,
    C93_8X8_INTRA      = 0x0F,
} C93BlockType;

#define WIDTH   320
#define HEIGHT  192

#define C93_HAS_PALETTE 0x01
#define C93_FIRST_FRAME 0x02

static int decode_init(AVCodecContext *avctx)
{
    avctx->pix_fmt = PIX_FMT_PAL8;
    return 0;
}

static int decode_end(AVCodecContext *avctx)
{
    C93DecoderContext * const c93 = avctx->priv_data;

    if (c93->pictures[0].data[0])
        avctx->release_buffer(avctx, &c93->pictures[0]);
    if (c93->pictures[1].data[0])
        avctx->release_buffer(avctx, &c93->pictures[1]);
    return 0;
}

static inline int copy_block(AVCodecContext *avctx, uint8_t *to,
        uint8_t *from, int offset, int height, int stride)
{
    int i;
    int width = height;
    int from_x = offset % WIDTH;
    int from_y = offset / WIDTH;
    int overflow = from_x + width - WIDTH;

    if (!from) {
        /* silently ignoring predictive blocks in first frame */
        return 0;
    }

    if (from_y + height > HEIGHT) {
        av_log(avctx, AV_LOG_ERROR, "invalid offset %d during C93 decoding\n",
               offset);
        return -1;
    }

    if (overflow > 0) {
        width -= overflow;
        for (i = 0; i < height; i++) {
            memcpy(&to[i*stride+width], &from[(from_y+i)*stride], overflow);
        }
    }

    for (i = 0; i < height; i++) {
        memcpy(&to[i*stride], &from[(from_y+i)*stride+from_x], width);
    }

    return 0;
}

static inline void draw_n_color(uint8_t *out, int stride, int width,
         int height, int bpp, uint8_t cols[4], uint8_t grps[4], uint32_t col)
{
    int x, y;
    for (y = 0; y < height; y++) {
        if (grps)
            cols[0] = grps[3 * (y >> 1)];
        for (x = 0; x < width; x++) {
            if (grps)
                cols[1]= grps[(x >> 1) + 1];
            out[x + y*stride] = cols[col & ((1 << bpp) - 1)];
            col >>= bpp;
        }
    }
}

static int decode_frame(AVCodecContext *avctx, void *data,
                            int *data_size, uint8_t * buf, int buf_size)
{
    C93DecoderContext * const c93 = avctx->priv_data;
    AVFrame * const newpic = &c93->pictures[c93->currentpic];
    AVFrame * const oldpic = &c93->pictures[c93->currentpic^1];
    AVFrame *picture = data;
    uint8_t *out;
    int stride, i, x, y, bt = 0;

    c93->currentpic ^= 1;

    newpic->reference = 1;
    newpic->buffer_hints = FF_BUFFER_HINTS_VALID | FF_BUFFER_HINTS_PRESERVE |
                         FF_BUFFER_HINTS_REUSABLE | FF_BUFFER_HINTS_READABLE;
    if (avctx->reget_buffer(avctx, newpic)) {
        av_log(avctx, AV_LOG_ERROR, "reget_buffer() failed\n");
        return -1;
    }

    stride = newpic->linesize[0];

    if (buf[0] & C93_FIRST_FRAME) {
        newpic->pict_type = FF_I_TYPE;
        newpic->key_frame = 1;
    } else {
        newpic->pict_type = FF_P_TYPE;
        newpic->key_frame = 0;
    }

    if (*buf++ & C93_HAS_PALETTE) {
        uint32_t *palette = (uint32_t *) newpic->data[1];
        uint8_t *palbuf = buf + buf_size - 768 - 1;
        for (i = 0; i < 256; i++) {
            palette[i] = bytestream_get_be24(&palbuf);
        }
    } else {
        if (oldpic->data[1])
            memcpy(newpic->data[1], oldpic->data[1], 256 * 4);
    }

    for (y = 0; y < HEIGHT; y += 8) {
        out = newpic->data[0] + y * stride;
        for (x = 0; x < WIDTH; x += 8) {
            uint8_t *copy_from = oldpic->data[0];
            unsigned int offset, j;
            uint8_t cols[4], grps[4];
            C93BlockType block_type;

            if (!bt)
                bt = *buf++;

            block_type= bt & 0x0F;
            switch (block_type) {
            case C93_8X8_FROM_PREV:
                offset = bytestream_get_le16(&buf);
                if (copy_block(avctx, out, copy_from, offset, 8, stride))
                    return -1;
                break;

            case C93_4X4_FROM_CURR:
                copy_from = newpic->data[0];
            case C93_4X4_FROM_PREV:
                for (j = 0; j < 8; j += 4) {
                    for (i = 0; i < 8; i += 4) {
                        offset = bytestream_get_le16(&buf);
                        if (copy_block(avctx, &out[j*stride+i],
                                           copy_from, offset, 4, stride))
                            return -1;
                    }
                }
                break;

            case C93_8X8_2COLOR:
                bytestream_get_buffer(&buf, cols, 2);
                for (i = 0; i < 8; i++) {
                    draw_n_color(out + i*stride, stride, 8, 1, 1, cols,
                                     NULL, *buf++);
                }

                break;

            case C93_4X4_2COLOR:
            case C93_4X4_4COLOR:
            case C93_4X4_4COLOR_GRP:
                for (j = 0; j < 8; j += 4) {
                    for (i = 0; i < 8; i += 4) {
                        if (block_type == C93_4X4_2COLOR) {
                            bytestream_get_buffer(&buf, cols, 2);
                            draw_n_color(out + i + j*stride, stride, 4, 4,
                                    1, cols, NULL, bytestream_get_le16(&buf));
                        } else if (block_type == C93_4X4_4COLOR) {
                            bytestream_get_buffer(&buf, cols, 4);
                            draw_n_color(out + i + j*stride, stride, 4, 4,
                                    2, cols, NULL, bytestream_get_le32(&buf));
                        } else {
                            bytestream_get_buffer(&buf, grps, 4);
                            draw_n_color(out + i + j*stride, stride, 4, 4,
                                    1, cols, grps, bytestream_get_le16(&buf));
                        }
                    }
                }
                break;

            case C93_NOOP:
                break;

            case C93_8X8_INTRA:
                for (j = 0; j < 8; j++)
                    bytestream_get_buffer(&buf, out + j*stride, 8);
                break;

            default:
                av_log(avctx, AV_LOG_ERROR, "unexpected type %x at %dx%d\n",
                       block_type, x, y);
                return -1;
            }
            bt >>= 4;
            out += 8;
        }
    }

    *picture = *newpic;
    *data_size = sizeof(AVFrame);

    return buf_size;
}

AVCodec c93_decoder = {
    "c93",
    CODEC_TYPE_VIDEO,
    CODEC_ID_C93,
    sizeof(C93DecoderContext),
    decode_init,
    NULL,
    decode_end,
    decode_frame,
    CODEC_CAP_DR1,
};
