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

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "include.h" 

//extern int h_errno;  /* Defined in netdb.h */

/********************************************/

int open_socket_server(char *host, char *service)
{
int fd, /*iret, */opt_val;
socklen_t sockaddr_len;
struct servent *sp;
struct sockaddr_in sockaddr;

#undef NAME
#define NAME "open_socket_server()"

    DEBUG (3, DEBUG_HDR, "Called, host <%s>, service <%s>\n", host, service);

    DEBUG (3, DEBUG_HDR, "Opening socket\n");
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        ERROR ("Open socket failed\n");
        DEBUG (3, DEBUG_HDR, "Returning <0>\n");
        return 0;  
    }

    /* Set socket option.. this allows the service to be used immediately 
       after being closed without waiting */ 
    opt_val = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const void *) &opt_val, sizeof(opt_val));

    memset(&sockaddr, 0, sizeof(struct sockaddr_in));
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_addr.s_addr = INADDR_ANY;
    sockaddr.sin_port = 0;
    sockaddr_len = sizeof(struct sockaddr_in);

    if (atoi(service) == 0)
    {
        DEBUG (3, DEBUG_HDR, "Getting port/service by name, getservbyname()\n");
        if ((sp = getservbyname(service, "tcp")) == NULL)
        {
            ERROR ("Unable to get network services data for service <%s>\n", service);
            DEBUG (3, DEBUG_HDR, "Returning <0>\n");
            close (fd);
            return 0;
        }
        DEBUG (3, DEBUG_HDR, "Success, name <%s>, aliases <%s>, port <%d>, proto <%s>\n",
            sp->s_name, sp->s_aliases, sp->s_port, sp->s_proto);

        sockaddr.sin_port = htons((uint16_t)sp->s_port);

    }
    else
    {
        sockaddr.sin_port = htons((uint16_t)atoi(service));
    }

    DEBUG (3, DEBUG_HDR, "Sockaddr struct, sin_family <%d>, sin_port <%d>, sin_addr <%s>, len <%d>\n",
        sockaddr.sin_family, sockaddr.sin_port, inet_ntoa(sockaddr.sin_addr), sockaddr_len);

    DEBUG (3, DEBUG_HDR, "Binding socket\n");
    if (bind(fd, (struct sockaddr *) &sockaddr, sockaddr_len) < 0)
    {
        ERROR ("Socket bind failed, service <%s>, fd <%d>\n", service, fd);
        DEBUG (3, DEBUG_HDR, "Returning <0>\n");
        close (fd);
        return 0;  
    }

    DEBUG (3, DEBUG_HDR, "Listening at socket\n");
    if (listen(fd, 5) < 0)
    {
        ERROR ("Listen failed, fd <%d>\n", fd);
        DEBUG (3, DEBUG_HDR, "Returning <0>\n");
        close (fd);
        return 0;  
    }

    DEBUG (3, DEBUG_HDR, "Socket opened, returning, fd <%d>\n", fd);
    return fd;
}

/********************************************/

int open_socket_server_local(char *name)
{
int fd/*, iret*/;
socklen_t len;
/*struct servent *sp;*/
struct sockaddr_un unix_addr, tmp;
#undef NAME
#define NAME "open_socket_server_local()"

    DEBUG (3, DEBUG_HDR, "Called, name <%s>\n", name);

    DEBUG (3, DEBUG_HDR, "Opening socket\n");
    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) { 
        ERROR ("Open socket failed\n");
        DEBUG (3, DEBUG_HDR, "Returning <0>\n");
        return 0;  
    }

    memset(&unix_addr, 0, sizeof(unix_addr));
    unix_addr.sun_family = AF_UNIX;
    strncpy(unix_addr.sun_path, name, sizeof(unix_addr.sun_path)-1);
    len = (socklen_t)(strlen(unix_addr.sun_path) + sizeof (unix_addr.sun_family));

    unlink(unix_addr.sun_path);  /* Delete if already exists */

#ifdef oldcode
    if (len != 16) {
        ERROR ("Invalid length for unix_addr, should be 16, len <%d>\n", len);
        DEBUG (3, DEBUG_HDR, "Returning <0>\n");
        return 0;  
    } 
#endif

    DEBUG (3, DEBUG_HDR, "Sockaddr_un struct, sun_family <%d>, sun_path <%s>, len <%d>\n",
        unix_addr.sun_family, unix_addr.sun_path, len);

    DEBUG (3, DEBUG_HDR, "Binding socket\n");
    if (bind(fd, (struct sockaddr *) &unix_addr, sizeof(struct sockaddr_un)) < 0) {
        ERROR ("Socket bind failed, name <%s>, fd <%d>\n", name, fd);
        DEBUG (3, DEBUG_HDR, "Returning <0>\n");
        close (fd);
        return 0;  
    }

    DEBUG (3, DEBUG_HDR, "Listening at socket\n");
    if (listen(fd, 5) < 0) {
        ERROR ("Listen failed, fd <%d>\n", fd);
        DEBUG (3, DEBUG_HDR, "Returning <0>\n");
        close (fd);
        return 0;  
    }

    len = sizeof(tmp);
    getsockname(fd, (struct sockaddr *) &tmp, &len);
    DEBUG (3, DEBUG_HDR, "Bound name <%s>, len <%d>\n", tmp.sun_path, len);

    DEBUG (3, DEBUG_HDR, "Socket opened, returning, fd <%d>\n", fd);
    return fd;
}

/********************************************/

int open_socket_client(char *host, char *service)
{
	int fd;
	socklen_t sockaddr_len;
	struct servent *sp;
	struct hostent *hp;
	struct sockaddr_in sockaddr;

#undef NAME
#define NAME "open_socket_client()"

    DEBUG (3, DEBUG_HDR, "Called, host <%s>, service <%s>\n", host, service);

    DEBUG (3, DEBUG_HDR, "Opening socket\n");
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        ERROR ("Open socket failed\n");
        DEBUG (3, DEBUG_HDR, "Returning <0>\n");
        return 0;
    }

    memset(&sockaddr, 0, sizeof(struct sockaddr_in));
    sockaddr.sin_family = AF_INET;
    sockaddr_len = sizeof(struct sockaddr_in);
  
    /* Assumes host was specified in dot notation */ 
    sockaddr.sin_addr.s_addr = inet_addr(host);

//    if (sockaddr.sin_addr.s_addr == -1)
    if (sockaddr.sin_addr.s_addr == UINT32_MAX)
    {  /* Dot notation not used, use gethostbyname() */
        DEBUG (3, DEBUG_HDR, "Getting host by name, host <%s>\n", host);
        if ((hp = gethostbyname(host)) == NULL)
        {
            ERROR ("Unable to get host data for host <%s>, h_errno <%d> (defined in netdb.h)\n",
                host, h_errno);
            DEBUG (3, DEBUG_HDR, "Returning <0>\n");
            close (fd);
            return 0;
        }
        sockaddr.sin_addr.s_addr = ((struct in_addr *) (hp->h_addr))->s_addr;
    }
    DEBUG (3, DEBUG_HDR, "sin_addr <%s>\n", inet_ntoa(sockaddr.sin_addr));

    if (atoi(service) == 0)
    {
        DEBUG (3, DEBUG_HDR, "Getting server by name info\n");
        if ((sp = getservbyname(service, "tcp")) == NULL)
        {
            ERROR ("Unable to get network services data for service <%s>\n", service);
            DEBUG (3, DEBUG_HDR, "Returning <0>\n");
            close (fd);
            return 0;
        }
        DEBUG (3, DEBUG_HDR, "Success, name <%s>, aliases <%s>, port <%d>, proto <%s>\n",
            sp->s_name, sp->s_aliases, sp->s_port, sp->s_proto);
        sockaddr.sin_port = htons((uint16_t)sp->s_port);

    }
    else
    {
        sockaddr.sin_port = htons((uint16_t)atoi(service));
    }
    DEBUG (3, DEBUG_HDR, "sin_port <%d>\n", sockaddr.sin_port);

    DEBUG (3, DEBUG_HDR, "Sockaddr struct, sin_family <%d>, sin_port <%d>, sin_addr <%s>, len <%d>\n",
        sockaddr.sin_family, sockaddr.sin_port, inet_ntoa(sockaddr.sin_addr), sockaddr_len);

    if (g_syntax_check)
    {

    }
    else
    {
        DEBUG (3, DEBUG_HDR, "Connecting to socket\n");
        if (connect(fd, (struct sockaddr *) &sockaddr, sockaddr_len) < 0)
        {
            ERROR ("Connect failed, service <%s>, fd <%d>\n", service, fd);
            DEBUG (3, DEBUG_HDR, "Returning <0>\n");
            close (fd);
            return 0;
        }
    }

    DEBUG (3, DEBUG_HDR, "Socket opened, returning, fd <%d>\n", fd);
    return fd;
}

/********************************************/

int open_socket_client_local(char *name)
{
int fd;
socklen_t len;
struct sockaddr_un unix_addr, tmp;
#undef NAME
#define NAME "open_socket_client_local()"

    DEBUG (3, DEBUG_HDR, "Called, name <%s>\n", name);

    DEBUG (3, DEBUG_HDR, "Opening socket\n");
    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        ERROR ("Open socket failed\n");
        DEBUG (3, DEBUG_HDR, "Returning <0>\n");
        return 0;
    }

    memset(&unix_addr, 0, sizeof(unix_addr));
    unix_addr.sun_family = AF_UNIX;
    sprintf(unix_addr.sun_path, "%s%05d", name, getpid());
    len = (socklen_t)(strlen(unix_addr.sun_path) + sizeof (unix_addr.sun_family));

    unlink(unix_addr.sun_path);  /* Delete if already exists */

    DEBUG (3, DEBUG_HDR, "Sockaddr_un struct, sin_family <%d>, sun_path <%s>, len <%d>\n",
        unix_addr.sun_family, unix_addr.sun_path, len);

    DEBUG (3, DEBUG_HDR, "Binding socket\n");
    if (bind(fd, (struct sockaddr *) &unix_addr, len) < 0) {
        ERROR ("Socket bind failed, name <%s>, fd <%d>\n", name, fd);
        DEBUG (3, DEBUG_HDR, "Returning <0>\n");
        close (fd);
        return 0;
    }

    /* For debugging only */
    len = sizeof(tmp);
    getsockname(fd, (struct sockaddr *) &tmp, &len);
    DEBUG (3, DEBUG_HDR, "Bound name <%s>, len <%d>\n", tmp.sun_path, len);

    if (chmod(unix_addr.sun_path, S_IRWXU) < 0) {
        ERROR ("Error setting permission on local socket, name <%s>, fd <%d>\n", name, fd);
        DEBUG (3, DEBUG_HDR, "Returning <0>\n");
        close (fd);
        return 0;
    }

    memset(&unix_addr, 0, sizeof(unix_addr));
    unix_addr.sun_family = AF_UNIX;
    strncpy(unix_addr.sun_path, name, sizeof(unix_addr.sun_path)-1);
    len = (socklen_t)(strlen(unix_addr.sun_path) + sizeof (unix_addr.sun_family));
    DEBUG (3, DEBUG_HDR, "Sockaddr_un struct, sin_family <%d>, sun_path <%s>, len <%d>\n",
        unix_addr.sun_family, unix_addr.sun_path, len);

    if (g_syntax_check) {

    } else {
        DEBUG (3, DEBUG_HDR, "Connecting to socket\n");
        if (connect(fd, (struct sockaddr *) &unix_addr, len) < 0) {
            ERROR ("Connect failed, name <%s>, fd <%d>\n", name, fd);
            DEBUG (3, DEBUG_HDR, "Returning <0>\n");
            close (fd);
            return 0;
        }
    }

    DEBUG (3, DEBUG_HDR, "Socket opened, returning, fd <%d>\n", fd);
    return fd;
}

/********************************************/

