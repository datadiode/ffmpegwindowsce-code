/*
 * RIFF codec tags
 * copyright (c) 2000 Fabrice Bellard
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
 * @file riff.h
 * internal header for RIFF based (de)muxers
 * do NOT include this in end user applications
 */

#ifndef FF_RIFF_H
#define FF_RIFF_H

offset_t start_tag(ByteIOContext *pb, const char *tag);
void end_tag(ByteIOContext *pb, offset_t start);

typedef struct AVCodecTag {
    int id;
    unsigned int tag;
} AVCodecTag;

void put_bmp_header(ByteIOContext *pb, AVCodecContext *enc, const AVCodecTag *tags, int for_asf);
int put_wav_header(ByteIOContext *pb, AVCodecContext *enc);
int wav_codec_get_id(unsigned int tag, int bps);
void get_wav_header(ByteIOContext *pb, AVCodecContext *codec, int size);

extern const AVCodecTag codec_bmp_tags[];
extern const AVCodecTag codec_wav_tags[];

unsigned int codec_get_tag(const AVCodecTag *tags, int id);
enum CodecID codec_get_id(const AVCodecTag *tags, unsigned int tag);
/**
 * @deprecated Use av_codec_get_tag instead.
 */
unsigned int codec_get_bmp_tag(int id) attribute_deprecated;
/**
 * @deprecated Use av_codec_get_tag instead.
 */
unsigned int codec_get_wav_tag(int id) attribute_deprecated;
/**
 * @deprecated Use av_codec_get_id instead.
 */
enum CodecID codec_get_bmp_id(unsigned int tag) attribute_deprecated;
/**
 * @deprecated Use av_codec_get_id instead.
 */
enum CodecID codec_get_wav_id(unsigned int tag) attribute_deprecated;
void ff_parse_specific_params(AVCodecContext *stream, int *au_rate, int *au_ssize, int *au_scale);

#endif
