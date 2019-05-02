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

#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "include.h"

/*
 * Functions in this file are use when receiving stuff from an HSMS connection
 */

/********************************************/

uint32_t recv_hsms(const char *name, int fd, uint32_t *program_idx, bool *primary_msg_flag)
{
	uint32_t iret = 0;
	bool arrays = false;
	uint32_t stype, system_bytes/*, length*/;
	char num[64], sbuf[128];
	char var_prefix[32];
	uint8_t header[10];
	struct buffer log = INIT_BUFFER;
	struct buffer tmp_name = INIT_BUFFER;

#undef NAME
#define NAME "recv_hsms()"

    DEBUG (3, DEBUG_HDR, "Called, name <%s>, fd <%d>\n", name, fd);

    if (strcmp(get_option("hsms_logging=", name), "off") == 0)
    {
        set_logging(name, 2);
    }
    else if (strcmp(get_option("hsms_logging=", name), "on") == 0)
    {
        set_logging(name, 3);
    }

    arrays = true;
    if (strcmp(get_option("arrays=", name), "no") == 0)
    {
        arrays = false;
    }

//    bufcpy(&encoded_secs_msg, "");

    struct binary_buffer binary_secs_msg = INIT_BINARY_BUFFER;
    iret = recv_hsmsI(name, fd, header, &binary_secs_msg);

    if (iret)
    {
    	;
    }
    else
    {
        stype = header[5];
        DEBUG (3, DEBUG_HDR, "stype <%d>\n", stype);

        if (stype == 0)
        {
            /* Secs message */ 
 
            sprintf(var_prefix, "S%dF%d", (unsigned char) header[2]&0x7F, (unsigned char) header[3]);
            sprintf(num, "%d", (((unsigned char) header[0] & 0x7F) * 256) + (unsigned char) header[1]); 
            bufcpy(&log, "");
            bufcat(&log, var_prefix);
            bufcat(&log, " DEVICE_ID=");
            bufcat(&log, num);

            bufcpy(&tmp_name, "SXFY");
            tmp_variable_add(bufdata(&tmp_name), var_prefix, -1);

            bufcpy(&tmp_name, var_prefix);
            bufcat(&tmp_name, ".DEVICE_ID");
            tmp_variable_add(bufdata(&tmp_name), num, -1);

            
            bufcpy(&tmp_name, var_prefix);
            bufcat(&tmp_name, ".WAIT_BIT");
            if (header[2] & 0x80)
            {
                bufcat(&log, " WAIT_BIT=1");
                tmp_variable_add(bufdata(&tmp_name), "1", -1);
            }
            else
            {
                bufcat(&log, " WAIT_BIT=0");
                tmp_variable_add(bufdata(&tmp_name), "0", -1);
            }

            bufcat(&log, "\n");

            memcpy(&system_bytes, &header[6], sizeof(system_bytes));
            swap((uint8_t *) &system_bytes, (uint32_t)4);
            if (arrays == true)
            {
                iret = secs_decode(var_prefix, &binary_secs_msg, &log);

            }
            else
            {
                iret = secs_decode_no_arrays(var_prefix, &binary_secs_msg, &log);
            }

            /* See if we got a primary or secondary reply message.  Check if y in SxFy 
               is even (primary) or odd (secondary) */

            /* Fill in the logging buffer */
            sprintf(sbuf, "s%df%d", (unsigned char) header[2] & 0x7F, (unsigned char) header[3]);

            if ((unsigned char) header[3] % 2)
            {
                /* Odd function number, so must be a primary message */
                FD_LOG1(name) ("Recv primary HSMS msg from <%s>, <%s>\n", name, sbuf);
                FD_LOG2(name) ("%s", bufmkprint(&log));
                secs_set_system_bytes(system_bytes);
                *primary_msg_flag = true;

            }
            else
            {
                FD_LOG1(name) ("Recv secondary HSMS reply msg from <%s>, <%s>\n", name, sbuf);
                FD_LOG2(name) ("%s", bufmkprint(&log));
                *primary_msg_flag = false;
           
                /* Get the program idx (where we left of in the program) */
                iret = open_trans_table_del(system_bytes, program_idx);
                if (iret)
                {
                    /* Received a message that was not in the open_transaction_table */
                    ERROR ("Discarding secs message\n");
                    iret = 2;  /* Error 2 means a protocol error, not really bad */
                }
            }

        /* Received a Select Request.. Reply with a Select Response */
        }
        else  if (stype == 1)
        {
            FD_LOG1(name) ("Recv HSMS \"Select Request\" from <%s>\n", name);
            header[5] = 0x02;  /* Stype Select response */
            FD_LOG1(name) ("Send HSMS \"Select Response\" to <%s>\n", name);
            iret = send_hsmsI(name, fd, header, (uint8_t *)"", 0);
            if (iret) {
                ERROR ("Can't connect to hsms server <%s>\n", name);
            }
            iret = 2;

        /* Received a Link Test Request.. Reply with a Link Test Response */
        }
        else  if (stype == 5)
        {
            FD_LOG1(name) ("Recv HSMS \"Link Test Request\" from <%s>\n", name);
            header[5] = 0x06;  /* Stype link test response */
            FD_LOG1(name) ("Send HSMS \"Link Test Response\" to <%s>\n", name);
            iret = send_hsmsI(name, fd, header, (uint8_t *)"", 0);
            if (iret) {
                ERROR ("Can't connect to hsms server <%s>\n", name);
            }
            iret = 2;

        }
        else  if (stype == 9)
        {
            FD_LOG1(name) ("Recv HSMS \"Disconnect\" from <%s>\n", name);
            iret = 2;

        }
        else
        {
            FATAL ("Unknown hsms stype <%d>\n", stype);
        }
    }

    binary_buffree(&binary_secs_msg);
    buffree(&log);
    buffree(&tmp_name);
    DEBUG (3, DEBUG_HDR, "Returning iret <%d>, program_idx\n", iret, *program_idx);
    return iret;
}

/********************************************/

uint32_t recv_hsmsI(const char *name, int fd, uint8_t *header, struct binary_buffer *body)
{
//#define MAX_READ 1000  /* Max number of bytes to read each time */
#define MAX_READ 2048  /* Max number of bytes to read each time */
	uint32_t  t8 = 0;
	uint32_t i = 0;
//	uint32_t length = 0;
	uint32_t num_reads = 0;
	uint32_t num_max_reads = 0;
	int64_t bytes_read = 0;
	uint8_t c = 0;
//	uint8_t len[4] = {0, 0, 0, 0};
	union U4Union u4_union;
	uint8_t buf[MAX_READ];
#ifdef __CYGWIN__
  static sigjmp_buf timer_t8;
#else
	static jmp_buf timer_t8;
#endif

#undef NAME
#define NAME "recv_hsmsI()"

    DEBUG (3, DEBUG_HDR, "Called, fd <%d>\n", fd);

    t8 = (uint32_t)atoi(get_option("t8=", name));

    /* FD_LOG1(name) ("Recv data from <%s>:\n", name); This is confusing in the logs */

    /* Get the number of message bytes (first 4 bytes) */
    for (uint32_t i=0; i<4; i++)
    {
//        if (sigsetjmp(timer_t8, 1) != 0)
        if (do_the_setsigjump(timer_t8, 1) != 0)
        {
            ERROR ("T8 Timeout detected while waiting for length bytes from <%s>\n", name);
            return 1; 
        }

        start_proto_timer(t8, 8, &timer_t8);

        if (my_readc(name, fd, &c))
        {
            stop_proto_timer(8);
            ERROR ("Error while reading length bytes from <%s>\n", name);
            return 1; 
        }

        stop_proto_timer(8);

        u4_union.the_bytes[i] = c;
//        len[i] = c;
    }

//    memcpy(&length, len, sizeof(len));
//    swap((uint8_t *) &length, (uint32_t)4);
    swap( u4_union.the_bytes, sizeof(u4_union.the_bytes));
    DEBUG (3, DEBUG_HDR, "Length <%d>\n", u4_union.the_uint32);

    if (get_logging(name) >= 3)
    {
        LOG_DATA(name) ("    Length   ", u4_union.the_bytes, 4);
    }

    /* Get the message header (next 10 bytes) */
    for (i=0; i<10; i++)
    {
//        if (sigsetjmp(timer_t8, 1) != 0)
        if (do_the_setsigjump(timer_t8, 1) != 0)
        {
            stop_proto_timer(8);
            ERROR ("T8 Timeout detected while waiting for header data from <%s>\n", name);
            return 1; 
        }
        start_proto_timer(t8, 8, &timer_t8);
        if (my_readc(name, fd, &c))
        {
            ERROR ("Error while reading header from <%s>\n", name);
            return 1; 
        }
        stop_proto_timer(8);
        header[i] = c;
    }

    if (get_logging(name) >= 3)
    {
        LOG_DATA(name) ("    Header   ", header, 10);
    }

    num_reads = 14;  /* From above */
    num_max_reads = 0;

    /* Get the rest of the message */
    i = u4_union.the_uint32 - 10;
    while (i)
    {  /* for (i=0; i<length-10; i++) { */
//        if (sigsetjmp(timer_t8, 1) != 0)
        if (do_the_setsigjump(timer_t8, 1) != 0)
        {
            stop_proto_timer(8);
            ERROR ("T8 Timeout detected while waiting for data from <%s>\n", name);
            return 1; 
        }
        start_proto_timer(t8, 8, &timer_t8);
        bytes_read = my_readx(name, fd, buf, MAX_READ);
        if (bytes_read == -1)
        {
            stop_proto_timer(8);
            ERROR ("Error while reading body from <%s>\n", name);
            return 1; 
        }
        stop_proto_timer(8);
        /* bufncpy(body, &c, 1); **************/

        if (bytes_read == MAX_READ)
        {
            num_max_reads++;
        }

        num_reads++;
        // bytes_read should never be HUGE so conversion to U32 should be okay
        i = i - (uint32_t)bytes_read;
        binary_bufncat(body, buf, (uint32_t)bytes_read);
    }

    if (get_logging(name) >= 3)
    {
        LOG_DATA(name) ("    body     ", binary_buffer_data(body), binary_buffer_num_data(body));
    }

    FD_LOG2(name) ("Finished reading hsms message, size <%d> (<%d> reads, <%d> of max size <%d>)\n", 
    		u4_union.the_uint32, num_reads, num_max_reads, MAX_READ);

    DEBUG (3, DEBUG_HDR, "Returning success\n");
    return 0;
}

/********************************************/


