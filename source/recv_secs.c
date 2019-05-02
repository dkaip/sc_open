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
 * Functions in this file are used in the case of SECSI communication
 */

//static int recv_secsI(char *, int, uint8_t *, uint8_t *, bool);
static uint32_t recv_secsI(const char *name, int fd, uint8_t *block, uint8_t *block_size, bool flag);
static void completion(const char *, int, uint32_t);

/********************************************/

uint32_t recv_secs(const char *name, int fd, bool flag, uint32_t *program_idx, bool *primary_msg_flag)
{
	uint32_t i = 0;
	bool duplicate_block = false;
	uint32_t iret = 0;
	uint32_t /*t3,*/ t4, first, arrays;
	uint32_t system_bytes;
	char num[64];
	char sbuf[128];
	uint8_t block_size = 0;
	char var_prefix[32];
	uint8_t header[10] = {0};
	uint8_t last_header[10] = {0};
	uint8_t block[254] = {0};
#ifdef __CYGWIN__
  static sigjmp_buf /*timer_t3, */timer_t4;
#else
  static jmp_buf /*timer_t3, */timer_t4;
#endif
struct binary_buffer encoded_secs_msg = INIT_BINARY_BUFFER;
struct buffer log = INIT_BUFFER;
struct buffer tmp_name = INIT_BUFFER;
#undef NAME
#define NAME "recv_secs()"

	num[0] = 0;
	sbuf[0] = 0;
	var_prefix[0] = 0;
    DEBUG (3, DEBUG_HDR, "Called, name <%s>, fd <%d>, flag <%d>\n", name, fd, flag);

    if (strcmp(get_option("secsI_logging=", name), "off") == 0)
    {
        set_logging(name, 2);
    }
    else  if (strcmp(get_option("secsI_logging=", name), "on") == 0)
    {
        set_logging(name, 3);
    }

    arrays = 1;
    if (strcmp(get_option("arrays=", name), "no") == 0)
    {
        arrays = 0;
    }

    /* 
    * flag values
    * false = Receive a primary message or secondary message
    * true = Receive a primary message, but the first ENQ has already been received
    *     so don't wait for it (do only once).
    */

    first = 1;
    binary_buf_empty(&encoded_secs_msg);
//    t3 = atoi(get_option("t3=", name));
    t4 = (uint32_t)atoi(get_option("t4=", name));

    /* Get all the secs I level blocks */
    while (1)
    {
        if (iret)
        {
            break;  /* Incase of t3 timeout above */
        }

        iret = recv_secsI(name, fd, block, &block_size, flag);
        if (flag == true)
        {  /* If set, should only be set for the first time */
            flag = false;
        }

        if (iret == 1)
        {  /* Non specific error */
            ERROR ("Error receiving secsI message from <%s>\n", name);
            break;

        }
        else if (iret == 2)
        {  /* MEssage was NAK-ed */
            break;
        }

        stop_proto_timer(3);
        stop_proto_timer(4); 

        duplicate_block = false;
        if (first)
        {
            first = 0;
            memcpy(header, block, 10);
        }
        else
        {
            /* Check for duplicate block */
            for (i=0; i<10; i++)
            {
                if (last_header[i] == block[i])
                {
                	;
                }
                else
                {
                    break;
                }
            }

            if (i == 10)
            {
                ERROR ("Duplicate block detected, discarding block\n");
                duplicate_block = true;
            }
            else
            {
                /* Save the header */
                for (i=0; i<10; i++)
                {
                    last_header[i] = block[i];  
                }
            }
        }
      
        if (duplicate_block)
        {
        	;
        }
        else
        {
            binary_bufncat(&encoded_secs_msg, &block[10], (uint32_t)(block_size-10));
        }

        if (block[4] & 0x80)
        {
            break;  /* Last message block, so quit */
        }

        /* Start the t4 timer and get the next block */
//        if (sigsetjmp(timer_t4, 1) != 0)
        if (do_the_setsigjump(timer_t4, 1) != 0)
        {
            ERROR ("T4 Timeout detected while waiting for block from <%s>\n", name);
            iret = 1;
            break;
        }
        start_proto_timer(t4, (uint32_t)4, &timer_t4);
    }
    stop_proto_timer(3);
    stop_proto_timer(4); 

    if (iret)
    {
    	;
    }
    else
    {
        if (g_debug >= 5) binary_bufdump_hex(&encoded_secs_msg);

        sprintf(var_prefix, "S%dF%d", header[2]&0x7F, header[3]);
        sprintf(num, "%d", (((unsigned char) header[0] & 0x7F) * 256) + (unsigned char) header[1]); 
        bufcpy(&log, "");
        bufcat(&log, var_prefix);
        bufcat(&log, " DEVICE_ID=");
        bufcat(&log, num);

        /* Tmp variable SXFY=sxfy */
        bufcpy(&tmp_name, "SXFY");
        tmp_variable_add(bufdata(&tmp_name), var_prefix, -1);

        /* Tmp variable sxfy.DEVICE_ID=x */
        bufcpy(&tmp_name, var_prefix);
        bufcat(&tmp_name, ".DEVICE_ID");
        tmp_variable_add(bufdata(&tmp_name), num, -1);

        /* Tmp variable sxfy.WAIT_BIT=1 or 0 */
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

        system_bytes = (uint32_t)((header[6] * 16777216) +
            (header [7] * 65536) +
            (header [8] * 256) +
            header [9]);

        if (arrays)
        {
            iret = secs_decode(var_prefix, &encoded_secs_msg, &log);

        }
        else
        {
            iret = secs_decode_no_arrays(var_prefix, &encoded_secs_msg, &log);
        }

        /* See if we got a primary or secondary reply message.  Check if y in SxFy 
           is even (primary) or odd (secondary) */

        /* Fill in the logging buffer */
        sprintf(sbuf, "s%df%d", header[2] & 0x7F, header[3]);
        /* sprintf(sbuf, "s%df%d, device_id <%d>, wait_bit <%d>", 
            (unsigned char) header[2] & 0x7F,
            (unsigned char) header[3],
            (((unsigned char) header[0] & 0x7F) * 256) + (unsigned char) header[1],
            (header[2] & 0x80) ? 1 : 0); */

        if ((unsigned char) header[3] % 2)
        {
            /* Odd function number, so must be a primary message */
            *primary_msg_flag = true;
            FD_LOG1(name) ("Recv primary SECS msg from <%s>, <%s>\n", name, sbuf);
            FD_LOG2(name) ("%s", bufmkprint(&log));
            secs_set_system_bytes(system_bytes);
        }
        else
        {
            *primary_msg_flag = false;
            FD_LOG1(name) ("Recv Secondary SECS reply msg from <%s>, <%s>\n", name, sbuf);
            FD_LOG2(name) ("%s", bufmkprint(&log));
           
            /* Get the program idx (where we left of in the program) */
            iret = open_trans_table_del(system_bytes, program_idx);
            if (iret)
            {
                /* Received a message that was not in the open_transaction_table */
                ERROR ("Discarding secs message\n");
                iret = 2;  /* ret 2 means a protocol error, not really bad */
            }
        }
    }

    binary_buffree(&encoded_secs_msg);
    buffree(&log);
    buffree(&tmp_name);
    DEBUG (3, DEBUG_HDR, "Returning iret <%d>, program_idx\n", iret, *program_idx);
    return iret;
}

/********************************************/

static uint32_t recv_secsI(const char *name, int fd, uint8_t *block, uint8_t *block_size, bool flag)
{
	uint32_t t1 = 0;
	uint32_t t2 = 0;
	uint8_t checksum[2];
	uint8_t size = 0;;
	uint8_t c = 0;
	uint8_t cs[2];
#ifdef __CYGWIN__
  static sigjmp_buf timer_t1, timer_t2;
#else
  static jmp_buf timer_t1, timer_t2;
#endif

#undef NAME
#define NAME "recv_secsI()"

    DEBUG (3, DEBUG_HDR, "Called, fd <%d>\n", fd);
    t1 = (uint32_t)atoi(get_option("t1=", name));
    t2 = (uint32_t)atoi(get_option("t2=", name));

    if (flag == true)
    {  /* Receive primary, but the ENQ has already been received */
    	;
    }
    else
    {
        if (my_readc(name, fd, &c))
        {
            ERROR ("Error while reading ENQ from <%s>\n", name);
            return 1;
        } 

        if (c == ENQ)
        {
            DEBUG (3, DEBUG_HDR, "Received ENQ\n");
            FD_LOG3(name) ("Recv ENQ from <%s>\n", name);
        }
        else
        {
            FD_LOG3(name) ("Recv <%.2X> from <%s>, expecting ENQ, discarding\n", 
                (unsigned char) c, name);
            /* ERROR ("Expecting ENQ, received <%.2X> from <%s>\n", 
                (unsigned char) c, name); */
            return 2;
        }
    }

    FD_LOG3(name) ("Send EOT to <%s>\n", name);
    if (my_writec(name, fd, EOT))
    {
        ERROR ("Error while sending EOT to <%s>\n", name);
        return 1;
    }

    while (1)
    {
//        if (sigsetjmp(timer_t2, 1) != 0)
        if (do_the_setsigjump(timer_t2, 1) != 0)
        {
            ERROR ("T2 Timeout detected while waiting for block size byte from <%s>\n", name);
            FD_LOG3(name) ("Send NAK to <%s>\n", name);
            if (my_writec(name, fd, NAK))
            {
                ERROR ("Error while sending NAK to <%s>\n", name);
                return 1;
            }
            return 2;
        }
        start_proto_timer(t2, 2, &timer_t2);

        DEBUG (3, DEBUG_HDR, "Reading block size byte...\n");
        if (my_readc(name, fd, &c))
        {
            stop_proto_timer(2);
            ERROR ("Error while reading block size from <%s>\n", name);
            return 1;
        } 
        stop_proto_timer(2);

        if (c == ENQ)
        {
            FD_LOG3(name) ("Ignoring ENQ from <%s>\n", name);
            continue;
        }
        break;
    }

    size = (unsigned char) c;
    DEBUG (3, DEBUG_HDR, "Block size byte is <%d>\n", size);

    if ((size < 10) || (size > 254))
    {
        ERROR ("Invalid block size <%d> from <%s>\n", size, name);
        completion(name, fd, t1);
        FD_LOG3(name) ("Send NAK to <%s>\n", name);
        if (my_writec(name, fd, NAK))
        {
            ERROR ("While sending NAK to <%s>\n", name);
            return 1;
        }
        return 2;
    }     

    DEBUG (3, DEBUG_HDR, "Reading block...\n");
    *block_size = 0;
    for (uint32_t i=0; i<size; i++)
    {
//        if (sigsetjmp(timer_t1, 1) != 0)
        if (do_the_setsigjump(timer_t1, 1) != 0)
        {
            ERROR ("T1 Timeout while reading block from <%s>\n", name);
            FD_LOG3(name) ("Send NAK to <%s>\n", name);
            if (my_writec(name, fd, NAK)) {
                ERROR ("Error while sending NAK to <%s>\n", name);
                return 1;
            }
            return 2;
        }
        start_proto_timer(t1, 1, &timer_t1);

        if (my_readc(name, fd, &c)) {  /* t1 */ 
            stop_proto_timer(1);      
            ERROR ("Error while reading block from <%s>\n", name);
            return 1;
        } 
        stop_proto_timer(1);      

        block[i] = (unsigned char) c;
        (*block_size)++;
    }

    DEBUG (3, DEBUG_HDR, "Reading checksum...\n");
    // TODO  should I really loop for 2 items?
    for (uint32_t i=0; i<2; i++)
    {
//        if (sigsetjmp(timer_t1, 1) != 0)
        if (do_the_setsigjump(timer_t1, 1) != 0)
        {
            ERROR ("T1 Timeout while reading checksum from <%s>\n", name);
            FD_LOG3(name) ("Send NAK to <%s>\n", name);
            completion(name, fd, t1);
            if (my_writec(name, fd, NAK))
            {
                ERROR ("While sending NAK to <%s>\n", name);
                return 1;
            }
            return 2;
        }
        start_proto_timer(t1, 1, &timer_t1);

        if (my_readc(name, fd, &cs[i]))
        {  /* t1 */
            stop_proto_timer(1);      
            ERROR ("Error while reading checksum from <%s>\n", name);
            return 1;
        }
        stop_proto_timer(1);      
    } 

    DEBUG (3, DEBUG_HDR, "Received checksum is <%.2Xh> <%.2Xh>\n", 
        (unsigned char) cs[0], (unsigned char) cs[1]);

    secs_calc_checksum(block, *block_size, checksum);
    DEBUG (3, DEBUG_HDR, "Calc checksum is <%.2Xh> <%.2Xh>\n",
        (unsigned char) checksum[0], (unsigned char) checksum[1]);

    FD_LOG3(name) ("Recv Block data from <%s>:\n", name);
    if (get_logging(name) >= 3)
    {
        LOG_DATA(name) ("    Size     ", &size, 1);
        LOG_DATA(name) ("    Header   ", block, 10);
        LOG_DATA(name) ("    body     ", &block[10], (uint32_t)(size-10));
        LOG_DATA(name) ("    Checksum ", cs, 2); 
    }

    DEBUG (3, DEBUG_HDR, "Comparing checksums\n");
    if (((unsigned char) cs[0] == checksum[0]) && 
        ((unsigned char) cs[1] == checksum[1]))
    {
    	;
    }
    else
    {
        ERROR ("Checksum from <%s> check failed\n", name);
        completion(name, fd, t1);
        FD_LOG3(name) ("Send NAK to <%s>\n", name);
        if (my_writec(name, fd, NAK))
        {
            ERROR ("While sending NAK to<%s>\n", name);
            return 1;
        }
        return 2; 
    }

    DEBUG (3, DEBUG_HDR, "Sending ACK\n");
    FD_LOG3(name) ("Send ACK to <%s>\n", name);
    if (my_writec(name, fd, ACK))
    {
        ERROR ("While sending ACK to <%s>\n", name);
        return 1;
    }

    DEBUG (3, DEBUG_HDR, "Returning success\n");
    return 0;
}

/********************************************/

static void completion(const char *name, int fd, uint32_t t1)
{
/*int i;*/
	uint8_t c;
#ifdef __CYGWIN__
  static sigjmp_buf timer_t1;
#else
  static jmp_buf timer_t1;
#endif

#undef NAME
#define NAME "completion()"

    DEBUG (3, DEBUG_HDR, "Called, fd <%d>, t1 <%d>\n", fd, t1);

    while (1)
    {
//        if (sigsetjmp(timer_t1, 1) != 0)
        if (do_the_setsigjump(timer_t1, 1) != 0)
        {
            return;
        }
        start_proto_timer(t1, 1, &timer_t1);

        if (my_readc(name, fd, &c))
        {
            stop_proto_timer(1);      
            return;
        } 
        stop_proto_timer(1);      
        ERROR ("Discarding char <%.2X> from <%s>\n", (unsigned char) c, name);
    }

    return; 
}

#ifdef __CYGWIN__
int do_the_setsigjump(sigjmp_buf the_jump_buffer, int save_mask)
{
	return sigsetjmp(the_jump_buffer, save_mask);
}
#else
int do_the_setsigjump(jmp_buf the_jump_buffer, int save_mask)
{
	return sigsetjmp(the_jump_buffer, save_mask);
}
#endif
