/*
 * DVB subtitle decoding for ffmpeg
 * Copyright (c) 2005 Ian Caulfield.
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
#include "avcodec.h"
#include "dsputil.h"
#include "bitstream.h"

//#define DEBUG
//#define DEBUG_PACKET_CONTENTS
//#define DEBUG_SAVE_IMAGES

#define DVBSUB_PAGE_SEGMENT     0x10
#define DVBSUB_REGION_SEGMENT   0x11
#define DVBSUB_CLUT_SEGMENT     0x12
#define DVBSUB_OBJECT_SEGMENT   0x13
#define DVBSUB_DISPLAY_SEGMENT  0x80

#define cm (ff_cropTbl + MAX_NEG_CROP)

#ifdef DEBUG_SAVE_IMAGES
#undef fprintf
#if 0
static void png_save(const char *filename, uint8_t *bitmap, int w, int h,
                     uint32_t *rgba_palette)
{
    int x, y, v;
    FILE *f;
    char fname[40], fname2[40];
    char command[1024];

    snprintf(fname, 40, "%s.ppm", filename);

    f = fopen(fname, "w");
    if (!f) {
        perror(fname);
        exit(1);
    }
    fprintf(f, "P6\n"
            "%d %d\n"
            "%d\n",
            w, h, 255);
    for(y = 0; y < h; y++) {
        for(x = 0; x < w; x++) {
            v = rgba_palette[bitmap[y * w + x]];
            putc((v >> 16) & 0xff, f);
            putc((v >> 8) & 0xff, f);
            putc((v >> 0) & 0xff, f);
        }
    }
    fclose(f);


    snprintf(fname2, 40, "%s-a.pgm", filename);

    f = fopen(fname2, "w");
    if (!f) {
        perror(fname2);
        exit(1);
    }
    fprintf(f, "P5\n"
            "%d %d\n"
            "%d\n",
            w, h, 255);
    for(y = 0; y < h; y++) {
        for(x = 0; x < w; x++) {
            v = rgba_palette[bitmap[y * w + x]];
            putc((v >> 24) & 0xff, f);
        }
    }
    fclose(f);

    snprintf(command, 1024, "pnmtopng -alpha %s %s > %s.png 2> /dev/null", fname2, fname, filename);
    system(command);

    snprintf(command, 1024, "rm %s %s", fname, fname2);
    system(command);
}
#endif

static void png_save2(const char *filename, uint32_t *bitmap, int w, int h)
{
    int x, y, v;
    FILE *f;
    char fname[40], fname2[40];
    char command[1024];

    snprintf(fname, 40, "%s.ppm", filename);

    f = fopen(fname, "w");
    if (!f) {
        perror(fname);
        exit(1);
    }
    fprintf(f, "P6\n"
            "%d %d\n"
            "%d\n",
            w, h, 255);
    for(y = 0; y < h; y++) {
        for(x = 0; x < w; x++) {
            v = bitmap[y * w + x];
            putc((v >> 16) & 0xff, f);
            putc((v >> 8) & 0xff, f);
            putc((v >> 0) & 0xff, f);
        }
    }
    fclose(f);


    snprintf(fname2, 40, "%s-a.pgm", filename);

    f = fopen(fname2, "w");
    if (!f) {
        perror(fname2);
        exit(1);
    }
    fprintf(f, "P5\n"
            "%d %d\n"
            "%d\n",
            w, h, 255);
    for(y = 0; y < h; y++) {
        for(x = 0; x < w; x++) {
            v = bitmap[y * w + x];
            putc((v >> 24) & 0xff, f);
        }
    }
    fclose(f);

    snprintf(command, 1024, "pnmtopng -alpha %s %s > %s.png 2> /dev/null", fname2, fname, filename);
    system(command);

    snprintf(command, 1024, "rm %s %s", fname, fname2);
    system(command);
}
#endif

#define RGBA(r,g,b,a) (((a) << 24) | ((r) << 16) | ((g) << 8) | (b))

typedef struct DVBSubCLUT {
    int id;

    uint32_t clut4[4];
    uint32_t clut16[16];
    uint32_t clut256[256];

    struct DVBSubCLUT *next;
} DVBSubCLUT;

static DVBSubCLUT default_clut;

typedef struct DVBSubObjectDisplay {
    int object_id;
    int region_id;

    int x_pos;
    int y_pos;

    int fgcolour;
    int bgcolour;

    struct DVBSubObjectDisplay *region_list_next;
    struct DVBSubObjectDisplay *object_list_next;
} DVBSubObjectDisplay;

typedef struct DVBSubObject {
    int id;

    int type;

    DVBSubObjectDisplay *display_list;

    struct DVBSubObject *next;
} DVBSubObject;

typedef struct DVBSubRegionDisplay {
    int region_id;

    int x_pos;
    int y_pos;

    struct DVBSubRegionDisplay *next;
} DVBSubRegionDisplay;

typedef struct DVBSubRegion {
    int id;

    int width;
    int height;
    int depth;

    int clut;
    int bgcolour;

    uint8_t *pbuf;
    int buf_size;

    DVBSubObjectDisplay *display_list;

    struct DVBSubRegion *next;
} DVBSubRegion;

typedef struct DVBSubContext {
    int composition_id;
    int ancillary_id;

    int time_out;
    DVBSubRegion *region_list;
    DVBSubCLUT   *clut_list;
    DVBSubObject *object_list;

    int display_list_size;
    DVBSubRegionDisplay *display_list;
} DVBSubContext;


static DVBSubObject* get_object(DVBSubContext *ctx, int object_id)
{
    DVBSubObject *ptr = ctx->object_list;

    while (ptr != NULL && ptr->id != object_id) {
        ptr = ptr->next;
    }

    return ptr;
}

static DVBSubCLUT* get_clut(DVBSubContext *ctx, int clut_id)
{
    DVBSubCLUT *ptr = ctx->clut_list;

    while (ptr != NULL && ptr->id != clut_id) {
        ptr = ptr->next;
    }

    return ptr;
}

static DVBSubRegion* get_region(DVBSubContext *ctx, int region_id)
{
    DVBSubRegion *ptr = ctx->region_list;

    while (ptr != NULL && ptr->id != region_id) {
        ptr = ptr->next;
    }

    return ptr;
}

static void delete_region_display_list(DVBSubContext *ctx, DVBSubRegion *region)
{
    DVBSubObject *object, *obj2, **obj2_ptr;
    DVBSubObjectDisplay *display, *obj_disp, **obj_disp_ptr;

    while (region->display_list != NULL) {
        display = region->display_list;

        object = get_object(ctx, display->object_id);

        if (object != NULL) {
            obj_disp = object->display_list;
            obj_disp_ptr = &object->display_list;

            while (obj_disp != NULL && obj_disp != display) {
                obj_disp_ptr = &obj_disp->object_list_next;
                obj_disp = obj_disp->object_list_next;
            }

            if (obj_disp) {
                *obj_disp_ptr = obj_disp->object_list_next;

                if (object->display_list == NULL) {
                    obj2 = ctx->object_list;
                    obj2_ptr = &ctx->object_list;

                    while (obj2 != NULL && obj2 != object) {
                        obj2_ptr = &obj2->next;
                        obj2 = obj2->next;
                    }

                    *obj2_ptr = obj2->next;

                    av_free(obj2);
                }
            }
        }

        region->display_list = display->region_list_next;

        av_free(display);
    }

}

static void delete_state(DVBSubContext *ctx)
{
    DVBSubRegion *region;
    DVBSubCLUT *clut;

    while (ctx->region_list != NULL)
    {
        region = ctx->region_list;

        ctx->region_list = region->next;

        delete_region_display_list(ctx, region);
        if (region->pbuf != NULL)
            av_free(region->pbuf);

        av_free(region);
    }

    while (ctx->clut_list != NULL)
    {
        clut = ctx->clut_list;

        ctx->clut_list = clut->next;

        av_free(clut);
    }

    /* Should already be null */
    if (ctx->object_list != NULL)
        av_log(0, AV_LOG_ERROR, "Memory deallocation error!\n");
}

static int dvbsub_init_decoder(AVCodecContext *avctx)
{
    int i, r, g, b, a = 0;
    DVBSubContext *ctx = (DVBSubContext*) avctx->priv_data;

    memset(avctx->priv_data, 0, sizeof(DVBSubContext));

    ctx->composition_id = avctx->sub_id & 0xffff;
    ctx->ancillary_id = avctx->sub_id >> 16;

    default_clut.id = -1;
    default_clut.next = NULL;

    default_clut.clut4[0] = RGBA(  0,   0,   0,   0);
    default_clut.clut4[1] = RGBA(255, 255, 255, 255);
    default_clut.clut4[2] = RGBA(  0,   0,   0, 255);
    default_clut.clut4[3] = RGBA(127, 127, 127, 255);

    default_clut.clut16[0] = RGBA(  0,   0,   0,   0);
    for (i = 1; i < 16; i++) {
        if (i < 8) {
            r = (i & 1) ? 255 : 0;
            g = (i & 2) ? 255 : 0;
            b = (i & 4) ? 255 : 0;
        } else {
            r = (i & 1) ? 127 : 0;
            g = (i & 2) ? 127 : 0;
            b = (i & 4) ? 127 : 0;
        }
        default_clut.clut16[i] = RGBA(r, g, b, 255);
    }

    default_clut.clut256[0] = RGBA(  0,   0,   0,   0);
    for (i = 1; i < 256; i++) {
        if (i < 8) {
            r = (i & 1) ? 255 : 0;
            g = (i & 2) ? 255 : 0;
            b = (i & 4) ? 255 : 0;
            a = 63;
        } else {
            switch (i & 0x88) {
            case 0x00:
                r = ((i & 1) ? 85 : 0) + ((i & 0x10) ? 170 : 0);
                g = ((i & 2) ? 85 : 0) + ((i & 0x20) ? 170 : 0);
                b = ((i & 4) ? 85 : 0) + ((i & 0x40) ? 170 : 0);
                a = 255;
                break;
            case 0x08:
                r = ((i & 1) ? 85 : 0) + ((i & 0x10) ? 170 : 0);
                g = ((i & 2) ? 85 : 0) + ((i & 0x20) ? 170 : 0);
                b = ((i & 4) ? 85 : 0) + ((i & 0x40) ? 170 : 0);
                a = 127;
                break;
            case 0x80:
                r = 127 + ((i & 1) ? 43 : 0) + ((i & 0x10) ? 85 : 0);
                g = 127 + ((i & 2) ? 43 : 0) + ((i & 0x20) ? 85 : 0);
                b = 127 + ((i & 4) ? 43 : 0) + ((i & 0x40) ? 85 : 0);
                a = 255;
                break;
            case 0x88:
                r = ((i & 1) ? 43 : 0) + ((i & 0x10) ? 85 : 0);
                g = ((i & 2) ? 43 : 0) + ((i & 0x20) ? 85 : 0);
                b = ((i & 4) ? 43 : 0) + ((i & 0x40) ? 85 : 0);
                a = 255;
                break;
            }
        }
        default_clut.clut256[i] = RGBA(r, g, b, a);
    }

    return 0;
}

static int dvbsub_close_decoder(AVCodecContext *avctx)
{
    DVBSubContext *ctx = (DVBSubContext*) avctx->priv_data;
    DVBSubRegionDisplay *display;

    delete_state(ctx);

    while (ctx->display_list != NULL)
    {
        display = ctx->display_list;
        ctx->display_list = display->next;

        av_free(display);
    }

    return 0;
}

static int dvbsub_read_2bit_string(uint8_t *destbuf, int dbuf_len,
                                   uint8_t **srcbuf, int buf_size,
                                   int non_mod, uint8_t *map_table)
{
    GetBitContext gb;

    int bits;
    int run_length;
    int pixels_read = 0;

    init_get_bits(&gb, *srcbuf, buf_size << 8);

    while (get_bits_count(&gb) < (buf_size << 8) && pixels_read < dbuf_len) {
        bits = get_bits(&gb, 2);

        if (bits != 0) {
            if (non_mod != 1 || bits != 1) {
                if (map_table != NULL)
                    *destbuf++ = map_table[bits];
                else
                    *destbuf++ = bits;
            }
            pixels_read++;
        } else {
            bits = get_bits(&gb, 1);
            if (bits == 1) {
                run_length = get_bits(&gb, 3) + 3;
                bits = get_bits(&gb, 2);

                if (non_mod == 1 && bits == 1)
                    pixels_read += run_length;
                else {
                    if (map_table != NULL)
                        bits = map_table[bits];
                    while (run_length-- > 0 && pixels_read < dbuf_len) {
                        *destbuf++ = bits;
                        pixels_read++;
                    }
                }
            } else {
                bits = get_bits(&gb, 1);
                if (bits == 0) {
                    bits = get_bits(&gb, 2);
                    if (bits == 2) {
                        run_length = get_bits(&gb, 4) + 12;
                        bits = get_bits(&gb, 2);

                        if (non_mod == 1 && bits == 1)
                            pixels_read += run_length;
                        else {
                            if (map_table != NULL)
                                bits = map_table[bits];
                            while (run_length-- > 0 && pixels_read < dbuf_len) {
                                *destbuf++ = bits;
                                pixels_read++;
                            }
                        }
                    } else if (bits == 3) {
                        run_length = get_bits(&gb, 8) + 29;
                        bits = get_bits(&gb, 2);

                        if (non_mod == 1 && bits == 1)
                            pixels_read += run_length;
                        else {
                            if (map_table != NULL)
                                bits = map_table[bits];
                            while (run_length-- > 0 && pixels_read < dbuf_len) {
                                *destbuf++ = bits;
                                pixels_read++;
                            }
                        }
                    } else if (bits == 1) {
                        pixels_read += 2;
                        if (map_table != NULL)
                            bits = map_table[0];
                        else
                            bits = 0;
                        if (pixels_read <= dbuf_len) {
                            *destbuf++ = bits;
                            *destbuf++ = bits;
                        }
                    } else {
                        (*srcbuf) += (get_bits_count(&gb) + 7) >> 3;
                        return pixels_read;
                    }
                } else {
                    if (map_table != NULL)
                        bits = map_table[0];
                    else
                        bits = 0;
                    *destbuf++ = bits;
                    pixels_read++;
                }
            }
        }
    }

    if (get_bits(&gb, 6) != 0)
        av_log(0, AV_LOG_ERROR, "DVBSub error: line overflow\n");

    (*srcbuf) += (get_bits_count(&gb) + 7) >> 3;

    return pixels_read;
}

static int dvbsub_read_4bit_string(uint8_t *destbuf, int dbuf_len,
                                   uint8_t **srcbuf, int buf_size,
                                   int non_mod, uint8_t *map_table)
{
    GetBitContext gb;

    int bits;
    int run_length;
    int pixels_read = 0;

    init_get_bits(&gb, *srcbuf, buf_size << 8);

    while (get_bits_count(&gb) < (buf_size << 8) && pixels_read < dbuf_len) {
        bits = get_bits(&gb, 4);

        if (bits != 0) {
            if (non_mod != 1 || bits != 1) {
                if (map_table != NULL)
                    *destbuf++ = map_table[bits];
                else
                    *destbuf++ = bits;
            }
            pixels_read++;
        } else {
            bits = get_bits(&gb, 1);
            if (bits == 0) {
                run_length = get_bits(&gb, 3);

                if (run_length == 0) {
                    (*srcbuf) += (get_bits_count(&gb) + 7) >> 3;
                    return pixels_read;
                }

                run_length += 2;

                if (map_table != NULL)
                    bits = map_table[0];
                else
                    bits = 0;

                while (run_length-- > 0 && pixels_read < dbuf_len) {
                    *destbuf++ = bits;
                    pixels_read++;
                }
            } else {
                bits = get_bits(&gb, 1);
                if (bits == 0) {
                    run_length = get_bits(&gb, 2) + 4;
                    bits = get_bits(&gb, 4);

                    if (non_mod == 1 && bits == 1)
                        pixels_read += run_length;
                    else {
                        if (map_table != NULL)
                            bits = map_table[bits];
                        while (run_length-- > 0 && pixels_read < dbuf_len) {
                            *destbuf++ = bits;
                            pixels_read++;
                        }
                    }
                } else {
                    bits = get_bits(&gb, 2);
                    if (bits == 2) {
                        run_length = get_bits(&gb, 4) + 9;
                        bits = get_bits(&gb, 4);

                        if (non_mod == 1 && bits == 1)
                            pixels_read += run_length;
                        else {
                            if (map_table != NULL)
                                bits = map_table[bits];
                            while (run_length-- > 0 && pixels_read < dbuf_len) {
                                *destbuf++ = bits;
                                pixels_read++;
                            }
                        }
                    } else if (bits == 3) {
                        run_length = get_bits(&gb, 8) + 25;
                        bits = get_bits(&gb, 4);

                        if (non_mod == 1 && bits == 1)
                            pixels_read += run_length;
                        else {
                            if (map_table != NULL)
                                bits = map_table[bits];
                            while (run_length-- > 0 && pixels_read < dbuf_len) {
                                *destbuf++ = bits;
                                pixels_read++;
                            }
                        }
                    } else if (bits == 1) {
                        pixels_read += 2;
                        if (map_table != NULL)
                            bits = map_table[0];
                        else
                            bits = 0;
                        if (pixels_read <= dbuf_len) {
                            *destbuf++ = bits;
                            *destbuf++ = bits;
                        }
                    } else {
                        if (map_table != NULL)
                            bits = map_table[0];
                        else
                            bits = 0;
                        *destbuf++ = bits;
                        pixels_read ++;
                    }
                }
            }
        }
    }

    if (get_bits(&gb, 8) != 0)
        av_log(0, AV_LOG_ERROR, "DVBSub error: line overflow\n");

    (*srcbuf) += (get_bits_count(&gb) + 7) >> 3;

    return pixels_read;
}

static int dvbsub_read_8bit_string(uint8_t *destbuf, int dbuf_len,
                                    uint8_t **srcbuf, int buf_size,
                                    int non_mod, uint8_t *map_table)
{
    uint8_t *sbuf_end = (*srcbuf) + buf_size;
    int bits;
    int run_length;
    int pixels_read = 0;

    while (*srcbuf < sbuf_end && pixels_read < dbuf_len) {
        bits = *(*srcbuf)++;

        if (bits != 0) {
            if (non_mod != 1 || bits != 1) {
                if (map_table != NULL)
                    *destbuf++ = map_table[bits];
                else
                    *destbuf++ = bits;
            }
            pixels_read++;
        } else {
            bits = *(*srcbuf)++;
            run_length = bits & 0x7f;
            if ((bits & 0x80) == 0) {
                if (run_length == 0) {
                    return pixels_read;
                }

                if (map_table != NULL)
                    bits = map_table[0];
                else
                    bits = 0;
                while (run_length-- > 0 && pixels_read < dbuf_len) {
                    *destbuf++ = bits;
                    pixels_read++;
                }
            } else {
                bits = *(*srcbuf)++;

                if (non_mod == 1 && bits == 1)
                    pixels_read += run_length;
                if (map_table != NULL)
                    bits = map_table[bits];
                else while (run_length-- > 0 && pixels_read < dbuf_len) {
                    *destbuf++ = bits;
                    pixels_read++;
                }
            }
        }
    }

    if (*(*srcbuf)++ != 0)
        av_log(0, AV_LOG_ERROR, "DVBSub error: line overflow\n");

    return pixels_read;
}



static void dvbsub_parse_pixel_data_block(AVCodecContext *avctx, DVBSubObjectDisplay *display,
                                          uint8_t *buf, int buf_size, int top_bottom, int non_mod)
{
    DVBSubContext *ctx = (DVBSubContext*) avctx->priv_data;

    DVBSubRegion *region = get_region(ctx, display->region_id);
    uint8_t *buf_end = buf + buf_size;
    uint8_t *pbuf;
    int x_pos, y_pos;
    int i;

    uint8_t map2to4[] = { 0x0,  0x7,  0x8,  0xf};
    uint8_t map2to8[] = {0x00, 0x77, 0x88, 0xff};
    uint8_t map4to8[] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                         0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};
    uint8_t *map_table;

#ifdef DEBUG
    av_log(avctx, AV_LOG_INFO, "DVB pixel block size %d, %s field:\n", buf_size,
                top_bottom ? "bottom" : "top");
#endif

#ifdef DEBUG_PACKET_CONTENTS
    for (i = 0; i < buf_size; i++)
    {
        if (i % 16 == 0)
            av_log(avctx, AV_LOG_INFO, "0x%08p: ", buf+i);

        av_log(avctx, AV_LOG_INFO, "%02x ", buf[i]);
        if (i % 16 == 15)
            av_log(avctx, AV_LOG_INFO, "\n");
    }

    if (i % 16 != 0)
        av_log(avctx, AV_LOG_INFO, "\n");

#endif

    if (region == 0)
        return;

    pbuf = region->pbuf;

    x_pos = display->x_pos;
    y_pos = display->y_pos;

    if ((y_pos & 1) != top_bottom)
        y_pos++;

    while (buf < buf_end) {
        if (x_pos > region->width || y_pos > region->height) {
            av_log(avctx, AV_LOG_ERROR, "Invalid object location!\n");
            return;
        }

        switch (*buf++) {
        case 0x10:
            if (region->depth == 8)
                map_table = map2to8;
            else if (region->depth == 4)
                map_table = map2to4;
            else
                map_table = NULL;

            x_pos += dvbsub_read_2bit_string(pbuf + (y_pos * region->width) + x_pos,
                                                region->width - x_pos, &buf, buf_size,
                                                non_mod, map_table);
            break;
        case 0x11:
            if (region->depth < 4) {
                av_log(avctx, AV_LOG_ERROR, "4-bit pixel string in %d-bit region!\n", region->depth);
                return;
            }

            if (region->depth == 8)
                map_table = map4to8;
            else
                map_table = NULL;

            x_pos += dvbsub_read_4bit_string(pbuf + (y_pos * region->width) + x_pos,
                                                region->width - x_pos, &buf, buf_size,
                                                non_mod, map_table);
            break;
        case 0x12:
            if (region->depth < 8) {
                av_log(avctx, AV_LOG_ERROR, "8-bit pixel string in %d-bit region!\n", region->depth);
                return;
            }

            x_pos += dvbsub_read_8bit_string(pbuf + (y_pos * region->width) + x_pos,
                                                region->width - x_pos, &buf, buf_size,
                                                non_mod, NULL);
            break;

        case 0x20:
            map2to4[0] = (*buf) >> 4;
            map2to4[1] = (*buf++) & 0xf;
            map2to4[2] = (*buf) >> 4;
            map2to4[3] = (*buf++) & 0xf;
            break;
        case 0x21:
            for (i = 0; i < 4; i++)
                map2to8[i] = *buf++;
            break;
        case 0x22:
            for (i = 0; i < 16; i++)
                map4to8[i] = *buf++;
            break;

        case 0xf0:
            x_pos = display->x_pos;
            y_pos += 2;
            break;
        default:
            av_log(avctx, AV_LOG_INFO, "Unknown/unsupported pixel block 0x%x\n", *(buf-1));
        }
    }

}

static void dvbsub_parse_object_segment(AVCodecContext *avctx,
                                        uint8_t *buf, int buf_size)
{
    DVBSubContext *ctx = (DVBSubContext*) avctx->priv_data;

    uint8_t *buf_end = buf + buf_size;
    uint8_t *block;
    int object_id;
    DVBSubObject *object;
    DVBSubObjectDisplay *display;
    int top_field_len, bottom_field_len;

    int coding_method, non_modifying_colour;

    object_id = AV_RB16(buf);
    buf += 2;

    object = get_object(ctx, object_id);

    if (!object)
        return;

    coding_method = ((*buf) >> 2) & 3;
    non_modifying_colour = ((*buf++) >> 1) & 1;

    if (coding_method == 0) {
        top_field_len = AV_RB16(buf);
        buf += 2;
        bottom_field_len = AV_RB16(buf);
        buf += 2;

        if (buf + top_field_len + bottom_field_len > buf_end) {
            av_log(avctx, AV_LOG_ERROR, "Field data size too large\n");
            return;
        }

        for (display = object->display_list; display != 0; display = display->object_list_next) {
            block = buf;

            dvbsub_parse_pixel_data_block(avctx, display, block, top_field_len, 0,
                                            non_modifying_colour);

            if (bottom_field_len > 0)
                block = buf + top_field_len;
            else
                bottom_field_len = top_field_len;

            dvbsub_parse_pixel_data_block(avctx, display, block, bottom_field_len, 1,
                                            non_modifying_colour);
        }

/*  } else if (coding_method == 1) {*/

    } else {
        av_log(avctx, AV_LOG_ERROR, "Unknown object coding %d\n", coding_method);
    }

}

#define SCALEBITS 10
#define ONE_HALF  (1 << (SCALEBITS - 1))
#define FIX(x)    ((int) ((x) * (1<<SCALEBITS) + 0.5))

#define YUV_TO_RGB1_CCIR(cb1, cr1)\
{\
    cb = (cb1) - 128;\
    cr = (cr1) - 128;\
    r_add = FIX(1.40200*255.0/224.0) * cr + ONE_HALF;\
    g_add = - FIX(0.34414*255.0/224.0) * cb - FIX(0.71414*255.0/224.0) * cr + \
            ONE_HALF;\
    b_add = FIX(1.77200*255.0/224.0) * cb + ONE_HALF;\
}

#define YUV_TO_RGB2_CCIR(r, g, b, y1)\
{\
    y = ((y1) - 16) * FIX(255.0/219.0);\
    r = cm[(y + r_add) >> SCALEBITS];\
    g = cm[(y + g_add) >> SCALEBITS];\
    b = cm[(y + b_add) >> SCALEBITS];\
}


static void dvbsub_parse_clut_segment(AVCodecContext *avctx,
                                        uint8_t *buf, int buf_size)
{
    DVBSubContext *ctx = (DVBSubContext*) avctx->priv_data;

    uint8_t *buf_end = buf + buf_size;
    int clut_id;
    DVBSubCLUT *clut;
    int entry_id, depth , full_range;
    int y, cr, cb, alpha;
    int r, g, b, r_add, g_add, b_add;

#ifdef DEBUG_PACKET_CONTENTS
    int i;

    av_log(avctx, AV_LOG_INFO, "DVB clut packet:\n");

    for (i=0; i < buf_size; i++)
    {
        av_log(avctx, AV_LOG_INFO, "%02x ", buf[i]);
        if (i % 16 == 15)
            av_log(avctx, AV_LOG_INFO, "\n");
    }

    if (i % 16 != 0)
        av_log(avctx, AV_LOG_INFO, "\n");

#endif

    clut_id = *buf++;
    buf += 1;

    clut = get_clut(ctx, clut_id);

    if (clut == NULL) {
        clut = av_malloc(sizeof(DVBSubCLUT));

        memcpy(clut, &default_clut, sizeof(DVBSubCLUT));

        clut->id = clut_id;

        clut->next = ctx->clut_list;
        ctx->clut_list = clut;
    }

    while (buf + 4 < buf_end)
    {
        entry_id = *buf++;

        depth = (*buf) & 0xe0;

        if (depth == 0) {
            av_log(avctx, AV_LOG_ERROR, "Invalid clut depth 0x%x!\n", *buf);
            return;
        }

        full_range = (*buf++) & 1;

        if (full_range) {
            y = *buf++;
            cr = *buf++;
            cb = *buf++;
            alpha = *buf++;
        } else {
            y = buf[0] & 0xfc;
            cr = (((buf[0] & 3) << 2) | ((buf[1] >> 6) & 3)) << 4;
            cb = (buf[1] << 2) & 0xf0;
            alpha = (buf[1] << 6) & 0xc0;

            buf += 2;
        }

        if (y == 0)
            alpha = 0xff;

        YUV_TO_RGB1_CCIR(cb, cr);
        YUV_TO_RGB2_CCIR(r, g, b, y);

#ifdef DEBUG
        av_log(avctx, AV_LOG_INFO, "clut %d := (%d,%d,%d,%d)\n", entry_id, r, g, b, alpha);
#endif

        if (depth & 0x80)
            clut->clut4[entry_id] = RGBA(r,g,b,255 - alpha);
        if (depth & 0x40)
            clut->clut16[entry_id] = RGBA(r,g,b,255 - alpha);
        if (depth & 0x20)
            clut->clut256[entry_id] = RGBA(r,g,b,255 - alpha);
    }
}


static void dvbsub_parse_region_segment(AVCodecContext *avctx,
                                        uint8_t *buf, int buf_size)
{
    DVBSubContext *ctx = (DVBSubContext*) avctx->priv_data;

    uint8_t *buf_end = buf + buf_size;
    int region_id, object_id;
    DVBSubRegion *region;
    DVBSubObject *object;
    DVBSubObjectDisplay *display;
    int fill;

    if (buf_size < 10)
        return;

    region_id = *buf++;

    region = get_region(ctx, region_id);

    if (region == NULL)
    {
        region = av_mallocz(sizeof(DVBSubRegion));

        region->id = region_id;

        region->next = ctx->region_list;
        ctx->region_list = region;
    }

    fill = ((*buf++) >> 3) & 1;

    region->width = AV_RB16(buf);
    buf += 2;
    region->height = AV_RB16(buf);
    buf += 2;

    if (region->width * region->height != region->buf_size) {
        if (region->pbuf != 0)
            av_free(region->pbuf);

        region->buf_size = region->width * region->height;

        region->pbuf = av_malloc(region->buf_size);

        fill = 1;
    }

    region->depth = 1 << (((*buf++) >> 2) & 7);
    region->clut = *buf++;

    if (region->depth == 8)
        region->bgcolour = *buf++;
    else {
        buf += 1;

        if (region->depth == 4)
            region->bgcolour = (((*buf++) >> 4) & 15);
        else
            region->bgcolour = (((*buf++) >> 2) & 3);
    }

#ifdef DEBUG
    av_log(avctx, AV_LOG_INFO, "Region %d, (%dx%d)\n", region_id, region->width, region->height);
#endif

    if (fill) {
        memset(region->pbuf, region->bgcolour, region->buf_size);
#ifdef DEBUG
        av_log(avctx, AV_LOG_INFO, "Fill region (%d)\n", region->bgcolour);
#endif
    }

    delete_region_display_list(ctx, region);

    while (buf + 5 < buf_end) {
        object_id = AV_RB16(buf);
        buf += 2;

        object = get_object(ctx, object_id);

        if (object == NULL) {
            object = av_mallocz(sizeof(DVBSubObject));

            object->id = object_id;
            object->next = ctx->object_list;
            ctx->object_list = object;
        }

        object->type = (*buf) >> 6;

        display = av_mallocz(sizeof(DVBSubObjectDisplay));

        display->object_id = object_id;
        display->region_id = region_id;

        display->x_pos = AV_RB16(buf) & 0xfff;
        buf += 2;
        display->y_pos = AV_RB16(buf) & 0xfff;
        buf += 2;

        if ((object->type == 1 || object->type == 2) && buf+1 < buf_end) {
            display->fgcolour = *buf++;
            display->bgcolour = *buf++;
        }

        display->region_list_next = region->display_list;
        region->display_list = display;

        display->object_list_next = object->display_list;
        object->display_list = display;
    }
}

static void dvbsub_parse_page_segment(AVCodecContext *avctx,
                                        uint8_t *buf, int buf_size)
{
    DVBSubContext *ctx = (DVBSubContext*) avctx->priv_data;
    DVBSubRegionDisplay *display;
    DVBSubRegionDisplay *tmp_display_list, **tmp_ptr;

    uint8_t *buf_end = buf + buf_size;
    int region_id;
    int page_state;

    if (buf_size < 1)
        return;

    ctx->time_out = *buf++;
    page_state = ((*buf++) >> 2) & 3;

#ifdef DEBUG
    av_log(avctx, AV_LOG_INFO, "Page time out %ds, state %d\n", ctx->time_out, page_state);
#endif

    if (page_state == 2)
    {
        delete_state(ctx);
    }

    tmp_display_list = ctx->display_list;
    ctx->display_list = NULL;
    ctx->display_list_size = 0;

    while (buf + 5 < buf_end) {
        region_id = *buf++;
        buf += 1;

        display = tmp_display_list;
        tmp_ptr = &tmp_display_list;

        while (display != NULL && display->region_id != region_id) {
            tmp_ptr = &display->next;
            display = display->next;
        }

        if (display == NULL)
            display = av_mallocz(sizeof(DVBSubRegionDisplay));

        display->region_id = region_id;

        display->x_pos = AV_RB16(buf);
        buf += 2;
        display->y_pos = AV_RB16(buf);
        buf += 2;

        *tmp_ptr = display->next;

        display->next = ctx->display_list;
        ctx->display_list = display;
        ctx->display_list_size++;

#ifdef DEBUG
        av_log(avctx, AV_LOG_INFO, "Region %d, (%d,%d)\n", region_id, display->x_pos, display->y_pos);
#endif
    }

    while (tmp_display_list != 0) {
        display = tmp_display_list;

        tmp_display_list = display->next;

        av_free(display);
    }

}


#ifdef DEBUG_SAVE_IMAGES
static void save_display_set(DVBSubContext *ctx)
{
    DVBSubRegion *region;
    DVBSubRegionDisplay *display;
    DVBSubCLUT *clut;
    uint32_t *clut_table;
    int x_pos, y_pos, width, height;
    int x, y, y_off, x_off;
    uint32_t *pbuf;
    char filename[32];
    static int fileno_index = 0;

    x_pos = -1;
    y_pos = -1;
    width = 0;
    height = 0;

    for (display = ctx->display_list; display != NULL; display = display->next) {
        region = get_region(ctx, display->region_id);

        if (x_pos == -1) {
            x_pos = display->x_pos;
            y_pos = display->y_pos;
            width = region->width;
            height = region->height;
        } else {
            if (display->x_pos < x_pos) {
                width += (x_pos - display->x_pos);
                x_pos = display->x_pos;
            }

            if (display->y_pos < y_pos) {
                height += (y_pos - display->y_pos);
                y_pos = display->y_pos;
            }

            if (display->x_pos + region->width > x_pos + width) {
                width = display->x_pos + region->width - x_pos;
            }

            if (display->y_pos + region->height > y_pos + height) {
                height = display->y_pos + region->height - y_pos;
            }
        }
    }

    if (x_pos >= 0) {

        pbuf = av_malloc(width * height * 4);

        for (display = ctx->display_list; display != NULL; display = display->next) {
            region = get_region(ctx, display->region_id);

            x_off = display->x_pos - x_pos;
            y_off = display->y_pos - y_pos;

            clut = get_clut(ctx, region->clut);

            if (clut == 0)
                clut = &default_clut;

            switch (region->depth) {
            case 2:
                clut_table = clut->clut4;
                break;
            case 8:
                clut_table = clut->clut256;
                break;
            case 4:
            default:
                clut_table = clut->clut16;
                break;
            }

            for (y = 0; y < region->height; y++) {
                for (x = 0; x < region->width; x++) {
                    pbuf[((y + y_off) * width) + x_off + x] =
                        clut_table[region->pbuf[y * region->width + x]];
                }
            }

        }

        snprintf(filename, 32, "dvbs.%d", fileno_index);

        png_save2(filename, pbuf, width, height);

        av_free(pbuf);
    }

    fileno_index++;
}
#endif

static int dvbsub_display_end_segment(AVCodecContext *avctx, uint8_t *buf,
                                        int buf_size, AVSubtitle *sub)
{
    DVBSubContext *ctx = (DVBSubContext*) avctx->priv_data;

    DVBSubRegion *region;
    DVBSubRegionDisplay *display;
    AVSubtitleRect *rect;
    DVBSubCLUT *clut;
    uint32_t *clut_table;
    int i;

    sub->rects = NULL;
    sub->start_display_time = 0;
    sub->end_display_time = ctx->time_out * 1000;
    sub->format = 0;

    sub->num_rects = ctx->display_list_size;

    if (sub->num_rects > 0)
        sub->rects = av_mallocz(sizeof(AVSubtitleRect) * sub->num_rects);

    i = 0;

    for (display = ctx->display_list; display != NULL; display = display->next) {
        region = get_region(ctx, display->region_id);
        rect = &sub->rects[i];

        if (region == NULL)
            continue;

        rect->x = display->x_pos;
        rect->y = display->y_pos;
        rect->w = region->width;
        rect->h = region->height;
        rect->nb_colors = 16;
        rect->linesize = region->width;

        clut = get_clut(ctx, region->clut);

        if (clut == NULL)
            clut = &default_clut;

        switch (region->depth) {
        case 2:
            clut_table = clut->clut4;
            break;
        case 8:
            clut_table = clut->clut256;
            break;
        case 4:
        default:
            clut_table = clut->clut16;
            break;
        }

        rect->rgba_palette = av_malloc((1 << region->depth) * sizeof(uint32_t));
        memcpy(rect->rgba_palette, clut_table, (1 << region->depth) * sizeof(uint32_t));

        rect->bitmap = av_malloc(region->buf_size);
        memcpy(rect->bitmap, region->pbuf, region->buf_size);

        i++;
    }

    sub->num_rects = i;

#ifdef DEBUG_SAVE_IMAGES
    save_display_set(ctx);
#endif

    return 1;
}

static int dvbsub_decode(AVCodecContext *avctx,
                         void *data, int *data_size,
                         uint8_t *buf, int buf_size)
{
    DVBSubContext *ctx = (DVBSubContext*) avctx->priv_data;
    AVSubtitle *sub = (AVSubtitle*) data;
    uint8_t *p, *p_end;
    int segment_type;
    int page_id;
    int segment_length;

#ifdef DEBUG_PACKET_CONTENTS
    int i;

    av_log(avctx, AV_LOG_INFO, "DVB sub packet:\n");

    for (i=0; i < buf_size; i++)
    {
        av_log(avctx, AV_LOG_INFO, "%02x ", buf[i]);
        if (i % 16 == 15)
            av_log(avctx, AV_LOG_INFO, "\n");
    }

    if (i % 16 != 0)
        av_log(avctx, AV_LOG_INFO, "\n");

#endif

    if (buf_size <= 2)
        return -1;

    p = buf;
    p_end = buf + buf_size;

    while (p < p_end && *p == 0x0f)
    {
        p += 1;
        segment_type = *p++;
        page_id = AV_RB16(p);
        p += 2;
        segment_length = AV_RB16(p);
        p += 2;

        if (page_id == ctx->composition_id || page_id == ctx->ancillary_id) {
            switch (segment_type) {
            case DVBSUB_PAGE_SEGMENT:
                dvbsub_parse_page_segment(avctx, p, segment_length);
                break;
            case DVBSUB_REGION_SEGMENT:
                dvbsub_parse_region_segment(avctx, p, segment_length);
                break;
            case DVBSUB_CLUT_SEGMENT:
                dvbsub_parse_clut_segment(avctx, p, segment_length);
                break;
            case DVBSUB_OBJECT_SEGMENT:
                dvbsub_parse_object_segment(avctx, p, segment_length);
                break;
            case DVBSUB_DISPLAY_SEGMENT:
                *data_size = dvbsub_display_end_segment(avctx, p, segment_length, sub);
                break;
            default:
#ifdef DEBUG
                av_log(avctx, AV_LOG_INFO, "Subtitling segment type 0x%x, page id %d, length %d\n",
                        segment_type, page_id, segment_length);
#endif
                break;
            }
        }

        p += segment_length;
    }

    if (p != p_end)
    {
#ifdef DEBUG
        av_log(avctx, AV_LOG_INFO, "Junk at end of packet\n");
#endif
        return -1;
    }

    return buf_size;
}


AVCodec dvbsub_decoder = {
    "dvbsub",
    CODEC_TYPE_SUBTITLE,
    CODEC_ID_DVB_SUBTITLE,
    sizeof(DVBSubContext),
    dvbsub_init_decoder,
    NULL,
    dvbsub_close_decoder,
    dvbsub_decode,
};
