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
/*
 * binary_buffer.c
 *
 *  Created on: Dec 29, 2016
 *      Author: Douglas Kaip
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "include.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))


/********************************************/

void binary_buffree(struct binary_buffer *buffer)
{

#undef NAME
#define NAME "binary_buffree()"

    if (buffer->data != (uint8_t *)NULL)
    {
        DEBUG (5, DEBUG_HDR, "Freeing all space for buffer\n");
        free(buffer->data);
        buffer->data = (uint8_t *)NULL;
    }

    buffer->current_in_use = 0;
    buffer->current_size = 0;
}

void binary_bufncat(struct binary_buffer *binbuf, const uint8_t *data, uint32_t length)
{

#undef NAME
#define NAME "binary_bufncat()"

    DEBUG (5, DEBUG_HDR, "Called, data length <%d>\n", length);
    DEBUG (5, DEBUG_HDR, "Available space <%d>\n", (binbuf->current_size - binbuf->current_in_use));

    if (binbuf->data == (uint8_t *)NULL)
    {
        binbuf->current_size = MAX(BINARY_BUFFER_DEFAULT_SIZE, length);
    	binbuf->data = (uint8_t *)malloc(binbuf->current_size);
        if (binbuf->data == (uint8_t *)NULL)
        {
            ERROR ("Can't malloc more space, exiting\n");
            exit (1);
        }
    }

    if ((length + binbuf->current_in_use) > binbuf->current_size)
    {
        binbuf->current_size = MAX((BINARY_BUFFER_DEFAULT_SIZE + binbuf->current_in_use), (length + binbuf->current_in_use));

        DEBUG (5, DEBUG_HDR, "New size <%d>\n", binbuf->current_size);

        binbuf->data = (uint8_t *)realloc(binbuf->data, binbuf->current_size);
        if (binbuf->data == (uint8_t *)NULL)
        {
            ERROR ("Can't realloc more space, exiting\n");
            exit (1);
        }
    }

    memcpy(&binbuf->data[binbuf->current_in_use], data, length);
    binbuf->current_in_use += length;

    DEBUG (5, DEBUG_HDR, "Remaining space <%d>\n", (binbuf->current_size- binbuf->current_in_use));
    return;
}

void binary_bufncpy(struct binary_buffer *buffer, const uint8_t *data, uint32_t length)
{

#undef NAME
#define NAME "binary_bufncpy()"

    DEBUG (5, DEBUG_HDR, "Called, data length <%d>\n", length);

    buffer->current_in_use = 0;
    binary_bufncat(buffer, data, length);
    return;
}

void binary_buf_empty(struct binary_buffer *buffer)
{
	buffer->current_in_use = 0;
	return;
}

inline uint8_t *binary_buffer_data(const struct binary_buffer *buffer)
{
    return(buffer->data);
}

/********************************************/

inline uint32_t binary_buffer_num_data(const struct binary_buffer *buffer)
{
    return(buffer->current_in_use);
}

/********************************************/

void binary_bufdump_hex(const struct binary_buffer *binbuf)
{
	uint8_t *b = (uint8_t *)NULL;

#undef NAME
#define NAME "bufdump_hex()"

    printf("Buffer dump:");
    b = binary_buffer_data(binbuf);
    for (uint32_t i=0; i<binary_buffer_num_data(binbuf); i++)
    {
        if ((i % 16) == 0)
        {
            printf("\n");
        }

        printf("%.2X ", b[i]);

        /******************
        if (isprint((unsigned char) b[i])) {
            printf(" %c ", (unsigned char) b[i]);
        } else {
            printf("%.2X ", (unsigned char) b[i]);
        }
        ******************/
    }
    printf("\nBuffer end of dump\n");
}


