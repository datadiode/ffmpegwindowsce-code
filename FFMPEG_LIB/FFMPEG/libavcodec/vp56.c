/**
 * @file vp56.c
 * VP5 and VP6 compatible video decoder (common features)
 *
 * Copyright (C) 2006  Aurelien Jacobs <aurel@gnuage.org>
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "avcodec.h"

#include "vp56.h"
#include "vp56data.h"


void vp56_init_dequant(vp56_context_t *s, int quantizer)
{
    s->quantizer = quantizer;
    s->dequant_dc = vp56_dc_dequant[quantizer] << 2;
    s->dequant_ac = vp56_ac_dequant[quantizer] << 2;
}

static int vp56_get_vectors_predictors(vp56_context_t *s, int row, int col,
                                       vp56_frame_t ref_frame)
{
    int nb_pred = 0;
    vp56_mv_t vect[2] = {{0,0}, {0,0}};
    int pos, offset;
    vp56_mv_t mvp;

    for (pos=0; pos<12; pos++) {
        mvp.x = col + vp56_candidate_predictor_pos[pos][0];
        mvp.y = row + vp56_candidate_predictor_pos[pos][1];
        if (mvp.x < 0 || mvp.x >= s->mb_width ||
            mvp.y < 0 || mvp.y >= s->mb_height)
            continue;
        offset = mvp.x + s->mb_width*mvp.y;

        if (vp56_reference_frame[s->macroblocks[offset].type] != ref_frame)
            continue;
        if ((s->macroblocks[offset].mv.x == vect[0].x &&
             s->macroblocks[offset].mv.y == vect[0].y) ||
            (s->macroblocks[offset].mv.x == 0 &&
             s->macroblocks[offset].mv.y == 0))
            continue;

        vect[nb_pred++] = s->macroblocks[offset].mv;
        if (nb_pred > 1) {
            nb_pred = -1;
            break;
        }
        s->vector_candidate_pos = pos;
    }

    s->vector_candidate[0] = vect[0];
    s->vector_candidate[1] = vect[1];

    return nb_pred+1;
}

static void vp56_parse_mb_type_models(vp56_context_t *s)
{
    vp56_range_coder_t *c = &s->c;
    int i, ctx, type;

    for (ctx=0; ctx<3; ctx++) {
        if (vp56_rac_get_prob(c, 174)) {
            int idx = vp56_rac_gets(c, 4);
            memcpy(s->mb_types_stats[ctx],vp56_pre_def_mb_type_stats[idx][ctx],
                   sizeof(s->mb_types_stats[ctx]));
        }
        if (vp56_rac_get_prob(c, 254)) {
            for (type=0; type<10; type++) {
                for(i=0; i<2; i++) {
                    if (vp56_rac_get_prob(c, 205)) {
                        int delta, sign = vp56_rac_get(c);

                        delta = vp56_rac_get_tree(c, vp56_pmbtm_tree,
                                                  vp56_mb_type_model_model);
                        if (!delta)
                            delta = 4 * vp56_rac_gets(c, 7);
                        s->mb_types_stats[ctx][type][i] += (delta ^ -sign) + sign;
                    }
                }
            }
        }
    }

    /* compute MB type probability tables based on previous MB type */
    for (ctx=0; ctx<3; ctx++) {
        int p[10];

        for (type=0; type<10; type++)
            p[type] = 100 * s->mb_types_stats[ctx][type][1];

        for (type=0; type<10; type++) {
            int p02, p34, p0234, p17, p56, p89, p5689, p156789;

            /* conservative MB type probability */
            s->mb_type_model[ctx][type][0] = 255 - (255 * s->mb_types_stats[ctx][type][0]) / (1 + s->mb_types_stats[ctx][type][0] + s->mb_types_stats[ctx][type][1]);

            p[type] = 0;    /* same MB type => weight is null */

            /* binary tree parsing probabilities */
            p02 = p[0] + p[2];
            p34 = p[3] + p[4];
            p0234 = p02 + p34;
            p17 = p[1] + p[7];
            p56 = p[5] + p[6];
            p89 = p[8] + p[9];
            p5689 = p56 + p89;
            p156789 = p17 + p5689;

            s->mb_type_model[ctx][type][1] = 1 + 255 * p0234/(1+p0234+p156789);
            s->mb_type_model[ctx][type][2] = 1 + 255 * p02  / (1+p0234);
            s->mb_type_model[ctx][type][3] = 1 + 255 * p17  / (1+p156789);
            s->mb_type_model[ctx][type][4] = 1 + 255 * p[0] / (1+p02);
            s->mb_type_model[ctx][type][5] = 1 + 255 * p[3] / (1+p34);
            s->mb_type_model[ctx][type][6] = 1 + 255 * p[1] / (1+p17);
            s->mb_type_model[ctx][type][7] = 1 + 255 * p56  / (1+p5689);
            s->mb_type_model[ctx][type][8] = 1 + 255 * p[5] / (1+p56);
            s->mb_type_model[ctx][type][9] = 1 + 255 * p[8] / (1+p89);

            /* restore initial value */
            p[type] = 100 * s->mb_types_stats[ctx][type][1];
        }
    }
}

static vp56_mb_t vp56_parse_mb_type(vp56_context_t *s,
                                    vp56_mb_t prev_type, int ctx)
{
    uint8_t *mb_type_model = s->mb_type_model[ctx][prev_type];
    vp56_range_coder_t *c = &s->c;

    if (vp56_rac_get_prob(c, mb_type_model[0]))
        return prev_type;
    else
        return vp56_rac_get_tree(c, vp56_pmbt_tree, mb_type_model);
}

static void vp56_decode_4mv(vp56_context_t *s, int row, int col)
{
    vp56_mv_t mv = {0,0};
    int type[4];
    int b;

    /* parse each block type */
    for (b=0; b<4; b++) {
        type[b] = vp56_rac_gets(&s->c, 2);
        if (type[b])
            type[b]++;  /* only returns 0, 2, 3 or 4 (all INTER_PF) */
    }

    /* get vectors */
    for (b=0; b<4; b++) {
        switch (type[b]) {
            case VP56_MB_INTER_NOVEC_PF:
				s->mv[b].x = 0;
				s->mv[b].y = 0;
                break;
            case VP56_MB_INTER_DELTA_PF:
                s->parse_vector_adjustment(s, &s->mv[b]);
                break;
            case VP56_MB_INTER_V1_PF:
                s->mv[b] = s->vector_candidate[0];
                break;
            case VP56_MB_INTER_V2_PF:
                s->mv[b] = s->vector_candidate[1];
                break;
        }
        mv.x += s->mv[b].x;
        mv.y += s->mv[b].y;
    }

    /* this is the one selected for the whole MB for prediction */
    s->macroblocks[row * s->mb_width + col].mv = s->mv[3];

    /* chroma vectors are average luma vectors */
    if (s->avctx->codec->id == CODEC_ID_VP5) {
        s->mv[4].x = s->mv[5].x = RSHIFT(mv.x,2);
        s->mv[4].y = s->mv[5].y = RSHIFT(mv.y,2);
    } else {
		s->mv[4].x = s->mv[5].x = mv.x/4;
        s->mv[4].y = s->mv[5].y = mv.x/4;
    }
}

static vp56_mb_t vp56_decode_mv(vp56_context_t *s, int row, int col)
{
    vp56_mv_t *mv, vect = {0,0};
    int ctx, b;

    ctx = vp56_get_vectors_predictors(s, row, col, VP56_FRAME_PREVIOUS);
    s->mb_type = vp56_parse_mb_type(s, s->mb_type, ctx);
    s->macroblocks[row * s->mb_width + col].type = s->mb_type;

    switch (s->mb_type) {
        case VP56_MB_INTER_V1_PF:
            mv = &s->vector_candidate[0];
            break;

        case VP56_MB_INTER_V2_PF:
            mv = &s->vector_candidate[1];
            break;

        case VP56_MB_INTER_V1_GF:
            vp56_get_vectors_predictors(s, row, col, VP56_FRAME_GOLDEN);
            mv = &s->vector_candidate[0];
            break;

        case VP56_MB_INTER_V2_GF:
            vp56_get_vectors_predictors(s, row, col, VP56_FRAME_GOLDEN);
            mv = &s->vector_candidate[1];
            break;

        case VP56_MB_INTER_DELTA_PF:
            s->parse_vector_adjustment(s, &vect);
            mv = &vect;
            break;

        case VP56_MB_INTER_DELTA_GF:
            vp56_get_vectors_predictors(s, row, col, VP56_FRAME_GOLDEN);
            s->parse_vector_adjustment(s, &vect);
            mv = &vect;
            break;

        case VP56_MB_INTER_4V:
            vp56_decode_4mv(s, row, col);
            return s->mb_type;

        default:
            mv = &vect;
            break;
    }

    s->macroblocks[row*s->mb_width + col].mv = *mv;

    /* same vector for all blocks */
    for (b=0; b<6; b++)
        s->mv[b] = *mv;

    return s->mb_type;
}

static void vp56_add_predictors_dc(vp56_context_t *s, vp56_frame_t ref_frame)
{
    int idx = s->scantable.permutated[0];
    int i;

    for (i=0; i<6; i++) {
        vp56_ref_dc_t *ab = &s->above_blocks[s->above_block_idx[i]];
        vp56_ref_dc_t *lb = &s->left_block[vp56_b6to4[i]];
        int count = 0;
        int dc = 0;

        if (ref_frame == lb->ref_frame) {
            dc += lb->dc_coeff;
            count++;
        }
        if (ref_frame == ab->ref_frame) {
            dc += ab->dc_coeff;
            count++;
        }
        if (s->avctx->codec->id == CODEC_ID_VP5) {
            if (count < 2 && ref_frame == ab[-1].ref_frame) {
                dc += ab[-1].dc_coeff;
                count++;
            }
            if (count < 2 && ref_frame == ab[1].ref_frame) {
                dc += ab[1].dc_coeff;
                count++;
            }
        }
        if (count == 0)
            dc = s->prev_dc[vp56_b6to3[i]][ref_frame];
        else if (count == 2)
            dc /= 2;

        s->block_coeff[i][idx] += dc;
        s->prev_dc[vp56_b6to3[i]][ref_frame] = s->block_coeff[i][idx];
        ab->dc_coeff = s->block_coeff[i][idx];
        ab->ref_frame = ref_frame;
        lb->dc_coeff = s->block_coeff[i][idx];
        lb->ref_frame = ref_frame;
        s->block_coeff[i][idx] *= s->dequant_dc;
    }
}

static void vp56_edge_filter(vp56_context_t *s, uint8_t *yuv,
                             int pix_inc, int line_inc, int t)
{
    int pix2_inc = 2 * pix_inc;
    int i, v;

    for (i=0; i<12; i++) {
        v = (yuv[-pix2_inc] + 3*(yuv[0]-yuv[-pix_inc]) - yuv[pix_inc] + 4) >>3;
        v = s->adjust(v, t);
        yuv[-pix_inc] = av_clip_uint8(yuv[-pix_inc] + v);
        yuv[0] = av_clip_uint8(yuv[0] - v);
        yuv += line_inc;
    }
}

static void vp56_deblock_filter(vp56_context_t *s, uint8_t *yuv,
                                int stride, int dx, int dy)
{
    int t = vp56_filter_threshold[s->quantizer];
    if (dx)  vp56_edge_filter(s, yuv +         10-dx ,      1, stride, t);
    if (dy)  vp56_edge_filter(s, yuv + stride*(10-dy), stride,      1, t);
}

static void vp56_mc(vp56_context_t *s, int b, uint8_t *src,
                    int stride, int x, int y)
{
    int plane = vp56_b6to3[b];
    uint8_t *dst=s->framep[VP56_FRAME_CURRENT]->data[plane]+s->block_offset[b];
    uint8_t *src_block;
    int src_offset;
    int overlap_offset = 0;
    int mask = s->vp56_coord_div[b] - 1;
    int deblock_filtering = s->deblock_filtering;
    int dx;
    int dy;

    if (s->avctx->skip_loop_filter >= AVDISCARD_ALL ||
        (s->avctx->skip_loop_filter >= AVDISCARD_NONKEY
         && !s->framep[VP56_FRAME_CURRENT]->key_frame))
        deblock_filtering = 0;

    dx = s->mv[b].x / s->vp56_coord_div[b];
    dy = s->mv[b].y / s->vp56_coord_div[b];

    if (b >= 4) {
        x /= 2;
        y /= 2;
    }
    x += dx - 2;
    y += dy - 2;

    if (x<0 || x+12>=s->plane_width[plane] ||
        y<0 || y+12>=s->plane_height[plane]) {
        ff_emulated_edge_mc(s->edge_emu_buffer,
                            src + s->block_offset[b] + (dy-2)*stride + (dx-2),
                            stride, 12, 12, x, y,
                            s->plane_width[plane],
                            s->plane_height[plane]);
        src_block = s->edge_emu_buffer;
        src_offset = 2 + 2*stride;
    } else if (deblock_filtering) {
        /* only need a 12x12 block, but there is no such dsp function, */
        /* so copy a 16x12 block */
        s->dsp.put_pixels_tab[0][0](s->edge_emu_buffer,
                                    src + s->block_offset[b] + (dy-2)*stride + (dx-2),
                                    stride, 12);
        src_block = s->edge_emu_buffer;
        src_offset = 2 + 2*stride;
    } else {
        src_block = src;
        src_offset = s->block_offset[b] + dy*stride + dx;
    }

    if (deblock_filtering)
        vp56_deblock_filter(s, src_block, stride, dx&7, dy&7);

    if (s->mv[b].x & mask)
        overlap_offset += (s->mv[b].x > 0) ? 1 : -1;
    if (s->mv[b].y & mask)
        overlap_offset += (s->mv[b].y > 0) ? stride : -stride;

    if (overlap_offset) {
        if (s->filter)
            s->filter(s, dst, src_block, src_offset, src_offset+overlap_offset,
                      stride, s->mv[b], mask, s->filter_selection, b<4);
        else
            s->dsp.put_no_rnd_pixels_l2[1](dst, src_block+src_offset,
                                           src_block+src_offset+overlap_offset,
                                           stride, 8);
    } else {
        s->dsp.put_pixels_tab[1][0](dst, src_block+src_offset, stride, 8);
    }
}

static void vp56_decode_mb(vp56_context_t *s, int row, int col)
{
    AVFrame *frame_current, *frame_ref;
    vp56_mb_t mb_type;
    vp56_frame_t ref_frame;
    int b, plan, off;

    if (s->framep[VP56_FRAME_CURRENT]->key_frame)
        mb_type = VP56_MB_INTRA;
    else
        mb_type = vp56_decode_mv(s, row, col);
    ref_frame = vp56_reference_frame[mb_type];

    memset(s->block_coeff, 0, sizeof(s->block_coeff));

    s->parse_coeff(s);

    vp56_add_predictors_dc(s, ref_frame);

    frame_current = s->framep[VP56_FRAME_CURRENT];
    frame_ref = s->framep[ref_frame];

    switch (mb_type) {
        case VP56_MB_INTRA:
            for (b=0; b<6; b++) {
                plan = vp56_b6to3[b];
                s->dsp.idct_put(frame_current->data[plan] + s->block_offset[b],
                                s->stride[plan], s->block_coeff[b]);
            }
            break;

        case VP56_MB_INTER_NOVEC_PF:
        case VP56_MB_INTER_NOVEC_GF:
            for (b=0; b<6; b++) {
                plan = vp56_b6to3[b];
                off = s->block_offset[b];
                s->dsp.put_pixels_tab[1][0](frame_current->data[plan] + off,
                                            frame_ref->data[plan] + off,
                                            s->stride[plan], 8);
                s->dsp.idct_add(frame_current->data[plan] + off,
                                s->stride[plan], s->block_coeff[b]);
            }
            break;

        case VP56_MB_INTER_DELTA_PF:
        case VP56_MB_INTER_V1_PF:
        case VP56_MB_INTER_V2_PF:
        case VP56_MB_INTER_DELTA_GF:
        case VP56_MB_INTER_4V:
        case VP56_MB_INTER_V1_GF:
        case VP56_MB_INTER_V2_GF:
            for (b=0; b<6; b++) {
                int x_off = b==1 || b==3 ? 8 : 0;
                int y_off = b==2 || b==3 ? 8 : 0;
                plan = vp56_b6to3[b];
                vp56_mc(s, b, frame_ref->data[plan], s->stride[plan],
                        16*col+x_off, 16*row+y_off);
                s->dsp.idct_add(frame_current->data[plan] + s->block_offset[b],
                                s->stride[plan], s->block_coeff[b]);
            }
            break;
    }
}

static int vp56_size_changed(AVCodecContext *avctx, vp56_context_t *s)
{
    int stride = s->framep[VP56_FRAME_CURRENT]->linesize[0];
    int i;

    s->plane_width[0] = s->avctx->coded_width;
    s->plane_width[1] = s->plane_width[2] = s->avctx->coded_width/2;
    s->plane_height[0] = s->avctx->coded_height;
    s->plane_height[1] = s->plane_height[2] = s->avctx->coded_height/2;

    for (i=0; i<3; i++)
        s->stride[i] = s->flip * s->framep[VP56_FRAME_CURRENT]->linesize[i];

    s->mb_width = (s->avctx->coded_width+15) / 16;
    s->mb_height = (s->avctx->coded_height+15) / 16;

    if (s->mb_width > 1000 || s->mb_height > 1000) {
        av_log(avctx, AV_LOG_ERROR, "picture too big\n");
        return -1;
    }

    s->above_blocks = av_realloc(s->above_blocks,
                                 (4*s->mb_width+6) * sizeof(*s->above_blocks));
    s->macroblocks = av_realloc(s->macroblocks,
                                s->mb_width*s->mb_height*sizeof(*s->macroblocks));
    av_free(s->edge_emu_buffer_alloc);
    s->edge_emu_buffer_alloc = av_malloc(16*stride);
    s->edge_emu_buffer = s->edge_emu_buffer_alloc;
    if (s->flip < 0)
        s->edge_emu_buffer += 15 * stride;

    return 0;
}

int vp56_decode_frame(AVCodecContext *avctx, void *data, int *data_size,
                      uint8_t *buf, int buf_size)
{
    vp56_context_t *s = avctx->priv_data;
    AVFrame *const p = s->framep[VP56_FRAME_CURRENT];
    int mb_row, mb_col, mb_row_flip, mb_offset = 0;
    int block, y, uv, stride_y, stride_uv;
    int golden_frame = 0;
    int res;

    res = s->parse_header(s, buf, buf_size, &golden_frame);
    if (!res)
        return -1;

    p->reference = 1;
    if (avctx->get_buffer(avctx, p) < 0) {
        av_log(avctx, AV_LOG_ERROR, "get_buffer() failed\n");
        return -1;
    }

    if (res == 2)
        if (vp56_size_changed(avctx, s)) {
            avctx->release_buffer(avctx, p);
            return -1;
        }

    if (p->key_frame) {
        p->pict_type = FF_I_TYPE;
        s->default_models_init(s);
        for (block=0; block<s->mb_height*s->mb_width; block++)
            s->macroblocks[block].type = VP56_MB_INTRA;
    } else {
        p->pict_type = FF_P_TYPE;
        vp56_parse_mb_type_models(s);
        s->parse_vector_models(s);
        s->mb_type = VP56_MB_INTER_NOVEC_PF;
    }

    s->parse_coeff_models(s);

    memset(s->prev_dc, 0, sizeof(s->prev_dc));
    s->prev_dc[1][VP56_FRAME_CURRENT] = 128;
    s->prev_dc[2][VP56_FRAME_CURRENT] = 128;

    for (block=0; block < 4*s->mb_width+6; block++) {
        s->above_blocks[block].ref_frame = -1;
        s->above_blocks[block].dc_coeff = 0;
        s->above_blocks[block].not_null_dc = 0;
    }
    s->above_blocks[2*s->mb_width + 2].ref_frame = 0;
    s->above_blocks[3*s->mb_width + 4].ref_frame = 0;

    stride_y  = p->linesize[0];
    stride_uv = p->linesize[1];

    if (s->flip < 0)
        mb_offset = 7;

    /* main macroblocks loop */
    for (mb_row=0; mb_row<s->mb_height; mb_row++) {
        if (s->flip < 0)
            mb_row_flip = s->mb_height - mb_row - 1;
        else
            mb_row_flip = mb_row;

        for (block=0; block<4; block++) {
            s->left_block[block].ref_frame = -1;
            s->left_block[block].dc_coeff = 0;
            s->left_block[block].not_null_dc = 0;
            memset(s->coeff_ctx[block], 0, 64*sizeof(s->coeff_ctx[block][0]));
        }
        memset(s->coeff_ctx_last, 24, sizeof(s->coeff_ctx_last));

        s->above_block_idx[0] = 1;
        s->above_block_idx[1] = 2;
        s->above_block_idx[2] = 1;
        s->above_block_idx[3] = 2;
        s->above_block_idx[4] = 2*s->mb_width + 2 + 1;
        s->above_block_idx[5] = 3*s->mb_width + 4 + 1;

        s->block_offset[s->frbi] = (mb_row_flip*16 + mb_offset) * stride_y;
        s->block_offset[s->srbi] = s->block_offset[s->frbi] + 8*stride_y;
        s->block_offset[1] = s->block_offset[0] + 8;
        s->block_offset[3] = s->block_offset[2] + 8;
        s->block_offset[4] = (mb_row_flip*8 + mb_offset) * stride_uv;
        s->block_offset[5] = s->block_offset[4];

        for (mb_col=0; mb_col<s->mb_width; mb_col++) {
            vp56_decode_mb(s, mb_row, mb_col);

            for (y=0; y<4; y++) {
                s->above_block_idx[y] += 2;
                s->block_offset[y] += 16;
            }

            for (uv=4; uv<6; uv++) {
                s->above_block_idx[uv] += 1;
                s->block_offset[uv] += 8;
            }
        }
    }

    if (s->framep[VP56_FRAME_PREVIOUS] == s->framep[VP56_FRAME_GOLDEN])
        FFSWAP(AVFrame *, s->framep[VP56_FRAME_PREVIOUS],
                          s->framep[VP56_FRAME_UNUSED]);
    else if (s->framep[VP56_FRAME_PREVIOUS]->data[0])
        avctx->release_buffer(avctx, s->framep[VP56_FRAME_PREVIOUS]);
    if (p->key_frame || golden_frame) {
        if (s->framep[VP56_FRAME_GOLDEN]->data[0])
            avctx->release_buffer(avctx, s->framep[VP56_FRAME_GOLDEN]);
        s->framep[VP56_FRAME_GOLDEN] = p;
    }
    FFSWAP(AVFrame *, s->framep[VP56_FRAME_CURRENT],
                      s->framep[VP56_FRAME_PREVIOUS]);

    *(AVFrame*)data = *p;
    *data_size = sizeof(AVFrame);

    return buf_size;
}

void vp56_init(vp56_context_t *s, AVCodecContext *avctx, int flip)
{
    int i;

    s->avctx = avctx;
    avctx->pix_fmt = PIX_FMT_YUV420P;

    if (s->avctx->idct_algo == FF_IDCT_AUTO)
        s->avctx->idct_algo = FF_IDCT_VP3;
    dsputil_init(&s->dsp, s->avctx);
    ff_init_scantable(s->dsp.idct_permutation, &s->scantable,ff_zigzag_direct);

    avcodec_set_dimensions(s->avctx, 0, 0);

    for (i=0; i<3; i++)
        s->framep[i] = &s->frames[i];
    s->framep[VP56_FRAME_UNUSED] = s->framep[VP56_FRAME_GOLDEN];
    s->edge_emu_buffer_alloc = NULL;

    s->above_blocks = NULL;
    s->macroblocks = NULL;
    s->quantizer = -1;
    s->deblock_filtering = 1;

    s->filter = NULL;

    if (flip) {
        s->flip = -1;
        s->frbi = 2;
        s->srbi = 0;
    } else {
        s->flip = 1;
        s->frbi = 0;
        s->srbi = 2;
    }
}

int vp56_free(AVCodecContext *avctx)
{
    vp56_context_t *s = avctx->priv_data;

    av_free(s->above_blocks);
    av_free(s->macroblocks);
    av_free(s->edge_emu_buffer_alloc);
    if (s->framep[VP56_FRAME_GOLDEN]->data[0]
        && (s->framep[VP56_FRAME_PREVIOUS] != s->framep[VP56_FRAME_GOLDEN]))
        avctx->release_buffer(avctx, s->framep[VP56_FRAME_GOLDEN]);
    if (s->framep[VP56_FRAME_PREVIOUS]->data[0])
        avctx->release_buffer(avctx, s->framep[VP56_FRAME_PREVIOUS]);
    return 0;
}
