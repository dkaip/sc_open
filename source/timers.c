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
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "include.h"

#define MAX_TIMERS 50 
#define SIZE 128

static uint32_t calc_min_time();

static struct
{
    bool active;
    time_t start;
    time_t end;
//    int time;   hmmm does not seem to be used anywhere
    uint32_t program_idx;
    char msg[SIZE];
    char comment[512];  /* Only used for logging */
} timers[MAX_TIMERS]; 

/********************************************/

uint32_t cmd_start_timer()
{
	uint32_t iret = 0;
	time_t seconds = 0;
	int idx = 0;
	uint32_t num = 0;
	uint32_t program_idx = 0;
	char msg[SIZE], comment[SIZE], handler[32];
	struct buffer token = INIT_BUFFER;
	struct buffer valu = INIT_BUFFER;
	enum token_types type = TT_NOT_ASSIGNED_YET;

#undef NAME
#define NAME "cmd_start_timer()"

    DEBUG (3, DEBUG_HDR, "Called\n");

    idx = 0;
    seconds = 0;
    iret = 0;
    memset(msg, '\0', sizeof(msg));
    memset(comment, '\0', sizeof(comment));

    while (1)
    {
        get_program_token(&token, &type);    
        DEBUG (3, DEBUG_HDR, "Token <%s>, type <%s>\n", bufdata(&token), show_type(type));
        if ((type == TT_END_OF_LINE) || (type == TT_NULL_TOKEN))
        {
            break;
        }
        else if (bufcmp(&token, "timer") == 0)
        {
            get_program_token(&token, &type);    
            if (type == TT_ASSIGN)
            {
                get_program_token(&token, &type);    
                idx = atoi(bufdata(&token));
                if ((idx >= 0) && (idx <= 9))
                {
                	;
                }
                else
                {
                    ERROR ("Invalid timer value.. must be 0 to 9\n");
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
        else if (bufcmp(&token, "seconds") == 0)
        {
            get_program_token(&token, &type);    
            if (type == TT_ASSIGN)
            {
                get_program_token(&token, &type);    
                seconds = atoi(bufdata(&token));
            }
            else
            {
                put_program_token(); 
                ERROR ("Expecting \"=\", found <%s>\n", bufdata(&token));
                iret = 1; 
                break;
            }
        }
        else if (bufcmp(&token, "handler") == 0)
        {
            get_program_token(&token, &type);    
            if (type == TT_ASSIGN)
            {
                get_program_token(&token, &type);    
                strncpy(handler, bufdata(&token), sizeof(handler) - 1);
            }
            else
            {
                put_program_token(); 
                ERROR ("Expecting \"=\", found <%s>\n", bufdata(&token));
                iret = 1; 
                break;
            }

            /* sprintf(comment, "Timer <%d> started at line <%d> timed-out, jumping to label <%s>",
                idx, get_line_number(), bufdata(&token)); */

            /* strncpy(comment, bufdata(&token), sizeof(comment)); */

            if (variable_get(bufdata(&token), &valu, VT_LABEL))
            {
                ERROR ("Label <%s> not found\n", bufdata(&token));
            }
            else
            {
                sscanf(bufdata(&valu), "%u", &num);
                program_idx = num;
            }
        }
        else if (bufcmp(&token, "msg") == 0)
        {
            get_program_token(&token, &type);    
            if (type == TT_ASSIGN)
            {
                get_program_token(&token, &type);    
            }
            else
            {
                put_program_token(); 
                ERROR ("Expecting \"=\", found <%s>\n", bufdata(&token));
                iret = 1; 
                break;
            }

            if (strlen(bufdata(&token)) >= sizeof(msg))
            {
                ERROR ("Msg <%s> is too long, limited to <%d>\n", sizeof(msg));
                iret = 1;
                break;
            }
            else
            {
                strcpy(msg, bufdata(&token));
            }
        }
    }

    if (iret)
    {
    	;
    }
    else
    {
        sprintf(comment, "Timer <%d> started at line <%s> has timed-out, jumping to label <%s>",
            idx, get_line_number(), handler); 
//        idx = start_timer(idx, seconds, program_idx, msg, comment);
        (void)start_timer(idx, seconds, program_idx, msg, comment);
    }

    buffree(&token);
    buffree(&valu);
    DEBUG (3, DEBUG_HDR, "Returning iret <%d>\n", iret); 
    return iret;
}

/********************************************/

uint32_t start_timer(int timer_idx, time_t seconds, uint32_t program_idx, const char *msg, const char *comment)
{
	uint32_t i = 0;

#undef NAME
#define NAME "start_timer()"

    DEBUG (3, DEBUG_HDR, "Called\n");
    DEBUG (3, DEBUG_HDR, "Called idx <%d>, seconds <%d>, program_idx <%u>, msg <%s>\n",
    		timer_idx, seconds, program_idx, msg);

    /* Only timers 0 to 9 can be specified directly */

    if ((timer_idx == -1) || ((timer_idx >= 0) && (timer_idx < 10)))
    {
        /* Idx value OK */
    	;
    }
    else
    {
        FATAL ("Invalid idx value <%d>\n", timer_idx);
    }

    /* idx of -1 means assign next available timer */

    if (timer_idx)
    {
        for (i=10; i<MAX_TIMERS; i++)
        {
            if (timers[i].active == false)
            {
            	timer_idx = (int)i;
                break;
            }
        }

        if (i == MAX_TIMERS)
        {
            FATAL ("No more timers available\n");
        }
    }
    DEBUG (3, DEBUG_HDR, "Using idx <%d>\n", timer_idx);

    timers[timer_idx].start = time(NULL);
    timers[timer_idx].end = timers[timer_idx].start + seconds;
    timers[timer_idx].program_idx = program_idx;

    if (strlen(msg) >= sizeof(timers[timer_idx].msg))
    {
        FATAL ("Msg <%s> is too long, limited to <%d>\n", sizeof(timers[timer_idx].msg));

    }
    else
    {
        strcpy(timers[timer_idx].msg, msg);
    }

    strncpy(timers[timer_idx].comment, comment, sizeof(timers[timer_idx].comment));

    timers[timer_idx].active = true;
    set_server_timeout(calc_min_time());
   
    DEBUG (3, DEBUG_HDR, "Returning idx <%d>\n", timer_idx);
    return (uint32_t)timer_idx;
}

/********************************************/

uint32_t cmd_stop_timer()
{
	uint32_t iret = 0;
	int idx = 0;
	struct buffer token = INIT_BUFFER;
	enum token_types type = TT_NOT_ASSIGNED_YET;

#undef NAME
#define NAME "cmd_stop_timer()"

    DEBUG (3, DEBUG_HDR, "Called\n");

    idx = 0;
    iret = 0;

    while (1) {
        get_program_token(&token, &type);    
        DEBUG (3, DEBUG_HDR, "Token <%s>, type <%s>\n", bufdata(&token), show_type(type));
        if ((type == TT_END_OF_LINE) || (type == TT_NULL_TOKEN))
        {
            break;
        }
        else if (bufcmp(&token, "timer") == 0)
        {
            get_program_token(&token, &type);    
            if (type == TT_ASSIGN)
            {
                get_program_token(&token, &type);    
                idx = atoi(bufdata(&token));
                if ((idx >= 0) && (idx < MAX_TIMERS))
                {
                	;
                }
                else
                {
                    ERROR ("Invalid idx value.. must be 0 to <%d>\n", MAX_TIMERS);
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
    }

    DEBUG (3, DEBUG_HDR, "Iret <%d>, idx <%d>\n", iret, idx);

    if (iret)
    {
    	;
    }
    else
    {
        DEBUG (3, DEBUG_HDR, "Stopping timer, idx <%d>\n", idx);
        stop_timer((uint32_t)idx);
    }
    
    buffree(&token);
    DEBUG (3, DEBUG_HDR, "Returning, error <%d>\n", iret);
    return iret;
}

/********************************************/

void stop_timer(uint32_t idx)
{
#undef NAME
#define NAME "stop_timer()"

    DEBUG (3, DEBUG_HDR, "Called, idx <%d>\n", idx);

    DEBUG (3, DEBUG_HDR, "Stopping timer, idx <%d>\n", idx);
    timers[idx].active = false;

    DEBUG (3, DEBUG_HDR, "Returning\n");
    return;
}

/********************************************/

static uint32_t calc_min_time()
{
	time_t min_time = (time_t)0;
	uint32_t num = 0;
	time_t t = (time_t)0;

#undef NAME
#define NAME "calc_min_time()"

    DEBUG (3, DEBUG_HDR, "Called\n");

    t = time(NULL);
    num = 0;
    min_time = 60 * 5;
    for (uint32_t i=0; i<MAX_TIMERS; i++)
    {
        if (timers[i].active == true)
        {
            DEBUG (3, DEBUG_HDR, "Timer <%d> still active\n", i);
            num++;
            /* Find the timer most over due */
            min_time = (min_time < (timers[i].end - t)) ? min_time : (timers[i].end - t);
        }
    }

    if (num)
    {
        if (min_time <= 0)
        {
            min_time = 1;
        }
    }
    else
    {
        min_time = 0;
    }

    DEBUG (3, DEBUG_HDR, "Returning, <%d>\n", min_time);
    return (uint32_t)min_time;
}

/********************************************/

void timer_handler() 
{
	time_t t = (time_t)0;
	time_t max = (time_t)0;
	int idx, i;
	uint32_t iret = 0;
struct buffer valu = INIT_BUFFER;
struct buffer token = INIT_BUFFER;
#undef NAME
#define NAME "timer_handler()"

    DEBUG (3, DEBUG_HDR, "Called\n");

    t = time(NULL);

    max = 0;
    idx = -1;

    /* Determine which timer expired */
    for (i=0; i<MAX_TIMERS; i++)
    {
        if (timers[i].active == true)
        {
            DEBUG (3, DEBUG_HDR, "Elasped time, idx <%d>, time <%d>\n", i, t - timers[i].end);
            if (t >= timers[i].end)
            {
                if (max <= (t - timers[i].end))
                {
                    max = t - timers[i].end;
                    idx = i;
                }
            }
        }
    }
    DEBUG (3, DEBUG_HDR, "Max <%d>, idx <%d>\n", max, idx);

    if (idx >= 0)
    {
        DEBUG (3, DEBUG_HDR, "Timeout detected for timer idx <%d>\n", idx);
        timers[idx].active = false;

        LOG ("%s, msg <%s>\n", timers[idx].comment, timers[idx].msg);

        DEBUG (3, DEBUG_HDR, "Setting token pointer\n");

//It looks like program_idx cannot be set less than zero
//        if (timers[idx].program_idx == -1)
//        {
//            /* No where to go to */
//
//        }
//        else
//        {
            make_tmp_variables(timers[idx].msg, "", 1);
            iret = resume_via_program_idx(timers[idx].program_idx);
            if (iret)
            {
                ERROR ("Error while processing timer event\n");
            }
//        }
    }
    else
    {
        DEBUG (3, DEBUG_HDR, "No timers expired\n");
    }

    /* Setup for the next time-out period */
    set_server_timeout(calc_min_time());

    buffree(&valu);
    buffree(&token);
    return;
}

/********************************************/

