/*
 * Copyright (C) 2007  FFmpeg Project
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

#include "common.h"

/**
 * Splits a single extradata buffer into the three headers that most
 * Xiph codecs use. (e.g. Theora and Vorbis)
 * Works both with Matroska's packing and lavc's packing.
 *
 * @param[in] extradata The single chunk that combines all three headers
 * @param[in] extradata_size The size of the extradata buffer
 * @param[in] first_header_size The size of the first header, used to
 * differentiate between the Matroska packing and lavc packing.
 * @param[out] header_start Pointers to the start of the three separate headers.
 * @param[out] header_len The sizes of each of the three headers.
 * @return On error a negative value is returned, on success zero.
 */
int ff_split_xiph_headers(uint8_t *extradata, int extradata_size,
                          int first_header_size, uint8_t *header_start[3],
                          int header_len[3]);
