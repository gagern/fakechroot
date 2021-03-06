/*
    libfakechroot -- fake chroot environment
    Copyright (c) 2010, 2011 Piotr Roszatycki <dexter@debian.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
*/


#include <config.h>

#ifdef HAVE_GETPEERNAME

#define _GNU_SOURCE
#include <sys/socket.h>

#ifdef AF_UNIX

#include <sys/un.h>

#include "libfakechroot.h"
#include "strlcpy.h"

#ifdef HAVE_GETPEERNAME_TYPE_ARG2___SOCKADDR_ARG__
# define SOCKADDR(addr) ((addr).__sockaddr__)
# define SOCKADDR_UN(addr) ((addr).__sockaddr_un__)
#else
# define SOCKADDR(addr) (addr)
# define SOCKADDR_UN(addr) (addr)
#endif


wrapper(getpeername, int, (int s, GETPEERNAME_TYPE_ARG2(addr), socklen_t * addrlen))
{
    int status;
    socklen_t newaddrlen;
    struct sockaddr_un newaddr;

    debug("getpeername(%d, &addr, &addrlen)", s);

    if (SOCKADDR(addr)->sa_family != AF_UNIX) {
        return nextcall(getpeername)(s, addr, addrlen);
    }

    newaddrlen = sizeof(struct sockaddr_un);
    memset(&newaddr, 0, newaddrlen);
    status = nextcall(getpeername)(s, (struct sockaddr *)&newaddr, &newaddrlen);
    if (status != 0) {
        return status;
    }
    if (newaddr.sun_family == AF_UNIX && newaddr.sun_path && *(newaddr.sun_path)) {
        char tmp[FAKECHROOT_PATH_MAX];
        strlcpy(tmp, newaddr.sun_path, FAKECHROOT_PATH_MAX);
        narrow_chroot_path(tmp);
        strlcpy(newaddr.sun_path, tmp, UNIX_PATH_MAX);
    }

    memcpy((struct sockaddr_un *)SOCKADDR_UN(addr), &newaddr, *addrlen < sizeof(struct sockaddr_un) ? *addrlen : sizeof(struct sockaddr_un));
    *addrlen = SUN_LEN(&newaddr);
    return status;
}

#else
typedef int empty_translation_unit;
#endif

#else
typedef int empty_translation_unit;
#endif
