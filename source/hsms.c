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

#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <setjmp.h>
#include <stdint.h>

#include "include.h"


static uint32_t client_connect(const char *, int);

/********************************************/

uint32_t hsms_client_connect(const char *name, int fd)
{
	uint32_t iret = 0;
	uint32_t t5 = 0;

#undef NAME
#define NAME "hsms_client_connect()"

    iret = client_connect(name, fd);
    if (iret)
    {
        t5 = (uint32_t)atoi(get_option("t5=", name));
        if (t5 == 0)
        {
            t5 = 10;  /* This will keep things from looping to fast if ommitted */
        }

        FD_LOG1(name) ("Waiting for T5 <%d> seconds...\n", t5);
        sleep (t5);
    } 

    DEBUG (3, DEBUG_HDR, "Returning iret <%d>\n", iret);
    return iret;
}

/********************************************/

static uint32_t client_connect(const char *name, int fd)
{
	uint8_t header[10];
	uint8_t stype = 0;
	uint32_t t6 = 0;
	uint32_t iret = 0;
	uint32_t system_bytes = 0;
//	static sigjmp_buf timer_t6;
#ifdef __CYGWIN__
	static sigjmp_buf timer_t6;
#else
	static jmp_buf timer_t6;
#endif
/* time_t t; */
	struct binary_buffer body = INIT_BINARY_BUFFER;

#undef NAME
#define NAME "client_connect()"

    DEBUG (3, DEBUG_HDR, "Called, fd <%d>\n", fd);

    if (strcmp(get_option("hsms_logging=", name), "off") == 0)
    {
        set_logging(name, 2);
    }

    if (strcmp(get_option("hsms_logging=", name), "on") == 0)
    {
        set_logging(name, 3);
    }

    FD_LOG1(name) ("Sending \"Select request\" to hsms server <%s>...\n", name);

    memset(header, '\0', sizeof(header));
    header[0] = 0xFF;
    header[1] = 0xFF; 
    header[5] = 0x01;  /* Stype */
    system_bytes = secs_inc_system_bytes();

    memcpy(&header[6], &system_bytes, sizeof(system_bytes));
    swap(&header[6], sizeof(system_bytes));

    iret = send_hsmsI(name, fd, header, (uint8_t *)"", (uint32_t)0);

    if (iret)
    {
        ERROR ("Can't connect to hsms server <%s>\n", name);

    }
    else
    {
        while (1)
        {  /* Do only once */
            t6 = (uint32_t)atoi(get_option("t6=", name));
//            if (sigsetjmp(timer_t6, 1) != 0)
            if (do_the_setsigjump(timer_t6, 1) != 0)
            {
                stop_proto_timer(6);
                ERROR ("T6 Timeout detected while waiting for \"Select Response\" from <%s>\n", 
                    name);
                break;
            }

            start_proto_timer(t6, 6, &timer_t6);
            iret = recv_hsmsI(name, fd, header, &body); 
            if (iret)
            {
                ERROR ("Error receiving HSMS \"Select Response\" from <%s>\n", name);     

            }
            else
            {
                stype = (unsigned char) header[5];
                if (stype == 2)
                {
                    FD_LOG1(name) ("Recv HSMS \"Select Response\" from <%s>\n", name);
            
                }
                else
                {
                    ERROR ("Expecting \"Select Response\", received stype <%d> from <%s>\n", 
                        stype, name);     
                }
            }
            stop_proto_timer(6);
            break;
        }
    }

    binary_buffree(&body);
    DEBUG (3, DEBUG_HDR, "Returning iret <%d>\n", iret);
    return iret;
}

/********************************************/


