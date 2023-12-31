/*
 * Various utilities for ffmpeg system
 * Copyright (c) 2000, 2001, 2002 Fabrice Bellard
 * copyright (c) 2002 Francois Revol
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

#undef restrict
#include "../config.h"
#include "avformat.h"
#if defined(__MINGW32__)
#include <sys/types.h>
#include <sys/timeb.h>
#elif defined(CONFIG_OS2)
#include <string.h>
#include <sys/time.h>
#elif defined(_MSC_VER)
#else
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#endif

#ifdef UNDER_CE
#include "time_ce.h"
#else
#include <time.h>
#endif 


#ifndef HAVE_SYS_POLL_H
#if defined(__MINGW32__) || defined(_MSC_VER)
#include <winsock2.h>
#else
#include <sys/select.h>
#endif
#endif

#if defined(_MSC_VER)
#include <windows.h>
#endif

/**
 * gets the current time in micro seconds.
 */
int64_t av_gettime(void)
{
#if defined(__MINGW32__)
    struct timeb tb;
    _ftime(&tb);
    return ((int64_t)tb.time * INT64_C(1000) + (int64_t)tb.millitm) * INT64_C(1000);
#elif defined(_MSC_VER)
	LARGE_INTEGER largeInteger;
	QueryPerformanceCounter(&largeInteger);
	return largeInteger.QuadPart;
#else
	struct timeval tv;
    gettimeofday(&tv,NULL);
    return (int64_t)tv.tv_sec * 1000000 + tv.tv_usec;
#endif
}

#if !defined(HAVE_LOCALTIME_R)
struct tm *localtime_r(const time_t *t, struct tm *tp)
{
    struct tm *l;

    l = localtime(t);
    if (!l)
        return 0;
    *tp = *l;
    return tp;
}
#endif /* !defined(HAVE_LOCALTIME_R) */

#ifdef CONFIG_NETWORK
#include "network.h"

#if !defined(HAVE_INET_ATON)
#include <stdlib.h>
#include <strings.h>

int inet_aton (const char * str, struct in_addr * add)
{
    const char * pch = str;
    unsigned int add1 = 0, add2 = 0, add3 = 0, add4 = 0;

    add1 = atoi(pch);
    pch = strpbrk(pch,".");
    if (pch == 0 || ++pch == 0) goto done;
    add2 = atoi(pch);
    pch = strpbrk(pch,".");
    if (pch == 0 || ++pch == 0) goto done;
    add3 = atoi(pch);
    pch = strpbrk(pch,".");
    if (pch == 0 || ++pch == 0) goto done;
    add4 = atoi(pch);

done:
    add->s_addr=(add4<<24)+(add3<<16)+(add2<<8)+add1;

    return 1;
}
#endif /* !defined(HAVE_INET_ATON) */

/* resolve host with also IP address parsing */
int resolve_host(struct in_addr *sin_addr, const char *hostname)
{
    struct hostent *hp;

    if (!inet_aton(hostname, sin_addr)) {
        hp = gethostbyname(hostname);
        if (!hp)
            return -1;
        memcpy(sin_addr, hp->h_addr, sizeof(struct in_addr));
    }
    return 0;
}

int ff_socket_nonblock(int socket, int enable)
{
#ifdef __MINGW32__
   return ioctlsocket(socket, FIONBIO, &enable);
#else
   if (enable)
      return fcntl(socket, F_SETFL, fcntl(socket, F_GETFL) | O_NONBLOCK);
   else
      return fcntl(socket, F_SETFL, fcntl(socket, F_GETFL) & ~O_NONBLOCK);
#endif
}
#endif /* CONFIG_NETWORK */

#ifdef CONFIG_FFSERVER
#ifndef HAVE_SYS_POLL_H
int poll(struct pollfd *fds, nfds_t numfds, int timeout)
{
    fd_set read_set;
    fd_set write_set;
    fd_set exception_set;
    nfds_t i;
    int n;
    int rc;

#ifdef __MINGW32__
    if (numfds >= FD_SETSIZE) {
        errno = EINVAL;
        return -1;
    }
#endif

    FD_ZERO(&read_set);
    FD_ZERO(&write_set);
    FD_ZERO(&exception_set);

    n = -1;
    for(i = 0; i < numfds; i++) {
        if (fds[i].fd < 0)
            continue;
#ifndef __MINGW32__
        if (fds[i].fd >= FD_SETSIZE) {
            errno = EINVAL;
            return -1;
        }
#endif

        if (fds[i].events & POLLIN)  FD_SET(fds[i].fd, &read_set);
        if (fds[i].events & POLLOUT) FD_SET(fds[i].fd, &write_set);
        if (fds[i].events & POLLERR) FD_SET(fds[i].fd, &exception_set);

        if (fds[i].fd > n)
            n = fds[i].fd;
    };

    if (n == -1)
        /* Hey!? Nothing to poll, in fact!!! */
        return 0;

    if (timeout < 0)
        rc = select(n+1, &read_set, &write_set, &exception_set, NULL);
    else {
        struct timeval    tv;

        tv.tv_sec = timeout / 1000;
        tv.tv_usec = 1000 * (timeout % 1000);
        rc = select(n+1, &read_set, &write_set, &exception_set, &tv);
    };

    if (rc < 0)
        return rc;

    for(i = 0; i < (nfds_t) n; i++) {
        fds[i].revents = 0;

        if (FD_ISSET(fds[i].fd, &read_set))      fds[i].revents |= POLLIN;
        if (FD_ISSET(fds[i].fd, &write_set))     fds[i].revents |= POLLOUT;
        if (FD_ISSET(fds[i].fd, &exception_set)) fds[i].revents |= POLLERR;
    };

    return rc;
}
#endif /* HAVE_SYS_POLL_H */
#endif /* CONFIG_FFSERVER */

