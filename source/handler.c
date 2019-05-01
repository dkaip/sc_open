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

#include <netdb.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "include.h"

/* This defines an opened connection, either client or server */

static struct
{
    char name[128];
    int fd;  /* The fd of the server */
    char options[128];
    char override_options[128];
    enum protos proto;
    int client_fd;  /* fd of the last client which sent a message */
    int logging;
    FILE *logfile_fp;
} fd_info[MAX_FDS];

/********************************************/

uint32_t handler(int s_fd, int c_fd)
{
	struct buffer reply = INIT_BUFFER;
	enum protos proto = PROTO_UNKNOWN;
	uint32_t cnt = 0;
	bool primary_msg_flag = false;
	uint32_t program_idx;
	uint32_t iret = 0;
	char buf[128];
	char *fmt = (char *)NULL;
	char *name = (char *)NULL;

#undef NAME
#define NAME "handler()"

	buf[0] = 0;

    DEBUG (3, DEBUG_HDR, "Called, server fd <%d>, client fd <%d>\n", s_fd, c_fd);
 
    LOG ("Recv unsol on <%s>, client connected as <%d>\n", get_name(s_fd), c_fd);

    iret = 0;

    name = get_name(s_fd);
    proto = get_proto(name);

    update_fd_info(name, c_fd);  /* Update the client_fd in the fd_info table */
    bufcpy(&reply, "");
    
    DEBUG (3, DEBUG_HDR, "Handling proto <%d>, <%s>\n", proto, show_proto(proto));

    if (proto == PROTO_SECS)
    {
        tmp_variable_delete_all();
        iret = recv_secs(name, c_fd, 0, &program_idx, &primary_msg_flag);
        if (iret)
        {
            if (iret == 2)
            {  /* A protocol error, not really an error */
                iret = 0;
            }
            else
            {
                ERROR ("Error receiving secs message\n");
            }
        }
        else if (primary_msg_flag == true)
        {
            /* Received primary message */
            DEBUG (3, DEBUG_HDR, "Processing received message as an event\n");
            /* make_tmp_variables(bufdata(&reply), "", 0); */
            tmp_variable_sort();
            iret = resume_via_program_label(get_name(s_fd));
        }
        else
        {
            /* Received secondary reply message */
            DEBUG (3, DEBUG_HDR, "Processing received message as a reply message\n");
            /* make_tmp_variables(bufdata(&reply), "", 0); */
            tmp_variable_sort();
            iret = resume_via_program_idx(program_idx);
        }
    }
    else if (proto == PROTO_HSMS)
    {
        tmp_variable_delete_all();
        iret = recv_hsms(name, c_fd, &program_idx, &primary_msg_flag);
        if (iret)
        {
            if (iret == 2)
            {  /* Not an error, just handing the protocol */
                iret = 0; 
            }
            else
            {
                ERROR ("Error receiving hsms message\n");
            }
        }
        else if (primary_msg_flag == true)
        {
            /* Received primary message */
            DEBUG (3, DEBUG_HDR, "Processing received message as an event\n");
            tmp_variable_sort();
            /* make_tmp_variables(bufdata(&reply), "", 0); */
            iret = resume_via_program_label(get_name(s_fd));
        }
        else
        {
            /* Received secondary reply message */
            DEBUG (3, DEBUG_HDR, "Processing received message as a reply message\n");
            tmp_variable_sort();
            /* make_tmp_variables(bufdata(&reply), "", 0); */
            iret = resume_via_program_idx(program_idx);
        }

    }
    else if (proto == PROTO_STANDARD)
    {
        bufcpy(&reply, "");
        iret = recv_standard(name, c_fd, &reply);
        if (get_logging(name) == 1)
        {
            fmt = "Recv <%.20s...>\n";

        }
        else
        {
            fmt = "Recv <%s>\n";
        } 
        FD_LOG1(name) (fmt, bufdata(&reply));
        if (iret)
        {
        	;
        }
        else
        {
            make_tmp_variables(bufdata(&reply), get_option("no_parse=", name), 1);
            iret = resume_via_program_label(get_name(s_fd));
        }
    }
    else if (proto == PROTO_XML)
    {
        bufcpy(&reply, "");
        iret = recv_xml(name, c_fd, &reply);
        LOG ("Recv <%s>\n", bufdata(&reply));
        if (iret)
        {
        	;
        }
        else
        {
            /* variable_put("ARGS", bufdata(&reply), VT_TMP); */
            iret = resume_via_program_label(get_name(s_fd));
        }
    }
    else
    {
        DEBUG (3, DEBUG_HDR, "Using default, command proto\n");
        while (1)
        {
            iret = my_read(name, c_fd, buf, sizeof(buf), &cnt);
            if (iret)
            {
                break;
            }
            else
            {
                bufncat(&reply, buf, cnt);
                if (strchr(buf, '\n') == (char *)NULL)
                {
                	;
                }
                else
                {
                    break;
                }
            }
        }

        if (iret)
        {
        	;
        }
        else
        {
            DEBUG (3, DEBUG_HDR, "Received on client fd <%d>, <%s>\n", c_fd, bufdata(&reply));
            bufcpy(&reply, buf);
            commands(&reply);
            const char *p1 = bufmkprint(&reply);
            /* my_write(name, c_fd, bufdata(&reply), bufsize(&reply)); */
            (void)my_write(name, c_fd, (const uint8_t *)p1, (uint32_t)strlen(p1));
            (void)my_write(name, c_fd, (const uint8_t *)SC_PROMPT, sizeof(SC_PROMPT));
        }
    }

    DEBUG (3, DEBUG_HDR, "Returning, <%d>\n", iret);
    buffree(&reply);
    return iret;
}

/********************************************/

int get_fd(const char *name)
{
	int i  = 0;
	int fd = 0;

#undef NAME
#define NAME "get_fd()"

    DEBUG (3, DEBUG_HDR, "Called, name <%s>\n", name);

    for (i=0; i<MAX_FDS; i++)
    {
        if (strcmp(name, fd_info[i].name) == 0)
        {
            fd = fd_info[i].fd;
            break;
        }
    }

    if (i == MAX_FDS)
    {
        ERROR ("No fd found for name <%s>\n", name);
        fd = 0;
    }

    DEBUG (3, DEBUG_HDR, "Returning, fd <%d>\n", fd);
    return fd;
}

/********************************************/

int get_client_fd(char *name)
{
int i, fd;
#undef NAME
#define NAME "get_client_fd()"

    DEBUG (3, DEBUG_HDR, "Called, name <%s>\n", name);

    for (i=0; i<MAX_FDS; i++) {
        if (strcmp(name, fd_info[i].name) == 0) {
            fd = fd_info[i].client_fd;
            break;
        }
    }

    if (i == MAX_FDS) {
        ERROR ("No client fd found for name <%s>\n", name);
        fd = 0;
    }

    DEBUG (3, DEBUG_HDR, "Returning, fd <%d>\n", fd);
    return fd;
}

/********************************************/

char *get_option(const char *option, const char *name)
{
	uint32_t i = 0;

#undef NAME
#define NAME "get_option()"

    DEBUG (3, DEBUG_HDR, "Called, option <%s>, name <%s>\n", option, name);

    for (i=0; i<MAX_FDS; i++)
    {
        if (strcmp(name, fd_info[i].name) == 0)
        {

        /* if ((fd_info[i].fd == fd) || 
            (fd_info[i].client_fd == fd)) { */

            break;
        }
    }

    if (i == MAX_FDS)
    {
        ERROR ("No option found for for <%s>\n", name);
        return "";
    }

    /* Check the override options first */
    char *opt = get_opt(fd_info[i].override_options, option);
    if (strlen(opt) == 0)
    {
        opt = get_opt(fd_info[i].options, option);
    }

    DEBUG (3, DEBUG_HDR, "Returning, option value <%s>\n", opt);
    return opt;
}

/********************************************/

char *get_opt(const char *options, const char *option)
{
	uint32_t i = 0;
	bool flag = false;
char *v_ptr, *ptr;
static char value[128];
#undef NAME
#define NAME "get_opt()"

    DEBUG (3, DEBUG_HDR, "Called, options <%s>, option <%s>\n", options, option);

    if ((ptr = strstr(options, option)) == '\0')
    {
        /* ERROR ("Option not found <%s>, returning <\"\">\n", option);  */
        return "";
    }

    memset(value, '\0', sizeof(value));
    v_ptr = value;
    ptr = ptr + strlen(option);
    flag = false;
    for (i=0; i<sizeof(value); i++)
    {
        if (flag)
        {  /* Inside a quoted string */
            if (*ptr == '"')
            {  /* Closing quote */
                break;  /* quit */
            }
            else
            {
                *v_ptr++ = *ptr++;  /* Copy char */ 
            }

        }
        else if (*ptr == '"')
        {  /* Opening quote */
            flag = true;
            ptr++;

        }
        else
        {
            if (strchr(" ", *ptr) == NULL)
            {
                *v_ptr++ = *ptr++; 
            }
            else
            {
                break;
            }
        }
    }

    if (i >= sizeof(value))
    {
        FATAL ("Option string length too big to handle <%s>\n", option);
    }
    /* sscanf(ptr, "%127s", value); */

    DEBUG (3, DEBUG_HDR, "Returning, option value <%s>\n", value);
    return value;
}

/********************************************/

char *get_name(int fd)
{
	char *name = (char *)NULL;

#undef NAME
#define NAME "get_name()"

    DEBUG (3, DEBUG_HDR, "Called, fd <%d>\n", fd);

    bool found_it = false;
    for (uint32_t i=0; i<MAX_FDS; i++)
    {
        if ((fd_info[i].fd == fd) || 
            (fd_info[i].client_fd == fd))
        {
            name = fd_info[i].name;
            found_it = true;
            break;
        }
    }

    if (found_it == false)
    {
        name = "";
        FATAL ("No name info found for fd <%d> in fd_info table\n", fd); 
    }

    DEBUG (3, DEBUG_HDR, "Returning, name <%s>\n", name);
    return name;
}

/********************************************/

FILE *get_logfile_fp(const char *name)
{
int i;
FILE *fp;
#undef NAME
#define NAME "get_logfile_fp()"

    DEBUG (3, DEBUG_HDR, "Called, name <%s>\n", name);

    for (i=0; i<MAX_FDS; i++)
    {
        if (strcmp(fd_info[i].name, name) == 0)
        {
            fp = fd_info[i].logfile_fp;
            break;
        }
    }

    if (i == MAX_FDS)
    {
        fp = NULL;
        /* FATAL ("No logfile fp info found for fd <%d> in fd_info table\n", fd); */
    }

    DEBUG (3, DEBUG_HDR, "Returning\n");
    return fp;
}

/********************************************/

int get_logging(const char *name)
{
	int i = 0;
	int logging = 0;

#undef NAME
#define NAME "get_logging()"

    DEBUG (3, DEBUG_HDR, "Called, name <%s>\n", name);
    for (i=0; i<MAX_FDS; i++)
    {
        if (strcmp(fd_info[i].name, name) == 0)
        {
            logging = fd_info[i].logging;
            break;
        }
    }

    if (i == MAX_FDS)
    {
        logging = 2;  /* Use as default */
        /* FATAL ("No logging info found for fd <%d> in fd_info table\n", fd); */
    }

    DEBUG (3, DEBUG_HDR, "Returning, logging <%d>\n", logging);
    return logging;
}

/********************************************/

void set_logging(const char *name, int value)
{
int i;
#undef NAME
#define NAME "set_logging()"

    DEBUG (3, DEBUG_HDR, "Called, name <%s>, value <%d>\n", name, value);

    for (i=0; i<MAX_FDS; i++) {
        if (strcmp(fd_info[i].name, name) == 0) {
            fd_info[i].logging = value;
            break;
        }
    }
    if (i == MAX_FDS) {
        FATAL ("No logging info found for name <%s> in fd_info table\n", name);
    }

    DEBUG (3, DEBUG_HDR, "Returning\n");
    return;
}

/********************************************/

void show_logging(struct buffer *buffer)
{
int i;
char buf[512];
#undef NAME
#define NAME "show_logging()"

    DEBUG (3, DEBUG_HDR, "Called\n");

    bufcat(buffer, "Current logging is:\n");

    for (i=0; i<MAX_FDS; i++)
    {
        if (fd_info[i].fd)
        {
            sprintf(buf, "  Name <%s>, logging <%d>\n",
                fd_info[i].name, fd_info[i].logging);
            bufcat(buffer, buf);
        }
    }

    return;
}

/********************************************/

void update_fd_info(const char *name, int client_fd)
{
int i;
#undef NAME
#define NAME "update_fd_info()"

    DEBUG (3, DEBUG_HDR, "Called, name <%s>, client fd <%d>\n", name, client_fd);

    for (i=0; i<MAX_FDS; i++)
    {
        if (strcmp(fd_info[i].name, name) == 0)
        {
            fd_info[i].client_fd = client_fd;
            break;
        }
    }
    if (i == MAX_FDS)
    {
        /* FATAL ("No fd <%d> info found in fd_info table\n", fd); */
    }

    DEBUG (3, DEBUG_HDR, "Returning\n");
    return;
}

/********************************************/

enum protos get_proto(const char *name)
{
	int i = 0;
	enum protos proto = PROTO_UNKNOWN;

#undef NAME
#define NAME "get_proto()"

    DEBUG (3, DEBUG_HDR, "Called, name <%s>\n", name);

    for (i=0; i<MAX_FDS; i++)
    {
        if ((strcmp(fd_info[i].name, name) == 0))
        {
            proto = fd_info[i].proto;
            break;
        }
    }

    if (i == MAX_FDS)
    {
        FATAL ("No proto info found for name <%s> in fd_info table\n", name);
    }

    DEBUG (3, DEBUG_HDR, "Returning <%s>\n", show_proto(proto));
    return proto;
}

/********************************************/

void set_fd_info(char *name, int fd, char *options, enum protos proto, int logging, FILE *logfile_fp)
{
int i;
#undef NAME
#define NAME "set_fd_info()"

    DEBUG (3, DEBUG_HDR, "Called, name <%s>, fd, <%d>, option <%s>\n", name, fd, options);

    for (i=0; i<MAX_FDS; i++)
    {
        if (strcmp(fd_info[i].name, name) == 0)
        {
            DEBUG (3, DEBUG_HDR, "Name already exists, so updating it\n");
            break;
        } 
    }

    if (i == MAX_FDS)
    {
        for (i=0; i<MAX_FDS; i++)
        {
            if (strlen(fd_info[i].name) == 0)
            {
                DEBUG (3, DEBUG_HDR, "Found empty location\n");
                break;
            }
        }

        if (i == MAX_FDS)
        {
            FATAL ("Too many fd_info info table entries, limited to <%d>\n", MAX_FDS);
        }
    }

    fd_info[i].fd = fd;
    fd_info[i].proto = proto;
    strcpy(fd_info[i].options, options);
    strcpy(fd_info[i].name, name);
    fd_info[i].logging = logging;
    fd_info[i].logfile_fp = logfile_fp;

    return;
}

/********************************************/

void override_options(char *name, char *override_options)
{
int i;
#undef NAME
#define NAME "override_options()"

    DEBUG (3, DEBUG_HDR, "Called, name <%s>, override options <%s>\n", name, override_options);

    for (i=0; i<MAX_FDS; i++) {
        if (strcmp(fd_info[i].name, name) == 0) {
            DEBUG (3, DEBUG_HDR, "Name already exists, so updating it\n");
            break;
        } 
    }

    if (i == MAX_FDS) {
        FATAL ("Trying to override options for name <%s>, but info does not exist\n", name);
    }

    strcpy(fd_info[i].override_options, override_options);

    return;
}

/********************************************/

void init_handler()
{
int i;
#undef NAME
#define NAME "init_handler()"

    DEBUG (3, DEBUG_HDR, "Called\n");

    for (i=0; i<MAX_FDS; i++)
    {
        fd_info[i].name[0] = 0;
        fd_info[i].fd = 0;
        fd_info[i].options[0] = 0;
        fd_info[i].override_options[0] = 0;
        fd_info[i].proto = PROTO_UNKNOWN;
        fd_info[i].client_fd = 0;
        fd_info[i].logging = 0;
        fd_info[i].logfile_fp = (FILE *)NULL;
//        strcpy(fd_info[i].name, "");
//        fd_info[i].fd = 0;
//        strcpy(fd_info[i].options, "");
//        strcpy(fd_info[i].override_options, "");
//        fd_info[i].client_fd = 0;
//        fd_info[i].logging = 0;
//        fd_info[i].proto = PROTO_UNKNOWN;
    }
    return;
}

/********************************************/

void dump_handler (struct buffer *buffer)
{
int i;
char buf[512];
#undef NAME
#define NAME "dump_handler()"

    DEBUG (3, DEBUG_HDR, "Called\n");

    bufcat(buffer, "Handlers:\n"); 
    for (i=0; i<MAX_FDS; i++) {
        if (fd_info[i].fd) { 
            sprintf(buf, "  Name <%s>, fd <%d>, proto <%s>, client fd <%d>, logging <%d>\n",
                fd_info[i].name, fd_info[i].fd, show_proto(fd_info[i].proto), 
                fd_info[i].client_fd, fd_info[i].logging);
            bufcat(buffer, buf);
            sprintf(buf, "    options <%s>, override <%s>\n", 
                fd_info[i].options, fd_info[i].override_options);
            bufcat(buffer, buf);
        } 
    }
}

/********************************************/


