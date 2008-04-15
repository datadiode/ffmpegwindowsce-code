/*
 * Buffered file io for ffmpeg system
 * Copyright (c) 2001 Fabrice Bellard
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
#include "avformat.h"

//#ifdef _WIN32_WCE
#if  defined(WIN32) || defined(UNDER_CE)


/* standard file protocol */

static int file_open(URLContext *h, const char *filename, int flags)
{
    char *access;
    FILE *fd;

    strstart(filename, "file:", &filename);

    if (flags & URL_RDWR) {
        access = "w+b";
    } else if (flags & URL_WRONLY) {
        access = "wb";
    } else {
        access = "rb";
    }
    fd = fopen(filename, access);
    if (fd == NULL)
        return -ENOENT;
    h->priv_data = (void *) fd;
    return 0;
}

static int file_read(URLContext *h, unsigned char *buf, int size)
{
    FILE *fd = h->priv_data;
    return fread(buf, 1, size, fd);
}

static int file_write(URLContext *h, unsigned char *buf, int size)
{
    FILE * fd = h->priv_data;
    return fwrite(buf, 1, size, fd);
}

/* XXX: use llseek */
static offset_t file_seek(URLContext *h, offset_t pos, int whence)
{
    FILE *fd = (FILE *)h->priv_data;
//ms99    return fseek(fd, pos, whence);
	long ret = -1;
	if( fseek(fd,pos,whence)==0 ){
		ret = ftell(fd);
	}
	return ret; 
}

static int file_close(URLContext *h)
{
    FILE *fd = (FILE *)h->priv_data;
    return fclose(fd);
}

URLProtocol file_protocol = {
    "file",
    file_open,
    file_read,
    file_write,
    file_seek,
    file_close,
};


#else

#include <fcntl.h>
#ifndef CONFIG_MSVC
//ms99 #include <unistd.h>
//ms99 #include <sys/time.h>
#ifdef UNDER_CE
#include "time_ce.h"
#else
#include <time.h>
#endif 

#endif

/* standard file protocol */

static int file_open(URLContext *h, const char *filename, int flags)
{
    int access;
    int fd;

    strstart(filename, "file:", &filename);

    if (flags & URL_RDWR) {
        access = O_CREAT | O_TRUNC | O_RDWR;
    } else if (flags & URL_WRONLY) {
        access = O_CREAT | O_TRUNC | O_WRONLY;
    } else {
        access = O_RDONLY;
    }
#if defined(__MINGW32__) || defined(CONFIG_OS2) || defined(__CYGWIN__)
    access |= O_BINARY;
#endif
    fd = open(filename, access, 0666);
    if (fd < 0)
        return -ENOENT;
    h->priv_data = (void *)(size_t)fd;
    return 0;
}

static int file_read(URLContext *h, unsigned char *buf, int size)
{

    int fd = (size_t)h->priv_data;
    return read(fd, buf, size);
}

static int file_write(URLContext *h, unsigned char *buf, int size)
{
    int fd = (size_t)h->priv_data;
    return write(fd, buf, size);
}

/* XXX: use llseek */
static offset_t file_seek(URLContext *h, offset_t pos, int whence)
{
    int fd = (size_t)h->priv_data;
    return lseek(fd, pos, whence);
}

static int file_close(URLContext *h)
{
    int fd = (size_t)h->priv_data;
    return close(fd);
}

URLProtocol file_protocol = {
    "file",
    file_open,
    file_read,
    file_write,
    file_seek,
    file_close,
};

/* pipe protocol */

static int pipe_open(URLContext *h, const char *filename, int flags)
{
    int fd;

    if (flags & URL_WRONLY) {
        fd = 1;
    } else {
        fd = 0;
    }
#if defined(__MINGW32__) || defined(CONFIG_OS2) || defined(__CYGWIN__)
    setmode(fd, O_BINARY);
#endif
    h->priv_data = (void *)(size_t)fd;
    h->is_streamed = 1;
    return 0;
}

static int pipe_read(URLContext *h, unsigned char *buf, int size)
{
    int fd = (size_t)h->priv_data;
    return read(fd, buf, size);
}

static int pipe_write(URLContext *h, unsigned char *buf, int size)
{
    int fd = (size_t)h->priv_data;
    return write(fd, buf, size);
}

static int pipe_close(URLContext *h)
{
    return 0;
}

URLProtocol pipe_protocol = {
    "pipe",
    pipe_open,
    pipe_read,
    pipe_write,
    NULL,
    pipe_close,
};

#endif
