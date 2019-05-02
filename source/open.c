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

#include <termios.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <termios.h>

#include "include.h"

extern int open_socket_client_local(char *name);

static void flush_open(const char *);

/********************************************/

uint32_t cmd_open()
{
	uint32_t iret = 1;
	int fd = 0;
	bool pipe = false;
	bool port = false;
	bool socket_client = false;
	bool socket_server = false;
	bool mbx = false;
	bool mbx_client = false;
	bool mbx_server = false;
	enum token_types type = TT_NOT_ASSIGNED_YET;
	enum protos proto = PROTO_UNKNOWN;
	struct buffer token = INIT_BUFFER;
	char name[128];
	char local_name[128];
	char device[128];
	char options[128];
	char host[128];
	char service[128];
	int logging;
	FILE *logfile_fp = (FILE *)NULL;

#undef NAME
#define NAME "cmd_open()"

    DEBUG (3, DEBUG_HDR, "Called\n");

    bufcpy(&token, "");
    /*
     * I'n not worried about optimizing the initing of the string fields
     * because this function is unlikely to be called in a loop.
     */
    strcpy(name, "");
    strcpy(local_name, "");
    strcpy(device, "");
    strcpy(options, "");
    strcpy(host, "");
    strcpy(service, "");
    logging = 2;  /* Default value, 0 means turn off */ 
    logfile_fp = NULL;

    pipe = port = socket_client = socket_server = 0;
    mbx = mbx_client = mbx_server = 0;
    iret = 0;

    while (1)
    {
        get_program_token(&token, &type);    

        /* printf("????? token <%s> type <%s>\n", bufdata(&token), show_type(type)); */

        DEBUG (3, DEBUG_HDR, "Token <%s>, type <%s>\n", bufdata(&token), show_type(type));
        if ((type == TT_END_OF_LINE) || 
            (type == TT_ERROR) || 
            (type == TT_NO_ERROR) || 
            (type == TT_NULL_TOKEN))
        {
            put_program_token(); 
            break;
        }
        else if (bufcmp(&token, "name") == 0)
        {
            get_program_token(&token, &type);    
            if (type == TT_ASSIGN)
            {
                get_program_token(&token, &type);    
                if (strlen(bufdata(&token)) >= sizeof(name))
                {
                    ERROR ("Name <%s> is too long, limited to <%d>\n", sizeof(name));
                    iret = 1;
                    break;
                }
                else
                {
                    strcpy(name, bufdata(&token));
                }
            }
            else
            {
                put_program_token(); 
                ERROR ("Expecting \"=\", found <%s>\n", bufdata(&token));
                iret = 1; 
                break;
            }
        }
        else if (bufcmp(&token, "local_name") == 0)
        {
            get_program_token(&token, &type);    
            if (type == TT_ASSIGN)
            {
                get_program_token(&token, &type);    
                if (strlen(bufdata(&token)) >= sizeof(local_name))
                {
                    ERROR ("Name <%s> is too long, limited to <%d>\n", sizeof(local_name));
                    iret = 1;
                    break;
                }
                else
                {
                    strcpy(local_name, bufdata(&token));
                }
            }
            else
            {
                put_program_token(); 
                ERROR ("Expecting \"=\", found <%s>\n", bufdata(&token));
                iret = 1; 
                break;
            }
        }
        else if (bufcmp(&token, "device") == 0)
        {
            get_program_token(&token, &type);    
            if (type == TT_ASSIGN)
            {
                get_program_token(&token, &type);    
                if (strlen(bufdata(&token)) >= sizeof(device))
                {
                    ERROR ("Device <%s> is too long, limited to <%d>\n", sizeof(device));
                    iret = 1;
                    break;
                }
                else
                {
                    strcpy(device, bufdata(&token));
                }
            }
            else
            {
                put_program_token(); 
                ERROR ("Expecting \"=\", found <%s>\n", bufdata(&token));
                iret = 1; 
                break;
            }
        }
        else if (bufcmp(&token, "proto") == 0)
        {
            get_program_token(&token, &type);    
            if (type == TT_ASSIGN)
            {
                get_program_token(&token, &type);    
                proto = lookup_proto(bufdata(&token));
            }
            else
            {
                put_program_token(); 
                ERROR ("Expecting \"=\", found <%s>\n", bufdata(&token));
                iret = 1; 
                break;
            }
        }
        else if (bufcmp(&token, "host") == 0)
        {
            get_program_token(&token, &type);    
            if (type == TT_ASSIGN) {
                get_program_token(&token, &type);    
                if (strlen(bufdata(&token)) >= sizeof(host))
                {
                    ERROR ("Host <%s> is too long, limited to <%d>\n", sizeof(host));
                    iret = 1;
                    break;
                }
                else
                {
                    strcpy(host, bufdata(&token));
                }
            }
            else
            {
                put_program_token(); 
                ERROR ("Expecting \"=\", found <%s>\n", bufdata(&token));
                iret = 1; 
                break;
            }
        }
        else if (bufcmp(&token, "service") == 0)
        {
            get_program_token(&token, &type);    
            if (type == TT_ASSIGN)
            {
                get_program_token(&token, &type);    
                if (strlen(bufdata(&token)) >= sizeof(service))
                {
                    ERROR ("Service <%s> is too long, limited to <%d>\n", sizeof(service));
                    iret = 1;
                    break;
                }
                else
                {
                    strcpy(service, bufdata(&token));
                }
            }
            else
            {
                put_program_token(); 
                ERROR ("Expecting \"=\", found <%s>\n", bufdata(&token));
                iret = 1; 
                break;
            }
        }
        else if (bufcmp(&token, "options") == 0)
        {
            get_program_token(&token, &type);    
            if (type == TT_ASSIGN)
            {
                get_program_token(&token, &type);    
                if (strlen(bufdata(&token)) >= sizeof(options))
                {
                    ERROR ("Options <%s> is too long, limited to <%d>\n", sizeof(options));
                    iret = 1;
                    break;

                }
                else
                {
                    strcpy(options, bufdata(&token));
                }
            }
            else
            {
                put_program_token(); 
                ERROR ("Expecting \"=\", found <%s>\n", bufdata(&token));
                iret = 1; 
                break;
            }
        }
        else if (bufcmp(&token, "logging") == 0)
        {
            get_program_token(&token, &type);    
            if (type == TT_ASSIGN)
            {
                get_program_token(&token, &type);    
                logging = atoi(bufdata(&token)); 
            }
            else
            {
                put_program_token(); 
                ERROR ("Expecting \"=\", found <%s>\n", bufdata(&token));
                iret = 1; 
                break;
            }
        }
        else if (bufcmp(&token, "logfile") == 0)
        {
            get_program_token(&token, &type);    
            if (type == TT_ASSIGN)
            {
                get_program_token(&token, &type);    
                if ((logfile_fp = fopen(bufdata(&token), "a")) == NULL)
                {
                    ERROR ("Can't open log file <%s>\n", bufdata(&token));
                    iret = 1;
                    break;
                }
            }
            else
            {
                put_program_token(); 
                ERROR ("Expecting \"=\", found <%s>\n", bufdata(&token));
                iret = 1; 
                break;
            }
        }
        else if (bufcmp(&token, "port") == 0)
        {
            port = true;
        }
        else if (bufcmp(&token, "pipe") == 0)
        {
            pipe = true;
        }
        else if (bufcmp(&token, "socket_client") == 0)
        {
            socket_client = true;
        }
        else if (bufcmp(&token, "socket_server") == 0)
        {
            socket_server = true;
        }
        else if (bufcmp(&token, "mbx") == 0)
        {
            mbx = true;
        }
        else if (bufcmp(&token, "mbx_client") == 0)
        {
            mbx_client = true;
        }
        else if (bufcmp(&token, "mbx_server") == 0)
        {
            mbx_server = true;
        }
    }

    /* All the parsing is done.. now do the open */

    if (iret)
    {
    	;
    }
    else if (g_syntax_check)
    {
    	;
    }
    else if (port)
    {
        LOG ("Opening port <%s>, device <%s>...\n", name, device);
        iret = open_port(name, device, proto, options, &fd);
        if (iret)
        {
            ERROR ("Can't open port <%s>, device <%s>\n", name, device);
            iret = 1;
        }
        else
        {
            set_server(fd, false);
            set_fd_info(name, fd, options, proto, -1, logfile_fp);
            FD_LOG1(name) ("Opened port <%s>, device <%s>, fd <%d>\n", name, device, fd);

            /* Flush garbage chars */
            flush_open(name);
        }
    }
    else if (pipe)
    {
        LOG ("Opening pipe <%s>, device <%s>...\n", name, device);
        if ((fd = open(device, O_RDWR)) == -1)
        {
            ERROR ("Can't open pipe <%s>\n", device);
            iret = 1;
        }
        else
        {
            set_server(fd, false);
            set_fd_info(name, fd, options, proto, -1, logfile_fp);
            FD_LOG1(name) ("Opened pipe <%s>, device <%s>, fd <%d>\n", name, device, fd);

            /* Flush garbage chars */
            flush_open(name);
        }
    }
    else if (socket_client)
    {
        if (strlen(local_name))
        {
            LOG ("Opening local socket as client <%s>, <%s>...\n", name, local_name);
            fd = open_socket_client_local(local_name);
            if (fd == 0)
            {
                ERROR ("Can't open local socket, name <%s>\n", local_name);
                iret = 1;

            }
            else
            {
                set_server(fd, false);
                set_fd_info(name, fd, options, proto, logging, logfile_fp);
                FD_LOG1(name) ("Opened local socket as client <%s>, <%s>, fd <%d>\n", name, local_name, fd);

                /* Flush garbage chars */
                flush_open(name);
            }
        }
        else
        {
            LOG ("Opening socket as client host <%s>, service <%s>...\n", host, service);
            fd = open_socket_client(host, service);
            if (fd == 0)
            {
                ERROR ("Can't open socket, host <%s>, service <%s>\n", host, service);
                iret = 1;
            }
            else
            {
                set_server(fd, false);
                set_fd_info(name, fd, options, proto, logging, logfile_fp);
                FD_LOG1(name) ("Opened socket as client host <%s>, service <%s>, fd <%d>\n", host, service, fd);

                /* Flush garbage chars */
                flush_open(name);
            }
        }
    }
    else if (socket_server)
    {
        if (strlen(local_name))
        {
            LOG ("Opening local socket as server <%s>, <%s>...\n", name, local_name);
            fd = open_socket_server_local(local_name);
            if (fd == 0)
            {
                ERROR ("Can't open local socket <%s>\n", local_name);
                iret = 1;
            }
            else
            {
                set_server(fd, true);
                set_fd_info(name, fd, options, proto, logging, logfile_fp);
                FD_LOG1(name) ("Opened local socket as server <%s>, <%s>, fd <%d>\n", name, local_name, fd);

                /* Flush garbage chars */
                flush_open(name);
            }
        }
        else
        {
            LOG ("Opening socket as server host <%s>, service <%s>...\n", host, service);
            fd = open_socket_server(host, service);
            if (fd == 0)
            {
                ERROR ("Can't open socket, host <%s>, service <%s>\n", host, service);
                iret = 1;
            }
            else
            {
                set_server(fd, true);
                set_fd_info(name, fd, options, proto, logging, logfile_fp);
                FD_LOG1(name) ("Opened socket as server host <%s>, service <%s>, fd <%d>\n", host, service, fd);

                /* Flush garbage chars */
                flush_open(name);
            }
        }
    }
    else if (mbx)
    {
        iret = mbx_open(name, options, proto);
        set_fd_info(name, 0, options, proto, logging, logfile_fp);
    }
    else if (mbx_client)
    {
        iret = mbx_open_client(name, options, proto);
        set_fd_info(name, 0, options, proto, logging, logfile_fp);
    }
    else if (mbx_server)
    {
        iret = mbx_open_server(name, options, proto);
        set_fd_info(name, 0, options, proto, logging, logfile_fp);
    }
    else
    {
    	;
    }

    /* If this is a hsms client connect, do the hsms protocol connect now */

    if (g_syntax_check)
    {
    	;
    }
    else if ((socket_client) && (proto == PROTO_HSMS) && (iret == 0))
    {
        while (1)
        {
            /* This loops forever until the hsms_client_connect() has success */
            iret = hsms_client_connect(name, fd);  /* hsms protocol connect */
            if (iret)
            {
                /* Close everything up and the re-open the connection */
                close(fd);
                unset_server(fd);

                /* This code is repeated from above */
                if (strlen(local_name))
                {
                    LOG ("Opening local socket as client <%s>, <%s>...\n", name, local_name);
                    fd = open_socket_client_local(local_name);
                    if (fd == 0)
                    {
                        ERROR ("Can't open local socket, name <%s>\n", local_name);
                        iret = 1;
                        break;
                    }
                    else
                    {
                        set_server(fd, false);
                        set_fd_info(name, fd, options, proto, logging, logfile_fp);
                        FD_LOG1(name) ("Opened local socket as client <%s>, <%s>, fd <%d>\n", 
                            name, local_name, fd);
                    }
                }
                else
                {
                    LOG ("Opening socket as client host <%s>, service <%s>...\n", host, service);
                    fd = open_socket_client(host, service);
                    if (fd == 0)
                    {
                        ERROR ("Can't open socket, host <%s>, service <%s>\n", host, service);
                        iret = 1;
                        break;
                    }
                    else
                    {
                        set_server(fd, false);
                        set_fd_info(name, fd, options, proto, logging, logfile_fp);
                        FD_LOG1(name) ("Opened socket as client host <%s>, service <%s>, fd <%d>\n", 
                            host, service, fd);
                    }
                }
            }
            else
            {
                break;  /* Success */
            }
        }
    }

    /* Flush garbage chars */
    /* flush_open(name); */

    buffree(&token);
    DEBUG (3, DEBUG_HDR, "Returning, error <%d>\n", iret);
    return iret;
}

/********************************************/

uint32_t cmd_close()
{
	int fd = 0;
	uint32_t iret = 0;
	enum token_types type = TT_NOT_ASSIGNED_YET;
	struct buffer token = INIT_BUFFER;
	char name[128];

#undef NAME
#define NAME "cmd_close()"

	DEBUG (3, DEBUG_HDR, "Called\n");

    bufcpy(&token, "");
//    memset(name, '\0', sizeof(name));
	name[0] = 0;

    while (1)
    {
        get_program_token(&token, &type);    
        DEBUG (3, DEBUG_HDR, "Token <%s>, type <%s>\n", bufdata(&token), show_type(type));
        if ((type == TT_END_OF_LINE) || (type == TT_NULL_TOKEN))
        {
            break;
        }
        else if (bufcmp(&token, "name") == 0)
        {
            get_program_token(&token, &type);    
            if (type == TT_ASSIGN)
            {
                get_program_token(&token, &type);    
                if (strlen(bufdata(&token)) >= sizeof(name))
                {
                    ERROR ("Name <%s> is too long, limited to <%d>\n", sizeof(name));
                    iret = 1;
                    break;
                }
                else
                {
                    strcpy(name, bufdata(&token));
                }
            }
            else
            {
                put_program_token(); 
                ERROR ("Expecting \"=\", found <%s>\n", bufdata(&token));
                iret = 1; 
                break;
            }
        }
    }
 
    while (1)
    {
        if (iret)
        {
            break;
        }
   
        if ((fd = get_fd(name)) == 0)
        {
            iret = 0;
            break;
        }

        FD_LOG1(name) ("Closing <%s>, fd <%d>\n", name, fd);
        if (close(fd) == -1)
        {
            ERROR ("Can't close connection fd, name <%s>, fd <%d>\n", name, fd);
            iret = 0;
            break;
        }
 
        iret = unset_server(fd);
        if (iret)
        {
            ERROR ("Error trying to reset server in CLOSE statement\n");
            iret = 0;
            break;
        }
        break;
    }

    buffree(&token);
    DEBUG (3, DEBUG_HDR, "Returning, error <%d>\n", iret);
    return iret;
}

/********************************************/

static void flush_open(const char *name)
{
#ifdef __CYGWIN__
  sigjmp_buf flush_timer;
#else
	jmp_buf flush_timer;
#endif
	int flush_time = 0;
	int fd = 0;;
	uint8_t c = 0;
	char *option = (char *)NULL;

#undef NAME
#define NAME "flush_open()"

    DEBUG (3, DEBUG_HDR, "Called, name <%s>\n", name);

    fd = get_fd(name);

    option = get_option("flush=", name);

    if (strlen(option))
    {
        flush_time = atoi(option);
        DEBUG (3, DEBUG_HDR, "Flush time is <%d>\n", flush_time);

    }
    else
    {
        DEBUG (3, DEBUG_HDR, "Flush option not defined\n");
        return;
    }

    if (g_syntax_check)
    {
        return;
    }

    LOG ("Flushing all data for <%d> seconds...\n", flush_time);

    DEBUG (3, DEBUG_HDR, "Flushing...\n");
    while (1)
    {
//        if (sigsetjmp(flush_timer, 1) != 0)
        if (do_the_setsigjump(flush_timer, 1) != 0)
        {
            break;
        }

        start_proto_timer((uint32_t)flush_time, 1, &flush_timer);
        my_readc(name, fd, &c);  
        DEBUG (3, DEBUG_HDR, "Flushing <%.2X> from fd <%s>", (unsigned char) c, name);
        stop_proto_timer(1);
    }

    DEBUG (3, DEBUG_HDR, "Done flushing\n");
    return;
}
