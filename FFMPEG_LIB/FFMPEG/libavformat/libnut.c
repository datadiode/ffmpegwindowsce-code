/*
 * NUT (de)muxing via libnut
 * copyright (c) 2006 Oded Shimon <ods15@ods15.dyndns.org>
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
 * @file libnut.c
 * NUT demuxing and muxing via libnut.
 * @author Oded Shimon <ods15@ods15.dyndns.org>
 */

#include "avformat.h"
#include "riff.h"
#include <libnut.h>

#define ID_STRING "nut/multimedia container"
#define ID_LENGTH (strlen(ID_STRING) + 1)

typedef struct {
    nut_context_t * nut;
    nut_stream_header_t * s;
} NUTContext;

static const AVCodecTag nut_tags[] = {
    { CODEC_ID_MPEG4,  MKTAG('m', 'p', '4', 'v') },
    { CODEC_ID_MP3,    MKTAG('m', 'p', '3', ' ') },
    { CODEC_ID_VORBIS, MKTAG('v', 'r', 'b', 's') },
    { 0, 0 },
};

#ifdef CONFIG_MUXERS
static int av_write(void * h, size_t len, const uint8_t * buf) {
    ByteIOContext * bc = h;
    put_buffer(bc, buf, len);
    //put_flush_packet(bc);
    return len;
}

static int nut_write_header(AVFormatContext * avf) {
    NUTContext * priv = avf->priv_data;
    ByteIOContext * bc = &avf->pb;
    nut_muxer_opts_t mopts = {
        .output = {
            .priv = bc,
            .write = av_write,
        },
        .alloc = { av_malloc, av_realloc, av_free },
        .write_index = 1,
        .realtime_stream = 0,
        .max_distance = 32768,
        .fti = NULL,
    };
    nut_stream_header_t * s;
    int i;

    priv->s = s = av_mallocz((avf->nb_streams + 1) * sizeof*s);

    for (i = 0; i < avf->nb_streams; i++) {
        AVCodecContext * codec = avf->streams[i]->codec;
        int j;
        int fourcc = 0;
        int num, denom, ssize;

        s[i].type = codec->codec_type == CODEC_TYPE_VIDEO ? NUT_VIDEO_CLASS : NUT_AUDIO_CLASS;

        if (codec->codec_tag) fourcc = codec->codec_tag;
        else fourcc = codec_get_tag(nut_tags, codec->codec_id);

        if (!fourcc) {
            if (codec->codec_type == CODEC_TYPE_VIDEO) fourcc = codec_get_tag(codec_bmp_tags, codec->codec_id);
            if (codec->codec_type == CODEC_TYPE_AUDIO) fourcc = codec_get_tag(codec_wav_tags, codec->codec_id);
        }

        s[i].fourcc_len = 4;
        s[i].fourcc = av_malloc(s[i].fourcc_len);
        for (j = 0; j < s[i].fourcc_len; j++) s[i].fourcc[j] = (fourcc >> (j*8)) & 0xFF;

        ff_parse_specific_params(codec, &num, &ssize, &denom);
        av_set_pts_info(avf->streams[i], 60, denom, num);

        s[i].time_base.num = denom;
        s[i].time_base.den = num;

        s[i].fixed_fps = 0;
        s[i].decode_delay = codec->has_b_frames;
        s[i].codec_specific_len = codec->extradata_size;
        s[i].codec_specific = codec->extradata;

        if (codec->codec_type == CODEC_TYPE_VIDEO) {
            s[i].width = codec->width;
            s[i].height = codec->height;
            s[i].sample_width = 0;
            s[i].sample_height = 0;
            s[i].colorspace_type = 0;
        } else {
            s[i].samplerate_num = codec->sample_rate;
            s[i].samplerate_denom = 1;
            s[i].channel_count = codec->channels;
        }
    }

    s[avf->nb_streams].type = -1;
    priv->nut = nut_muxer_init(&mopts, s, NULL);

    return 0;
}

static int nut_write_packet(AVFormatContext * avf, AVPacket * pkt) {
    NUTContext * priv = avf->priv_data;
    nut_packet_t p;

    p.len = pkt->size;
    p.stream = pkt->stream_index;
    p.pts = pkt->pts;
    p.flags = pkt->flags & PKT_FLAG_KEY ? NUT_FLAG_KEY : 0;
    p.next_pts = 0;

    nut_write_frame_reorder(priv->nut, &p, pkt->data);

    return 0;
}

static int nut_write_trailer(AVFormatContext * avf) {
    ByteIOContext * bc = &avf->pb;
    NUTContext * priv = avf->priv_data;
    int i;

    nut_muxer_uninit_reorder(priv->nut);
    put_flush_packet(bc);

    for(i = 0; priv->s[i].type != -1; i++ ) av_freep(&priv->s[i].fourcc);
    av_freep(&priv->s);

    return 0;
}

AVOutputFormat libnut_muxer = {
    "nut",
    "nut format",
    "video/x-nut",
    "nut",
    sizeof(NUTContext),
    CODEC_ID_VORBIS,
    CODEC_ID_MPEG4,
    nut_write_header,
    nut_write_packet,
    nut_write_trailer,
    .flags = AVFMT_GLOBALHEADER,
};
#endif //CONFIG_MUXERS

static int nut_probe(AVProbeData *p) {
    if (!memcmp(p->buf, ID_STRING, ID_LENGTH)) return AVPROBE_SCORE_MAX;

    return 0;
}

static size_t av_read(void * h, size_t len, uint8_t * buf) {
    ByteIOContext * bc = h;
    return get_buffer(bc, buf, len);
}

static off_t av_seek(void * h, long long pos, int whence) {
    ByteIOContext * bc = h;
    if (whence == SEEK_END) {
        pos = url_fsize(bc) + pos;
        whence = SEEK_SET;
    }
    return url_fseek(bc, pos, whence);
}

static int nut_read_header(AVFormatContext * avf, AVFormatParameters * ap) {
    NUTContext * priv = avf->priv_data;
    ByteIOContext * bc = &avf->pb;
    nut_demuxer_opts_t dopts = {
        .input = {
            .priv = bc,
            .seek = av_seek,
            .read = av_read,
            .eof = NULL,
            .file_pos = 0,
        },
        .alloc = { av_malloc, av_realloc, av_free },
        .read_index = 1,
        .cache_syncpoints = 1,
    };
    nut_context_t * nut = priv->nut = nut_demuxer_init(&dopts);
    nut_stream_header_t * s;
    int ret, i;

    if ((ret = nut_read_headers(nut, &s, NULL))) {
        av_log(avf, AV_LOG_ERROR, " NUT error: %s\n", nut_error(ret));
        nut_demuxer_uninit(nut);
        return -1;
    }

    priv->s = s;

    for (i = 0; s[i].type != -1 && i < 2; i++) {
        AVStream * st = av_new_stream(avf, i);
        int j;

        for (j = 0; j < s[i].fourcc_len && j < 8; j++) st->codec->codec_tag |= s[i].fourcc[j]<<(j*8);

        st->codec->has_b_frames = s[i].decode_delay;

        st->codec->extradata_size = s[i].codec_specific_len;
        if (st->codec->extradata_size) {
            st->codec->extradata = av_mallocz(st->codec->extradata_size);
            memcpy(st->codec->extradata, s[i].codec_specific, st->codec->extradata_size);
        }

        av_set_pts_info(avf->streams[i], 60, s[i].time_base.num, s[i].time_base.den);
        st->start_time = 0;
        st->duration = s[i].max_pts;

        st->codec->codec_id = codec_get_id(nut_tags, st->codec->codec_tag);

        switch(s[i].type) {
        case NUT_AUDIO_CLASS:
            st->codec->codec_type = CODEC_TYPE_AUDIO;
            if (st->codec->codec_id == CODEC_ID_NONE) st->codec->codec_id = codec_get_id(codec_wav_tags, st->codec->codec_tag);

            st->codec->channels = s[i].channel_count;
            st->codec->sample_rate = s[i].samplerate_num / s[i].samplerate_denom;
            break;
        case NUT_VIDEO_CLASS:
            st->codec->codec_type = CODEC_TYPE_VIDEO;
            if (st->codec->codec_id == CODEC_ID_NONE) st->codec->codec_id = codec_get_id(codec_bmp_tags, st->codec->codec_tag);

            st->codec->width = s[i].width;
            st->codec->height = s[i].height;
            st->codec->sample_aspect_ratio.num = s[i].sample_width;
            st->codec->sample_aspect_ratio.den = s[i].sample_height;
            break;
        }
        if (st->codec->codec_id == CODEC_ID_NONE) av_log(avf, AV_LOG_ERROR, "Unknown codec?!\n");
    }

    return 0;
}

static int nut_read_packet(AVFormatContext * avf, AVPacket * pkt) {
    NUTContext * priv = avf->priv_data;
    nut_packet_t pd;
    int ret;

    ret = nut_read_next_packet(priv->nut, &pd);

    if (ret || av_new_packet(pkt, pd.len) < 0) {
        if (ret != NUT_ERR_EOF)
            av_log(avf, AV_LOG_ERROR, " NUT error: %s\n", nut_error(ret));
        return -1;
    }

    if (pd.flags & NUT_FLAG_KEY) pkt->flags |= PKT_FLAG_KEY;
    pkt->pts = pd.pts;
    pkt->stream_index = pd.stream;
    pkt->pos = url_ftell(&avf->pb);

    ret = nut_read_frame(priv->nut, &pd.len, pkt->data);

    return ret;
}

static int nut_read_seek(AVFormatContext * avf, int stream_index, int64_t target_ts, int flags) {
    NUTContext * priv = avf->priv_data;
    int active_streams[] = { stream_index, -1 };
    double time_pos = target_ts * priv->s[stream_index].time_base.num / (double)priv->s[stream_index].time_base.den;

    if (nut_seek(priv->nut, time_pos, 2*!(flags & AVSEEK_FLAG_BACKWARD), active_streams)) return -1;

    return 0;
}

static int nut_read_close(AVFormatContext *s) {
    NUTContext * priv = s->priv_data;

    nut_demuxer_uninit(priv->nut);

    return 0;
}

AVInputFormat libnut_demuxer = {
    "nut",
    "nut format",
    sizeof(NUTContext),
    nut_probe,
    nut_read_header,
    nut_read_packet,
    nut_read_close,
    nut_read_seek,
    .extensions = "nut",
};
