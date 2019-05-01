/************************************************************************
sc - Station Controller
Copyright (C) 2005 
Written by John F. Poirier DBA Edge Integration

Contact information:
    john@edgeintegration.com

    Edge Integration
    885 North Main Street
    Raynham MA, 02767

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
************************************************************************/

#include <errno.h>
#include <unistd.h>
#include "include.h"

/*************************************************/
/*
 * It will read up to size - 1 characters and then null terminate buff
 */
uint32_t my_read(const char *name, int fd, char *buf, uint32_t size, uint32_t *cnt)
{
	int64_t iret = 0;

#undef NAME
#define NAME "my_read()"

    DEBUG (3, DEBUG_HDR, "Called, name <%s>, fd <%d>, size <%d>\n", name, fd, size);

    DEBUG (4, DEBUG_HDR, "Reading...\n");
    iret = read(fd, buf, size-1);
    DEBUG (4, DEBUG_HDR, "Read <%d> bytes\n", iret);

    if (iret < 0)
    {
        ERROR ("Read failed, connection <%s>\n", name);
        return 1;

    }
    else if (iret == 0)
    {
        /* ERROR ("Zero bytes read, client <%d>, possible client disconnect detected\n", fd); */
        LOG ("Zero bytes read from <%s>, possible client disconnect detected\n", name);
        client_disconnect(name, fd);
        return 1; 

    }

    *cnt = (uint32_t)iret;
    buf[iret] = '\0'; 
    DEBUG (4, DEBUG_HDR, "Read <%s>\n", buf);

    DEBUG (3, DEBUG_HDR, "Returning success, cnt <%u>\n", *cnt);
    return 0;
}

/*************************************************/

uint32_t my_readc(const char *name, int fd, uint8_t *buf)
{
	int64_t iret = 0;

#undef NAME
#define NAME "my_readc()"

    DEBUG (3, DEBUG_HDR, "Called, name <%s>, fd <%d>\n", name, fd);
    DEBUG (3, DEBUG_HDR, "Reading...\n");

    iret = read(fd, buf, 1);
    DEBUG (4, DEBUG_HDR, "Read <%d> bytes\n", iret);

    if (iret < 0)
    {
        ERROR ("Read failed, connection <%s>\n", name);
        client_disconnect(name, fd);
        return 1;
    }
    else if (iret == 0)
    {
        /* ERROR ("Zero bytes read, client <%d>, possible client disconnect detected\n", fd); */
        LOG ("Zero bytes read from <%s>, possible client disconnect detected\n", name);
        client_disconnect(name, fd);
        return 1;
    }

    DEBUG (3, DEBUG_HDR, "Returning success, fd <%d>, char <%.2x>\n", fd, (unsigned char) *buf);
    return 0;
}

/*************************************************/

// TODO why does this return negative instead of just non-zero like the others for an error?

int64_t my_readx(const char *name, int fd, uint8_t *buf, uint32_t len)
{
	int64_t iret = 0;

#undef NAME
#define NAME "my_readx()"

    DEBUG (3, DEBUG_HDR, "Called, name <%s>, fd <%d>, len <%d>\n", name, fd, len);
    DEBUG (3, DEBUG_HDR, "Reading...\n");

    iret = read(fd, buf, len);
    DEBUG (4, DEBUG_HDR, "Read <%d> bytes\n", iret);

    if (iret < 0)
    {
        ERROR ("Read failed, connection <%s>\n", name);
        client_disconnect(name, fd);
        return -1;
    }
    else if (iret == 0)
    {
        /* ERROR ("Zero bytes read, client <%d>, possible client disconnect detected\n", fd); */
        LOG ("Zero bytes read from <%s>, possible client disconnect detected\n", name);
        client_disconnect(name, fd);
        return -1;
    }

    DEBUG (3, DEBUG_HDR, "Returning success, fd <%d>, bytes_read <%d>\n", fd, iret);
    return iret;  /* Return number of bytes read */
}

/*************************************************/

uint32_t my_write(const char *name, int fd, const uint8_t *buf, uint32_t size)
{
	int64_t iret = 0;

#undef NAME
#define NAME "my_write()"

    DEBUG (3, DEBUG_HDR, "Called, name <%s>, fd <%d>, size <%d>\n", name, fd, size);
    iret = 0;

    /* if (g_syntax_check) {
        return 0;
    } */

    iret = write(fd, buf, size);

    if (iret == -1)
    {
       ERROR ("Can't write data, connection <%s>\n", name);
       return 1;
    }

    DEBUG (3, DEBUG_HDR, "Returning success\n");
    return 0;
}

/*************************************************/

uint32_t my_writec(const char *name, int fd, uint8_t byte)
{
	int64_t iret = 0;

#undef NAME
#define NAME "my_writec()"

    DEBUG (3, DEBUG_HDR, "Called, name <%s>, fd <%d>, char <%x>\n", name, fd, byte);
    iret = 0;

    /* if (g_syntax_check) {
        return 0;
    } */

    iret = write(fd, &byte, 1);
    if (iret == -1)
    {
       ERROR ("Can't write data, connection <%s>\n", name);
       return 1;
    }

    DEBUG (3, DEBUG_HDR, "Returning success\n");
    return 0;
}

/********************************************/

