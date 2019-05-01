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

#include "include.h"

/********************************************/

uint32_t send_standard(const char *name, int fd)
{
	uint32_t iret = 0;
	char *option = (char *)NULL;
	char *fmt = NULL;
	struct buffer token = INIT_BUFFER;
	struct buffer reply = INIT_BUFFER;
	struct buffer data = INIT_BUFFER;

#undef NAME
#define NAME "send_standard()"

    DEBUG (3, DEBUG_HDR, "Called name <%s>, fd <%d>\n", name, fd);

    iret = expression(&token);
    if (iret)
    {
        ERROR ("Error with expression in SEND statement\n");

    }
    else
    {
        bufcat(&token, get_option("xmt_eol=", name));
        if (get_logging(name) == 1)
        {
            fmt = "Send msg to <%s>, <%.20s...>\n";
        }
        else
        {
            fmt = "Send msg to <%s>, <%s>\n";
        }
        FD_LOG1(name) (fmt, name, bufdata(&token));
        bufcpy(&data, "");
        char_to_data(&data, bufdata(&token));  /* Convert chars to data */
        if (my_write(name, fd, (const uint8_t *)bufdata(&data), bufsize(&data)))
        {
            ERROR ("Error sending reply to <%s>, <%s>\n", name, bufdata(&token));
            iret = 1;
        }
    }

    if (iret)
    {
    	;
    }
    else
    {
        /* See if a reply is expected */
        option = get_option("wait_for_reply=", name);
        if (strcmp(option, "no") == 0)
        {
            /* No reply is expected */
        	;
        }
        else
        {
            bufcpy(&reply, "");
            iret = recv_standard(name, fd, &reply);
            if (get_logging(name) == 1)
            {
                fmt = "Recv reply, <%.20s...>\n";
            }
            else
            {
                fmt = "Recv reply, <%s>\n";
            }
            FD_LOG1(name) (fmt, bufdata(&reply));
            /* DEBUG (1, DEBUG_HDR, "Received reply iret <%d>, <%s>\n", iret, bufdata(&reply)); */
            if (iret)
            {
            	;
            }
            else
            {
                make_tmp_variables(bufdata(&reply), get_option("no_parse=", name), 1);
            }
        }
    }

    buffree(&token);
    buffree(&reply);
    buffree(&data);
    DEBUG (3, DEBUG_HDR, "Returning <%d>\n", iret);
    return iret;
}

/********************************************/

uint32_t send_standard_reply(const char *name, int fd)
{
	uint32_t iret = 0;
	char *fmt = (char *)NULL;
	struct buffer token = INIT_BUFFER;
	struct buffer data = INIT_BUFFER;

#undef NAME
#define NAME "send_standard_reply()"

    DEBUG (3, DEBUG_HDR, "Called name <%s>, fd <%d>\n", name, fd);

    iret = expression(&token);
    if (iret)
    {
        ERROR ("Error with expression in SEND_REPLY statement\n");

    }
    else
    {
        bufcat(&token, get_option("xmt_eol=", name));
        if (get_logging(name) == 1)
        {
            fmt = "Send reply to <%s>, <%.20s...>\n";
        }
        else
        {
            fmt = "Send reply to <%s>, <%s>\n";
        }
        FD_LOG1(name) (fmt, name, bufdata(&token));
        bufcpy(&data, "");
        char_to_data(&data, bufdata(&token));  /* Convert chars to data */
        if (my_write(name, fd, (const uint8_t *)bufdata(&data), bufsize(&data)))
        {
            ERROR ("Error sending reply to <%s>, <%s>\n", name, bufdata(&token));
            iret = 1;
        }
    }

    buffree(&token);
    buffree(&data);
    DEBUG (3, DEBUG_HDR, "Returning <%d>\n", iret);
    return iret;
}

/********************************************/

uint32_t recv_standard(const char *name, int fd, struct buffer *reply)
{
	uint32_t iret = 0;
	uint32_t timeout = 0;
	char *rcv_eol = (char *)NULL;
	char *option = (char *)NULL;
	uint8_t byte = 0;
	char c = 0;
	static jmp_buf timer;

#undef NAME
#define NAME "recv_standard()"

    DEBUG (3, DEBUG_HDR, "Called, name <%s>, fd <%d>\n", name, fd);

    bufcpy(reply, "");

    option = get_option("timeout=", name);
    if (strlen(option))
    {
        timeout = (uint32_t)atoi(option);
        DEBUG (3, DEBUG_HDR, "Timeout specififed <%d>\n", timeout);
    }
    else
    {
        timeout = 60;  /* Default timeout, avoids waiting forever */
        DEBUG (3, DEBUG_HDR, "Timeout not specified, using default value <%d>\n", timeout);
    }

    DEBUG (3, DEBUG_HDR, "Waiting for message...\n");

//    if (sigsetjmp(timer, 1) != 0)
    if (do_the_setsigjump(timer, 1) != 0)
    {
        ERROR ("Timeout detected receiving <%s> from <%s>\n", bufdata(reply), name);
        /* bufcpy(reply, get_option("timeout_msg=", fd)); */
        /* Include any part of the message that was received */
        bufprefix(reply, " ");
        bufprefix(reply, get_option("timeout_msg=", name));
        return 0;
    }

    start_proto_timer(timeout, 0, &timer);

    rcv_eol = get_option("rcv_eol=", name);
    while (1) {
        DEBUG (3, DEBUG_HDR, "Reading...\n");
        iret = my_readc(name, fd, &byte);  /* Read 1 char at a time */
        if (iret)
        {
            break;
        }
        else
        {
            /* Convert data char(s) to regular char(s) and append to reply */
        	c = (char)byte;
            data_to_char(reply, &c, 1);  

            DEBUG (3, DEBUG_HDR, "Read <%s>\n", bufdata(reply));
            if (strlen(rcv_eol)) {
                if (strstr(bufdata(reply), rcv_eol) == '\0')
                {
                    continue;  /* Not found yet */
                }
                else
                {
                    break;
                }
            }
            else
            {
                break;
            }
        }
    }

    stop_proto_timer(0);

    DEBUG (3, DEBUG_HDR, "Returning <%d>, <%s>\n", iret, bufdata(reply));
    return iret;
}

/********************************************/


