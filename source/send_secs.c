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

static uint32_t send_secsI(const char *, int, uint8_t *, uint8_t);
static uint32_t send_secsII(const char *name, int fd, uint32_t system_bytes, char *logging_hdr, bool *wait_flag);

/********************************************/

uint32_t send_secs(const char * name, int fd, bool *wait_flag)
{
	uint32_t iret = 0;
	char *hdr = (char *)NULL;

#undef NAME
#define NAME "send_secs()"

    DEBUG (3, DEBUG_HDR, "Called, name <%s>, fd <%d>\n", name, fd);

    iret = 0;

    if (strcmp(get_option("secsI_logging=", name), "off") == 0)
    {
        set_logging(name, 2);
    }
    else if (strcmp(get_option("secsI_logging=", name), "on") == 0)
    {
        set_logging(name, 3);
    }

    /* FD_LOG1(name) ("Send primary SECS msg to <%s>\n", name); */

    hdr = "Send primary SECS msg to";
    iret = send_secsII(name, fd, secs_inc_system_bytes(), hdr, wait_flag);
    if (iret) {
        ERROR ("Error sending secsII message to <%s>\n", name);
    }

    DEBUG (3, DEBUG_HDR, "Returning <%d>\n", iret);
    return iret;
}

/********************************************/

uint32_t send_secs_reply(const char *name, int fd)
{
	uint32_t iret = 0;
	bool wait_flag = false;
    char *logging_hdr = (char *)NULL;

#undef NAME
#define NAME "send_secs_reply()"

    DEBUG (3, DEBUG_HDR, "Called, name <%s>, fd <%d>\n", name, fd);

    iret = 0;

    logging_hdr = "Send secondary reply SECS msg to";
    iret = send_secsII(name, fd, secs_get_system_bytes(), logging_hdr, &wait_flag);
    if (iret) {
        ERROR ("Error sending secsII message to <%s>\n", name);
    }

    DEBUG (3, DEBUG_HDR, "Returning <%d>\n", iret);
    return iret;
}

/********************************************/

static uint32_t send_secsII(const char *name, int fd, uint32_t system_bytes, char *logging_hdr, bool *wait_flag)
{
	uint32_t timer_idx = 0;
	uint32_t t3 = 0, wait_bit = 0, stream = 0, function = 0, device_id = 0;
	bool primary_msg = false;
	uint32_t rbit = 0, msg_size = 0, iret = 0;
	uint32_t /*tmp_idx, */block_num, retry, data_size;
	uint32_t saved_idx = 0, program_idx = 0;
	uint8_t block_size = 0;
	uint8_t *msg_ptr = (uint8_t *)NULL;
	char comment[128];
	uint8_t header[10], data[244], block[254];
	struct binary_buffer encoded_secs_msg = INIT_BINARY_BUFFER;
/* struct buffer reply = INIT_BUFFER; */
struct buffer log = INIT_BUFFER;

#undef NAME
#define NAME "send_secsII()"

	comment[0] = 0;

    DEBUG (3, DEBUG_HDR, "Called, fd <%d>, system_bytes <%d>, hdr <%s>\n", fd, system_bytes, logging_hdr);

    while (1)
    {  /* Just do once */
    	binary_buf_empty(&encoded_secs_msg);
        iret = secs_encode(&encoded_secs_msg, &stream, &function, &device_id, &wait_bit, &log);
        if (iret)
        {
            ERROR ("Error trying to encode secs message for <%s>\n", name);
            break;
        }

        if (g_debug >= 5) binary_bufdump_hex(&encoded_secs_msg);

        FD_LOG1(name) ("%s <%s>, <s%df%d>\n", logging_hdr, name, stream, function);
        FD_LOG2(name) ("%s", bufmkprint(&log));  

        msg_size = binary_buffer_num_data(&encoded_secs_msg);
        msg_ptr = binary_buffer_data(&encoded_secs_msg);
        block_num = 1;

        DEBUG (3, DEBUG_HDR, "Msg size <%d>\n", msg_size);
        break;
    } 

    rbit = (uint32_t)atoi(get_option("rbit=", name));

    /* while (msg_size) { */
    iret = 0;
    msg_size = 0;
    data_size = 0;
    while (1)
    {  /* Repeat for each secsI block */
        if (iret)
        {
            break;
        }

        data_size = MIN(244, msg_size);
        DEBUG (3, DEBUG_HDR, "Data size <%d>\n", data_size);
        msg_size = msg_size - data_size;
        memcpy(data, msg_ptr, data_size);
        msg_ptr = msg_ptr + data_size;
        DEBUG (3, DEBUG_HDR, "Msg size <%d>\n", msg_size);

        header[0] = (device_id >> 8) & 255;
        if (rbit)
        {
        	header[0] |= 0x80;
        }

        header[1] = (uint8_t)(device_id & 255);
        header[2] = (uint8_t)stream;
        if (wait_bit)
        {
        	header[2] |= 0x80;
        }

        header[3] = (uint8_t)function;
        header[4] = (uint8_t)((block_num >> 8) & 255);
        if (msg_size == 0)
        {
        	header[4] |= 0x80;
        }

        header[5] = block_num & 255;
        // TODO consider doing the union/swap thing here
        header[6] = (uint8_t)((system_bytes >> 24) & 0xFF);
        header[7] = (uint8_t)((system_bytes >> 16) & 0xFF);
        header[8] = ((uint8_t)(system_bytes >> 8) & 0xFF);
        header[9] = (uint8_t)((system_bytes) & 0xFF);

        // TODO also consider doing a union with block, header, data
        memcpy(block, header, 10); 
        memcpy(&block[10], data, data_size); 
        block_size = (uint8_t)(data_size + 10);

        // FIXME may be too late to check for buffer problems here...maybe before the memcpy
        if (block_size > sizeof(block))
        {
            FATAL ("Block size <%d> too big, limited to <%d>\n", block_size, sizeof(block));
        }

        /* Repeat sending this secs block <retry> times */

        retry = (uint32_t)atoi(get_option("retry=", name)) + 1;
        while (retry)
        {
            iret = send_secsI(name, fd, block, block_size);
            DEBUG (3, DEBUG_HDR, "Send_secsI returning <%d>\n", iret);
            if (iret == 3)
            {  /* Bad error, don't retry */
                break;
            }
            else if (iret == 2)
            {  /* Contention detected during send */
                DEBUG (3, DEBUG_HDR, "Handling contention\n");
                saved_idx = get_program_token_idx();
                tmp_variable_delete_all();
                iret = recv_secs(name, fd, 1, &program_idx, &primary_msg);
                if (iret)
                {
                	;
                }
                else
                {
                    if (primary_msg)
                    {
                        /* Primany message */
                        update_fd_info(name, fd);  /* Set this for the send_reply() */
                        /* make_tmp_variables(bufdata(&reply), "", 1); */
                        tmp_variable_sort();
                        iret = resume_via_program_label(name);

                    }
                    else
                    {
                        /* Secondary reply message */
                        /* make_tmp_variables(bufdata(&reply), "", 1); */
                        tmp_variable_sort();
                        iret = resume_via_program_idx(program_idx);
                    }

                    if (iret)
                    {
                        ERROR ("Error processing received secs message from <%s>\n",
                            name);
                    }
                }
                DEBUG (3, DEBUG_HDR, "Handling contention, continuing with orginal send\n");
                retry = (uint32_t)atoi(get_option("retry=", name)) + 1;
                iret = 0;
                set_program_token_idx(saved_idx);
                continue;
            }
            else if (iret)
            {  /* Protocol error, so possible retry */
                /* Don't flag this as an error
                ERROR ("Error sending secsI message to <%s>\n", name); */
                retry--;
                if (retry)
                {
                    continue;

                }
                else
                {
                    ERROR ("Retry limit reached with <%s>\n", name);
                    iret = 1;
                }
                break;
            } 
            break;
        }

        if (iret)
        {  /* Error from above */
            break;
        }
        block_num++;

        if (msg_size)
        {
            continue;
        }
        else
        {
            break;
        }
    }

    if (iret)
    {  /* Error from above */
    	;
    }
    else if (wait_bit)
    {
        DEBUG (3, DEBUG_HDR, "Adding entry to open transaction table\n");
        t3 = (uint32_t)atoi(get_option("t3=", name));
        sprintf(comment, "T3 timeout waiting for <S%uF%u> reply at line <%s>",
            stream, function, get_line_number());
        timer_idx = start_timer(-1, t3, get_program_token_idx(), 
            get_option("timeout_msg=", name), comment);  
        /* timer_idx = start_timer(-1, t3, error_idx, comment); */ 
        open_trans_table_add(get_program_token_idx(), system_bytes, timer_idx, t3);
        *wait_flag = true;

    }
    else
    {
        *wait_flag = false;
    }

    binary_buffree(&encoded_secs_msg);
    buffree(&log);
    DEBUG (3, DEBUG_HDR, "Returning <%d>\n", iret);
    return iret;
}

/********************************************/

static uint32_t send_secsI(const char *name, int fd, uint8_t *block, uint8_t block_size)
{
	int i = 0;
	uint32_t t2 = 0;
	uint32_t rbit = 0;
	uint32_t iret = 0;
	uint8_t c = 0;
	uint8_t cs[2];
//static sigjmp_buf timer_t2;
	static jmp_buf timer_t2;

#undef NAME
#define NAME "send_secsI()"

    DEBUG (3, DEBUG_HDR, "Called, fd <%d>, block size <%d>\n", fd, block_size);

    t2 = (uint32_t)atoi(get_option("t2=", name));
    rbit = (uint32_t)atoi(get_option("rbit=", name));

    DEBUG (3, DEBUG_HDR, "Send ENQ\n");
    FD_LOG3(name) ("Send ENQ to <%s>\n", name);

    iret = my_writec(name, fd, ENQ);
    if (iret)
    {
        ERROR ("Error while sending ENQ to <%s>\n", name);
        return 3;
    }

//    if (sigsetjmp(timer_t2, 1) != 0)
    if (do_the_setsigjump(timer_t2, 1) != 0)
    {
        ERROR ("T2 Timeout detected while waiting for EOT from <%s>\n", name);
        return 1;
    }
    start_proto_timer(t2, 2, &timer_t2);

    while (1)
    {
        DEBUG (3, DEBUG_HDR, "Waiting for EOT...\n");
        iret = my_readc(name, fd, &c); 
        if (iret)
        {
            stop_proto_timer(2);
            ERROR ("Error while reading EOT from <%s>\n", name);
            return 3;
        }
    
        if (c == EOT)
        {
            stop_proto_timer(2);
            FD_LOG3(name) ("Recv EOT from <%s>\n", name);
            DEBUG (3, DEBUG_HDR, "Received EOT\n");
            break;
    
        }
        else if (c == ENQ)
        {  /* Possible contention */
            FD_LOG3(name) ("Recv ENQ from <%s>\n", name);
            DEBUG (3, DEBUG_HDR, "Received ENQ\n");

            if (rbit)
            {
                ERROR ("Expecting EOT, received <%.2Xh> from <%s>\n", (unsigned char) c, name);
                continue;
             }
            else
            {
                stop_proto_timer(2);
                FD_LOG2(name) ("SECSI contention detected from <%s>\n", name);
                return 2;
            }
        }
        else
        {
            ERROR ("Expecting EOT, received <%.2Xh> from <%s>\n", (unsigned char) c, name);
            continue;
        }
    }  /* End of while */

    secs_calc_checksum(block, block_size, cs);

    FD_LOG3(name) ("Sending Block data to <%s>:\n", name);
    if (get_logging(name) >= 3)
    {
        LOG_DATA(name) ("    Size     ",  &block_size, 1);
        LOG_DATA(name) ("    Header   ", &block[0], 10);
        LOG_DATA(name) ("    body     ", &block[10], (uint32_t)(block_size-10));
        LOG_DATA(name) ("    Checksum ", cs, 2);
    }

    /* Send the block size */
    iret = my_writec(name, fd, block_size);
    if (iret)
    {
        ERROR ("Error while sending block size to <%s>\n", name);
        return 3;
    }

    /* Send the block */
    // TODO why are we writing out bytes individually?
    for (i=0; i<block_size; i++)
    {
        iret = my_writec(name, fd, block[i]); 
        if (iret)
        {
            ERROR ("Error while sending data message to <%s>\n", name);
            return 3;
        }
    }

    /* Send the checksum */
    for (i=0; i<2; i++)
    {
        iret = my_writec(name, fd, cs[i]);
        if (iret)
        {
            ERROR ("Error while sending checksum to <%s>\n", name);
            return 3;
        }
    }

//    if (sigsetjmp(timer_t2, 1) != 0)
    if (do_the_setsigjump(timer_t2, 1) != 0)
    {
        ERROR ("T2 Timeout detected while waiting for ACK from <%s>\n", name);
        return 1;
    }
    start_proto_timer(t2, 2, &timer_t2);

    DEBUG (3, DEBUG_HDR, "Waiting for ACK...\n");
    iret = my_readc(name, fd, &c);  /* t2 */
    if (iret)
    {
        stop_proto_timer(2);
        ERROR ("Error while reading ACK <%s>\n", name);
        return 3;
    }
    stop_proto_timer(2);

    if (c == ACK)
    {
        FD_LOG3(name) ("Recv ACK from <%s>\n", name);
        DEBUG (3, DEBUG_HDR, "Received ACK\n");
    }
    else if (c == NAK)
    {
        FD_LOG3(name) ("Recv NAK from <%s>\n", name);
        return 1;
    }
    else
    {
        ERROR ("Expecting ACK/NAK, received <%.2Xh> from <%s>\n", (unsigned char) c, name);
        return 1;
    }

    return 0;
}

/********************************************/

