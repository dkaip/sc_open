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

#include <stdint.h>

#include "include.h"

/*
 * This UNUSED trickery is just to get rid of compiler warnings due to the
 * fact that these are just stubs and their arguments are not used.
 */
#define UNUSED(x) (void)(x)

/* MBX stubs */

uint32_t mbx_open_client(const char *name, const char *options, enum protos proto)
{
	UNUSED(name);
	UNUSED(options);
	UNUSED(proto);

#undef NAME
#define NAME "mbx_open_client()"

	FATAL ("MBX not supported\n");
    return 1;
}

uint32_t mbx_open_server(const char *name, const char *options, enum protos proto)
{
	UNUSED(name);
	UNUSED(options);
	UNUSED(proto);

#undef NAME
#define NAME "mbx_open_server()"

	FATAL ("MBX not supported\n");
    return 1;
}

uint32_t mbx_open(const char *name, const char *options, enum protos proto)
{
	UNUSED(name);
	UNUSED(options);
	UNUSED(proto);

#undef NAME
#define NAME "mbx_open()"

	FATAL ("MBX not supported\n");
    return 1;
}

uint32_t mbx_send(const char *name, enum protos proto)
{
	UNUSED(name);
	UNUSED(proto);

#undef NAME
#define NAME "mbx_send()"

	FATAL ("MBX not supported\n");
    return 1;
}

uint32_t mbx_send_reply(const char *name, enum protos proto)
{
	UNUSED(name);
	UNUSED(proto);

#undef NAME
#define NAME "mbx_send_reply()"

	FATAL ("MBX not supported\n");
    return 1;
}

void dump_mbx(struct buffer *buffer)
{
	UNUSED(buffer);

	return;
}

void user_sigusr1_handler()
{
	return;
}

void user_sigusr1_setup()
{
	return;
}

