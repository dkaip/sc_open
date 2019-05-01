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
#include <stdlib.h>
#include "include.h"

/********************************************/

int syntax_check(char *file)
{
//int iret = 0;
struct buffer token = INIT_BUFFER;
#undef NAME
#define NAME "syntax_check()"

    DEBUG (3, DEBUG_HDR, "Called, file <%s>\n", file);

    if (g_debug) {

    } else {
        freopen("/dev/null", "w", stderr);
    }

    g_syntax_check = 1;

//    iret = read_program(file);
    (void)read_program(file);

    set_program_token_idx(0);
//    iret = execute(&token);
    (void)execute(&token);

    /*
    printf ("PASS 2\n");
    g_syntax_check++;
    set_program_token_idx(0);
    iret = execute(&token);
    */

    fprintf (stdout, "Syntax check done\n");
    buffree(&token);

    exit(0);

}


