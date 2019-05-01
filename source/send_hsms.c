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

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "include.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))

//static int send_hsmsII(char *, int, unsigned int, char *, int *);
//static int send_hsmsII(char *name, int fd, uint32_t system_bytes, char *hdr, int *wait_flag);

static uint32_t send_hsmsII(const char *name, int fd, uint32_t system_bytes, const char *log_hdr, bool *wait_flag);

/********************************************/

uint32_t send_hsms(const char *name, int fd, bool *wait_flag)
{
	uint32_t iret = 0;
	char *log_hdr = (char *)NULL;

#undef NAME
#define NAME "send_hsms()"

    DEBUG (3, DEBUG_HDR, "Called, name <%s>, fd <%d>\n", name, fd);

    iret = 0;

    if (strcmp(get_option("hsms_logging=", name), "off") == 0)
    {
        set_logging(name, 2);
    }

    if (strcmp(get_option("hsms_logging=", name), "on") == 0)
    {
        set_logging(name, 3);
    }

    /* LOG ("Send primary HSMS msg to <%s>\n", name); */

    log_hdr = "Send primary HSMS msg to";
    iret = send_hsmsII(name, fd, secs_inc_system_bytes(), log_hdr, wait_flag);
    if (iret)
    {
        ERROR ("Error sending hsms message to <%s>\n", name);
    }

    DEBUG (3, DEBUG_HDR, "Returning <%d>\n", iret);
    return iret;
}

/********************************************/

uint32_t send_hsms_reply(const char *name, int fd)
{
	uint32_t iret = 0;
	bool wait_flag = false;;
	char *log_hdr = (char *)NULL;

#undef NAME
#define NAME "send_hsms_reply()"

    DEBUG (3, DEBUG_HDR, "Called, name <%s>, fd <%d>\n", name, fd);

    iret = 0;

    /* LOG ("Send secondary reply HSMS msg to <%s>\n", name); */

    log_hdr = "Send secondary reply HSMS msg to";
    iret = send_hsmsII(name, fd, secs_get_system_bytes(), log_hdr, &wait_flag);
    if (iret) {
        ERROR ("Error sending hsms message to <%s>\n", name);
    }

    DEBUG (3, DEBUG_HDR, "Returning <%d>\n", iret);
    return iret;
}

/********************************************/

static uint32_t send_hsmsII(const char *name, int fd, uint32_t system_bytes, const char *log_hdr, bool *wait_flag)
{
	uint32_t timer_idx, t3, stream, function, device_id/*, primary_msg*/;
	uint32_t wait_bit = 0;
	uint32_t iret = 0;
/*int data_size;*/
/*int saved_idx, program_idx;*/
	uint8_t header[10];
	char comment[128];
struct binary_buffer encoded_secs_msg = INIT_BINARY_BUFFER;
struct buffer reply = INIT_BUFFER;
struct buffer log = INIT_BUFFER;

#undef NAME
#define NAME "send_hsmsII()"

    DEBUG (3, DEBUG_HDR, "Called, fd <%d>, system_bytes <%d>\n", fd, system_bytes);

    while (1)
    {  /* Just do once */
        binary_buf_empty(&encoded_secs_msg);
        iret = secs_encode(&encoded_secs_msg, &stream, &function, &device_id, &wait_bit, &log);
        if (iret)
        {
            ERROR ("Error trying to encode secs message for <%s>\n", name);
            break;
        }

        FD_LOG1(name) ("%s <%s>, <s%df%d>\n", log_hdr, name, stream, function);
        FD_LOG2(name) ("%s", bufmkprint(&log));

        memset(header, '\0', sizeof(header));
        header[0] = (uint8_t)((device_id >> 8) & 255);
        header[1] = (uint8_t)(device_id & 255);
        header[2] = (uint8_t)stream;
        if (wait_bit) header[2] = header[2] | 0x80;
        header[3] = (uint8_t)function;
        memcpy(&header[6], &system_bytes, sizeof(system_bytes));
        swap(&header[6], 4);

        iret = send_hsmsI(name, fd, header, binary_buffer_data(&encoded_secs_msg), binary_buffer_num_data(&encoded_secs_msg));
        if (iret)
        {
            ERROR ("Can't send HSMS message\n");
            break;
        }

        if (wait_bit)
        {
            DEBUG (3, DEBUG_HDR, "Adding entry to open transaction table\n");
            t3 = (uint32_t)atoi(get_option("t3=", name));
            sprintf(comment, "T3 timeout waiting for <S%dF%d> reply at line <%s>",
                stream, function, get_line_number());
            timer_idx = start_timer(-1, t3, get_program_token_idx(), 
                get_option("timeout_msg=", name), comment);  
            open_trans_table_add(get_program_token_idx(), system_bytes, timer_idx, t3);
            *wait_flag = true;

        }
        else
        {
            *wait_flag = false;
        }
        break;
    }

    buffree(&reply);
    binary_buffree(&encoded_secs_msg);
    buffree(&log);
    DEBUG (3, DEBUG_HDR, "Returning <%d>\n", iret);
    return iret;
}

/********************************************/

uint32_t send_hsmsI(const char *name, int fd, uint8_t *header, const uint8_t *body, uint32_t size)
{
	uint32_t i, iret;
	union U4Union u4_union;

//unsigned int length;
/*char len[4],*/
	uint8_t length_plus_header[14];
#undef NAME
#define NAME "send_hsmsI()"

    DEBUG (3, DEBUG_HDR, "Called, fd <%d>\n", fd);

//    length = 10 + size;
    u4_union.the_uint32 = 10 + size;
    swap(u4_union.the_bytes, 4);
//    memcpy(len, &length, sizeof(length));

    FD_LOG3(name) ("Sending data to <%s>:\n", name);
    if (get_logging(name) >= 3)
    {
        LOG_DATA(name) ("    Length   ", (uint8_t *)u4_union.the_bytes, 4);
        LOG_DATA(name) ("    Header   ", (uint8_t *)header, 10);
        LOG_DATA(name) ("    body     ", (uint8_t *)body, size);
    }

//    memcpy(&tmp[0], &length, 4);
    memcpy(&length_plus_header[0], u4_union.the_bytes, 4);
    memcpy(&length_plus_header[4], header, 10);

    /* Send the length and header */
    iret = my_write(name, fd, length_plus_header, sizeof(length_plus_header));
    if (iret)
    {
        ERROR ("Error while sending hsms length and header to <%s>\n", name);
        return 1;
    }

    /* Send the rest of the message */
//    p = body;
    // TODO why are we writing out bytes when we could blast out the rest of the message???
    for (i=0; i<size; i++)
    {
        iret = my_writec(name, fd, *body);
        if (iret)
        {
            ERROR ("Error while sending hsms <%s>\n", name);
            return 1;
        }
        body++;
    }

    DEBUG (3, DEBUG_HDR, "Returning success\n");
    return 0;
}

/********************************************/

