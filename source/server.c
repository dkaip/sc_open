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

#include <sys/socket.h>
#include <sys/select.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <signal.h>
#include <setjmp.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include "include.h"

/* static struct fd_set rset, allset; */
static fd_set rset, allset;
static void calc_maxfd();
static void timeout_handler();
static void sigusr1_handler();

/* List of all the server connections (One for each listening server) */

static struct
{
    int fd;
    bool flag;  /* false=no clients so don't allow accept(), true=clients can connect so allow accept() */
} my_fds[MAX_FDS];

/* List of connected clients (One for each client connected, and the sever to which it is connected */

static struct {
    int client_fd;
    int fd;
} client_fds[MAX_CLIENTS];

static int maxfd;
static unsigned int timeout_value;
#ifdef __CYGWIN__
static sigjmp_buf env_alrm;
static sigjmp_buf env_sigusr1;
#else
static jmp_buf env_alrm;
static jmp_buf env_sigusr1;
#endif

/********************************************/

void server()
{
	int num_clients = 0;

	uint32_t i = 0;
	int n = 0;
	int client_fd = 0;
	socklen_t len;
	clock_t cpu_usage1=0, cpu_usage2=0;
	struct sockaddr sockaddr;
	struct timeval timeout, *p_timeout;

#undef NAME
#define NAME "server()"

    DEBUG (3, DEBUG_HDR, "Called\n");

    if (t_tv_sec || t_tv_usec)
    {
        timeout.tv_sec  = t_tv_sec;
        timeout.tv_usec = t_tv_usec;
        p_timeout = &timeout;
    
    }
    else
    {
        p_timeout = '\0';
    }

    while (1)
    {

/***************************************************************
        if (maxfd == 0) {
            ERROR ("No maxfd set, returning\n");
            break;
        }
***************************************************************/
        rset = allset;

        /* Setup for the user timers */

        timer_handler(); 
//        if (sigsetjmp(env_alrm, 1) != 0)
        if (do_the_setsigjump(env_alrm, 1) != 0)
        {
            /* Jump here if the timeout_handler is called */
            DEBUG (3, DEBUG_HDR, "siglongjmp() from timeout_handler() signal handler\n");
            timer_handler(); 
            continue;
        }

        if (signal(SIGALRM, (void (*) (int)) timeout_handler) == SIG_ERR)
        {
             FATAL ("Error setting signal for SIGALRM\n");
        }

        DEBUG (3, DEBUG_HDR, "Setting alarm to <%u>\n", timeout_value);
        alarm(timeout_value);

        /* Setup for SIGUSR1 handling */        

        user_sigusr1_setup();  /* Setup for sigusr1 handling */
//        if (sigsetjmp(env_sigusr1, 1) != 0)
        if (do_the_setsigjump(env_sigusr1, 1) != 0)
        {
            /* Jump here if the sigusr1_handler is called */
            DEBUG (3, DEBUG_HDR, "siglongjmp() from sigusr1_handler() signal handler\n");
            alarm(0);  /* Disable user timers */
            user_sigusr1_handler();  /* User handerl for sigusr1 */
            continue;
        }

        if (signal(SIGUSR1, (void (*) (int)) sigusr1_handler) == SIG_ERR)
        {
            FATAL ("Error setting signal for SIGUSR1\n");
        }

        /* fds_bits won't compile under Linux */
        /* DEBUG (3, DEBUG_HDR, "Prior to select, rset.fds_bits <%Xh> <%Xh> \n", 
            rset.fds_bits[0], rset.fds_bits[1]); */

        /* LOG ("Idle...\n"); */

        cpu_usage2 = clock();
        if (is_pragma(PRAGMA_LOG_CPU_USAGE))
        {
            LOG ("Going idle... cpu usage <%d>ms, total cpu usage <%d>ms \n",
                cpu_usage2 - cpu_usage1, cpu_usage2);
            cpu_usage1 = cpu_usage2;
        }

        DEBUG (3, DEBUG_HDR, "Waiting at select, maxfd <%d>...\n", maxfd);
        n = select(maxfd + 1, &rset, NULL, NULL, p_timeout);
        if (n < 0)
        {
            FATAL ("Select error, ret <%d>\n", n);
        }
        DEBUG (3, DEBUG_HDR, "Detected some activity from select\n");

        DEBUG (3, DEBUG_HDR, "Waiting at select, maxfd <%d>...\n", maxfd);
        alarm(0);  /* Disable user timers */

        /* Got something from list of inputs */ 

        /* DEBUG (3, DEBUG_HDR, "Select triggered, rset.fds_bits <%Xh> <%Xh> \n", 
            rset.fds_bits[0], rset.fds_bits[1]); */

        /* Determine which of the fd's is active 
           See if it is a client trying to connect to a server */

        for (i=0; i<MAX_FDS; i++)
        {
            if (my_fds[i].fd == -1)
            {
                continue;  /* Not a active fd */
            }

            DEBUG (3, DEBUG_HDR, "Checking FD_ISSET for fd <%d>\n", my_fds[i].fd);

            if (FD_ISSET(my_fds[i].fd, &rset))
            {
                /* Activity found on this fd */

                DEBUG (3, DEBUG_HDR, "Flag <%d>\n", my_fds[i].flag);
                if (my_fds[i].flag == true)
                {

                    /* A client is trying to connect for the first time, need to accept() */

                    DEBUG (3, DEBUG_HDR, "Waiting at accept, flag <%d>, fd <%d>...\n", 
                        my_fds[i].flag, my_fds[i].fd);
                    len = sizeof(struct sockaddr);
                    client_fd = accept(my_fds[i].fd, (struct sockaddr *) &sockaddr, &len);
                    if (client_fd  < 0)
                    {
                        ERROR ("Error waiting for client to connect\n");
                        return;
                    }

                    /* New client is now connected */

                    DEBUG (3, DEBUG_HDR, "Accept triggered, client fd <%d>\n", client_fd);
 
                    /* Add the client to the list of client_fds */

                    for (int j=0; j<MAX_CLIENTS; j++)
                    {
                        /* Look for an empty one */
                        if (client_fds[j].client_fd == 0)
                        {
                            DEBUG (3, DEBUG_HDR, "Setting client_fds[%d].client_fd to client_fd  value <%d>\n", j, client_fd);

                            client_fds[j].client_fd = client_fd;
                            client_fds[j].fd = my_fds[i].fd;

                            LOG ("New client connected to <%s> as <%d>\n", 
                                get_name(my_fds[i].fd), client_fd);

                            num_clients++;
                            break;
                        }
                        else
                        	num_clients++;
                    }

                    if (num_clients == MAX_CLIENTS)
                    {
                        ERROR ("Too many clients connected, MAX_CLIENTS is <%d>, exiting\n", 
                            MAX_CLIENTS);
                        exit(1);
                    }

                    FD_SET(client_fd, &allset);  /* Add the client to the list of fds */
                    calc_maxfd();
                    break; 
                }
                else
                {
                    /* This is not a client trying to connect */

                    DEBUG (3, DEBUG_HDR, "Non accept connect\n");
                    DEBUG (3, DEBUG_HDR, "Calling handler, fd <%d>\n", my_fds[i].fd);

                    handler(my_fds[i].fd, my_fds[i].fd);
                    continue; 
                }
            }
        }   

        if (i != MAX_FDS)
        {
            continue;
        }

        DEBUG (3, DEBUG_HDR, "Checking all the client_fds\n");

        for (i=0; i<MAX_CLIENTS; i++)
        {
            if (client_fds[i].client_fd == 0)
            {
                continue;
            }
         
            DEBUG (3, DEBUG_HDR, "Checking FD_ISSET for client fd <%d>\n", client_fds[i].client_fd);
            if (FD_ISSET(client_fds[i].client_fd, &rset))
            {

                DEBUG (3, DEBUG_HDR, "Found activity, client fd <%d>\n", client_fds[i].client_fd);

                /* Handle the message from the client */
                handler(client_fds[i].fd, client_fds[i].client_fd);
                continue;
            }
        }
    }

    DEBUG (3, DEBUG_HDR, "Returning\n");
}

/********************************************/

void set_server(int fd, bool flag)
{
#undef NAME
#define NAME "set_server()"

    DEBUG (3, DEBUG_HDR, "Called, fd <%d>, flag <%d>\n", fd, flag);

    FD_SET(fd, &allset);

    uint32_t fds_searched = 0;
    for (int i=0; i<MAX_FDS; i++)
    {
    	fds_searched++;
        if (my_fds[i].fd == -1)
        {
            my_fds[i].fd = fd;
            my_fds[i].flag = flag; 
            break;
        }
    }

    if (fds_searched == MAX_FDS)
    {
        ERROR ("Too many fds being used, MAX_FDS is <%d>, exiting\n", MAX_FDS);
        exit(1);
    }
    calc_maxfd();

    /* dump_server(); */

    DEBUG (3, DEBUG_HDR, "Returning\n");
}

/********************************************/

uint32_t unset_server(int fd)
{
	uint32_t i, iret;
#undef NAME
#define NAME "unset_server()"

    DEBUG (3, DEBUG_HDR, "Called, fd <%d>\n", fd);

    FD_CLR(fd, &allset);
    iret = 0;

    for (i=0; i<MAX_FDS; i++)
    {
        if (my_fds[i].fd == fd)
        {
            my_fds[i].fd = -1;
            my_fds[i].flag = false;
            break;
        }
    }
    if (i == MAX_FDS)
    {
        ERROR ("Fd not defined <%d>\n", fd);
        iret = 1;
    }

    maxfd = 0;
    for (i=0; i<MAX_FDS; i++)
    {
        if (my_fds[i].fd > maxfd)
        {
            maxfd = my_fds[i].fd;
        }
    }

    DEBUG (3, DEBUG_HDR, "Returning, iret <%d>\n", iret);
    return iret;
}

/********************************************/

void init_server()
{

#undef NAME
#define NAME "init_server()"

    DEBUG (3, DEBUG_HDR, "Called\n");

    FD_ZERO(&allset);
    timeout_value = 0;

    for (int i=0; i<MAX_FDS; i++)
    {
        my_fds[i].fd = -1;
        my_fds[i].flag = false;
    }

    for (int i=0; i<MAX_CLIENTS; i++)
    {
        client_fds[i].client_fd = 0;
    }

    DEBUG (3, DEBUG_HDR, "Returning\n");
}

/********************************************/

void kill_server()
{
#undef NAME
#define NAME "kill_server()"

    DEBUG (3, DEBUG_HDR, "Called\n");

    FD_ZERO(&allset);

    for (int i=0; i<MAX_FDS; i++)
    {
        if (my_fds[i].fd == -1)
        {
            continue;
        }

        FD_CLR(my_fds[i].fd, &allset);
        ERROR ("Closing connection <%s>\n", get_name(my_fds[i].fd));
        if (close(my_fds[i].fd) == -1)
        {
            ERROR ("Error closing fd <%d>\n", my_fds[i].fd);
        }
        else
        {
            my_fds[i].fd = -1;
        }
    }

    for (int i=0; i<MAX_CLIENTS; i++)
    {
        if (client_fds[i].client_fd == 0)
        {
            continue;
        }
        client_fds[i].client_fd = 0;
        FD_CLR(client_fds[i].client_fd, &allset);
    }

    maxfd = 0;

    DEBUG (3, DEBUG_HDR, "Returning\n");
}

/********************************************/

void dump_server(struct buffer *buffer)
{
int i;
char buf[512];
#undef NAME
#define NAME "dump_server()"

    bufcat(buffer, "Servers:\n");
    for (i=0; i<MAX_FDS; i++)
    {
        if (my_fds[i].fd == -1)
        {
            continue;
        }
        sprintf(buf, "  Fd <%d>, flag <%d>\n", my_fds[i].fd, my_fds[i].flag); 
        bufcat(buffer, buf);

    }

    /* bufcat(buffer, "\n"); */

    bufcat(buffer, "Connected clients:\n");
    for (i=0; i<MAX_CLIENTS; i++)
    {
        if (client_fds[i].client_fd == 0)
        {
            continue;
        }
        sprintf(buf, "  Client fd <%d>, connected to <%d>\n", 
            client_fds[i].client_fd, client_fds[i].fd); 
        bufcat(buffer, buf);
    }

}

/********************************************/

static void calc_maxfd()
{
#undef NAME
#define NAME "calc_maxfd()"

    for (int i=0; i<MAX_FDS; i++)
    {
        if (my_fds[i].fd > maxfd)
        {
            maxfd = my_fds[i].fd;
        }
    }

    for (int i=0; i<MAX_CLIENTS; i++)
    {
        if (client_fds[i].client_fd > maxfd)
        {
            maxfd = client_fds[i].client_fd;
        }
    }

    DEBUG (3, DEBUG_HDR, "Max fd is <%d>\n", maxfd);
    return;
}

/********************************************/

static void timeout_handler()
{

#undef NAME
#define NAME "timeout_handler()"

    DEBUG (3, DEBUG_HDR, "Called\n");

    siglongjmp(env_alrm, 1);

    return;
}

/********************************************/

static void sigusr1_handler()
{

#undef NAME
#define NAME "sigusr1_handler()"

    DEBUG (3, DEBUG_HDR, "Called\n");
    siglongjmp(env_sigusr1, 1);

    return;
}

/********************************************/

void set_server_timeout(uint32_t value)
{

#undef NAME
#define NAME "set_server_timeout()"

    DEBUG (3, DEBUG_HDR, "Called, value <%d>\n", value);
    timeout_value = value;

    return;
}

/********************************************/

void client_disconnect(const char *name, int fd)
{
	uint32_t i = 0;

#undef NAME
#define NAME "client_disconnect()"

    for (i=0; i<MAX_CLIENTS; i++)
    {
        if (client_fds[i].client_fd == fd)
        {
            break;
        }
    }

    if (i == MAX_CLIENTS)
    {
        LOG ("Closing connection <%s>, <%d>\n", name, fd); 
        for (i=0; i<MAX_FDS; i++)
        {
            if (my_fds[i].fd == fd)
            {
                LOG ("Closing connection <%s>, <%d>\n", name, fd); 
                FD_CLR(my_fds[i].fd, &allset);
                my_fds[i].fd = -1;
                close(fd);
                break;
            }
        }
    }
    else
    {
        LOG ("Closing client connected to <%s> as <%d>\n", name, fd);
        FD_CLR(client_fds[i].client_fd, &allset);
        client_fds[i].client_fd = 0; 
        close(fd);
    }

    return;
}


