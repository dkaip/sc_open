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

#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include "include.h"
#include "externs.h"
#include "globals.h"
#include "version.h"
#include "sc.h"

char *var_prefix[] = {"", "", "", "", "", "#", "#"};
//static char *sc_version = "@(#)" SC_VERSION;
//static char *greeting = "@(#)" SC_GREETING;

static void sigterm_handler();

/********************************************/

int main (int argc, char *argv[])
{
	int i, /* iret,*/ fd;
	/* time_t t, tp; */
	struct buffer buffer = INIT_BUFFER;

#undef NAME
#define NAME "main()"

    if (argc == 1)
    {  /* No arguments passed */
        fprintf(stdout, "%s\n", SC_GREETING);
        fprintf(stdout, "%s\n", SC_HELP);
        exit(0);
    }

    fprintf (stdout, "%s\n", SC_GREETING);

    /* Do all the inits here */    
    init_server();
    init_handler();

    /* Check that all tokens and their types are defined */
    show_all_types(); 

    /* Start parsing the command line */

    i = 1;
    while (i < argc)
    {
        if (strcmp(argv[i], "-h") == 0)
        {
            fprintf(stdout, "%s\n", SC_HELP);
            exit(0);
        }
        else if (strcmp(argv[i], "-g") == 0)
        {
            fprintf(stdout, "%s\n", GNU_GPL_1);
            fprintf(stdout, "%s\n", GNU_GPL_2);
            fprintf(stdout, "%s\n", GNU_GPL_3);
            fprintf(stdout, "%s\n", GNU_GPL_4);
            fprintf(stdout, "%s\n", GNU_GPL_5);
            fprintf(stdout, "%s\n", GNU_GPL_6);
            exit(0);
        }
        else if (strcmp(argv[i], "-i") == 0)
        {
            /* Run in interactive mode */
            fprintf (stdout, "%s\n", SC_INTERACTIVE_GREETING);
            fd = fileno(stdin);
            set_fd_info("stdin", fd, "", PROTO_COMMANDS, -1, NULL);
            set_server(fd, false);
        }
        else if (strcmp(argv[i], "-r") == 0)
        {
            fprintf(stdout, "%s\n", SC_REFERENCE_1);
            fprintf(stdout, "%s\n", SC_REFERENCE_2);
            fprintf(stdout, "%s\n", SC_REFERENCE_3);
            fprintf(stdout, "%s\n", SC_REFERENCE_4);
            fprintf(stdout, "%s\n", SC_REFERENCE_5);
            fprintf(stdout, "%s\n", SC_REFERENCE_6);
            fprintf(stdout, "%s\n", SC_REFERENCE_7);
            fprintf(stdout, "%s\n", SC_REFERENCE_8);
            fprintf(stdout, "%s\n", SC_REFERENCE_9);
            exit(0);
        }
        else if (strcmp(argv[i], "-c") == 0)
        {
            i++;
            if (i >= argc)
            {
                fprintf(stderr, "Missing argument for -c option\n");
                exit(1);
            }
            bufcpy(&buffer, argv[i]);
            commands(&buffer); 
            fd = fileno(stdout);
            my_write("stdout", fd, (const uint8_t *)bufdata(&buffer), bufsize(&buffer));
        }
        else if (strcmp(argv[i], "-n") == 0)
        {
            i++;
            if (i >= argc) { 
                fprintf(stderr, "Missing argument for -n option\n");
                exit(1);
            }
            strncpy(g_name, argv[i], sizeof(g_name)-1);
        }
        else if (strcmp(argv[i], "-s") == 0)
        {
            i++;
            fd = open_socket_server_local(argv[i]);
            if (fd)
            {
                set_server(fd, true);
                set_fd_info(argv[i], fd, "", PROTO_COMMANDS, -1, NULL);
            }
            else
            {
                ERROR ("Can't open socket <%s>\n", argv[i]);
            }
        }
        else if (strcmp(argv[i], "-d") == 0)
        {
            i++;
            if (i >= argc)
            {
                fprintf(stderr, "Missing argument for -d option\n");
                exit(1);
            }
            sscanf(argv[i], "%u", &g_debug);
            debug_init(); 
        }
        else if (strcmp(argv[i], "-x") == 0)
        {
            i++;
            if (i >= argc) { 
                fprintf(stderr, "Missing argument for -x option\n");
                exit(1);
            }
            syntax_check(argv[i]);
        }
        else if (strcmp(argv[i], "-w") == 0)
        {
            fprintf(stdout, "%s\n", NO_WARRANTY);
            exit(0);
        }
        else if (strcmp(argv[i], "-v") == 0)
        {
            fprintf(stdout, "%s\n", SC_HISTORY);
            exit(0);
        }
        else
        {
            fprintf(stderr, "Invalid option %s\n", argv[i]);
            fprintf(stdout, "%s\n", SC_HELP);
            exit(1);
        }
        i++;
    }

    /* Setup for sigterm */
    if (signal(SIGTERM, (void (*) (int)) sigterm_handler) == SIG_ERR)
    {
        FATAL ("Can't setup SIGTERM\n");
    }

    /* Assign any tunning parameters */
    tune();

    server();
    fprintf (stdout, "Exiting\n");

    exit( 0 );
}

/********************************************/

static void sigterm_handler()
{
#undef NAME
#define NAME "sigterm_handler()"

    DEBUG (4, DEBUG_HDR, "Called\n");

    LOG ("Received SIGTERM...\n");

    LOG ("Killing server\n");
    kill_server();

    LOG ("Exiting\n");
    exit (0);
}
