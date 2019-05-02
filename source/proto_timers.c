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
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdbool.h>
#include "include.h"

#define MAX_TIMERS 9 

static void timeout_handler();
static uint32_t calc_min_time();

static struct
{
    bool running;
    time_t start;
    time_t end;
    int time;
#ifdef __CYGWIN__
    sigjmp_buf *env_alrm;
#else
    jmp_buf *env_alrm;
#endif    
} timers[MAX_TIMERS]; 

/********************************************/

//int start_proto_timer(int seconds, int idx, sigjmp_buf *env_alrm)
//uint32_t start_proto_timer(uint32_t seconds, uint32_t idx, jmp_buf *env_alrm)
#ifdef __CYGWIN__
uint32_t start_proto_timer(uint32_t seconds, uint32_t idx, sigjmp_buf *env_alrm)
#else
uint32_t start_proto_timer(uint32_t seconds, uint32_t idx, jmp_buf *env_alrm)
#endif
{
	uint32_t min_time = 0;

#undef NAME
#define NAME "start_proto_timer()"

    DEBUG (4, DEBUG_HDR, "Called, seconds <%d>, idx <%d>\n", seconds, idx);

    if (idx >= MAX_TIMERS)
    {
        ERROR ("Invalid idx <%d>, must be from 0 to <%d>\n", idx, MAX_TIMERS-1);
        return 1;
    }

    if (timers[idx].running)
    {
        ERROR ("Timer idx <%d>, already running\n", idx);
        return 1;
    }

    timers[idx].start = time(NULL);
    timers[idx].end = timers[idx].start + seconds;
    timers[idx].running = true;
    timers[idx].env_alrm = env_alrm;

    if (signal(SIGALRM, (void (*) (int)) timeout_handler) == SIG_ERR)
    {
        ERROR ("Error setting signal for SIGALRM\n");
        return 1;
    }

    min_time = calc_min_time();
    DEBUG (4, DEBUG_HDR, "Setting alarm for <%d> seconds\n", min_time);
    alarm(min_time);

    DEBUG (4, DEBUG_HDR, "Returning, success\n");
    return 0;
}

/********************************************/

uint32_t stop_proto_timer(uint32_t idx)
{
	uint32_t iret=0, min_time = 0;

#undef NAME
#define NAME "stop_proto_timer()"

    DEBUG (4, DEBUG_HDR, "Called, timer idx <%d>\n", idx);

    if (idx >= MAX_TIMERS)
    {
        ERROR ("Invalid idx <%d>, must be from 0 to <%d>\n", idx, MAX_TIMERS-1);
        return 1;
    }

    timers[idx].running = false;

    min_time = calc_min_time();
    DEBUG (4, DEBUG_HDR, "Setting alarm for <%d> seconds\n", min_time);
    alarm(min_time);

    DEBUG (4, DEBUG_HDR, "Returning, <%d>\n", iret);
    return 0;
}

/********************************************/

static void timeout_handler()
{
	time_t max = (time_t)0;
	time_t t = (time_t)0;
	int idx = 0;
	uint32_t min_time = 0;

#undef NAME
#define NAME "timeout_handler()"

    DEBUG (4, DEBUG_HDR, "Called\n");

    t = time(NULL);

    max = (time_t)0;
    idx = -1;
    for (int i=0; i<MAX_TIMERS; i++)
    {
        if (timers[i].running)
        {
            DEBUG (4, DEBUG_HDR, "Elasped time, idx <%d>, time <%d>\n", i, t - timers[i].end);
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

    DEBUG (4, DEBUG_HDR, "Max <%d>, idx <%d>\n", max, idx);
    // TODO I think there is a logic error here.  The above code will detect multiple expired timers, but, only the highest numbered one will get reported.
    if (idx >= 0)
    {
        DEBUG (4, DEBUG_HDR, "Timeout detected for timer idx <%d>\n", idx);
        timers[idx].running = false;
        if (signal(SIGALRM, (void (*) (int)) timeout_handler) == SIG_ERR)
        {
             ERROR ("Error setting signal for SIGALRM\n");
             return;
        }

        min_time = calc_min_time();
        DEBUG (4, DEBUG_HDR, "Setting alarm for <%d> seconds\n", min_time);
        alarm(min_time);

        siglongjmp(*timers[idx].env_alrm, 1);
    }

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

    DEBUG (4, DEBUG_HDR, "Called\n");

    t = time(NULL);
    num = 0;
    min_time = 60 * 5;
    for (uint32_t i=0; i<MAX_TIMERS; i++)
    {
        if (timers[i].running)
        {
            DEBUG (4, DEBUG_HDR, "Timer <%d> still active\n", i);
            num++;
            /* Find the timer most over due */
            min_time = (min_time < (timers[i].end - t)) ? min_time : (timers[i].end - t);
        }
    }

    if (num)
    {
    	// TODO see if it is possible for min_time to be negative
        if (min_time <= 0)
        {
            min_time = 1;
        }

    }
    else
    {
        min_time = 0;
    }

    DEBUG (4, DEBUG_HDR, "Returning, <%d>\n", min_time);
    return (uint32_t)min_time;
}

/********************************************/

void init_proto_timers()
{

#undef NAME
#define NAME "init_proto_timers()"

    DEBUG (4, DEBUG_HDR, "Called\n");

    for (uint32_t i=0; i<MAX_TIMERS; i++)
    {
        timers[i].running = false;
    }

    DEBUG (4, DEBUG_HDR, "Returning\n");
    return ;
}

