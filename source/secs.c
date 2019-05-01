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
#include <stdint.h>
#include <stdbool.h>
#include "include.h"

#define MAX_OPEN_TRANS_TABLE 50  

static uint32_t saved_system_bytes;
static uint32_t system_bytes = 0;

/* Table of opened secs transaction messages, primary messages waiting for replies */
static struct
{
    bool active;
    uint32_t program_idx;
    uint32_t system_bytes;
    uint32_t timer_idx;
    time_t end_time;
} open_trans_table[MAX_OPEN_TRANS_TABLE];

/********************************************/

void open_trans_table_add(uint32_t program_idx, uint32_t system_bytes, uint32_t timer_idx, uint32_t seconds)
{
	uint32_t i = 0;
	time_t t = (time_t)0;

#undef NAME
#define NAME "open_trans_table_add()"

    DEBUG (3, DEBUG_HDR, "Called, program_idx <%d>, system_bytes <%d>, timer_idx <%d>, seconds <%d>\n", 
        program_idx, system_bytes, timer_idx, seconds);

    t = time(NULL);

    /* First clean-up any expired entires */
    for (i=0; i<MAX_OPEN_TRANS_TABLE; i++)
    {
        if (open_trans_table[i].active)
        {
            if (open_trans_table[i].end_time < t)
            {
                DEBUG (3, DEBUG_HDR, "Cleaning-up expired trans entry, index <%d>\n", i);
                open_trans_table[i].active = false;
            }
        }
    }

    /* Look for an empty entry and add this one in */
    for (i=0; i<MAX_OPEN_TRANS_TABLE; i++)
    {
        if (open_trans_table[i].active == false)
        {
            open_trans_table[i].active = true;
            open_trans_table[i].system_bytes = system_bytes;
            open_trans_table[i].program_idx = program_idx;
            open_trans_table[i].timer_idx = timer_idx;
            open_trans_table[i].end_time = t + seconds;
            break;
        }
    }

    if (i == MAX_OPEN_TRANS_TABLE)
    {
        FATAL ("Too many opened SECS transactions, limited to <%d>\n", MAX_OPEN_TRANS_TABLE);
    }

    DEBUG (3, DEBUG_HDR, "Open secs message transaction table\n");
    if (g_debug >= 3)
    {
        for (i=0; i<MAX_OPEN_TRANS_TABLE; i++)
        {
            if (open_trans_table[i].active)
            {
                DEBUG (3, DEBUG_HDR, "  System_bytes <%d>, prog_idx <%d>\n", 
                    open_trans_table[i].system_bytes,
                    open_trans_table[i].program_idx);
            }
        }
    }
}

/********************************************/

uint32_t open_trans_table_del(uint32_t system_bytes, uint32_t *program_idx)
{
	uint32_t i = 0;
	uint32_t iret = 0;
	time_t t = (time_t)0;

#undef NAME
#define NAME "open_trans_table_del()"

    DEBUG (3, DEBUG_HDR, "Called, system_bytes <%d>\n", system_bytes);

    t = time(NULL);

    for (i=0; i<MAX_OPEN_TRANS_TABLE; i++)
    {
        if (open_trans_table[i].system_bytes == system_bytes)
        {
            DEBUG (3, DEBUG_HDR, "Found entry in transaction table\n");
            if (t > open_trans_table[i].end_time)
            {
                ERROR ("Reply transaction arrived too late\n");
                iret = 1;
                break;
            } 

            open_trans_table[i].active = 0;
            *program_idx = open_trans_table[i].program_idx;
            stop_timer(open_trans_table[i].timer_idx);  /* Cancel the timer */
            break;
        }
    }

    if (i == MAX_OPEN_TRANS_TABLE)
    {
        ERROR ("Reply transaction not expected\n");
        iret = 1;
    }

    DEBUG (3, DEBUG_HDR, "Returning <%d>\n", iret);
    return iret;
}

/********************************************/

void secs_calc_checksum(const uint8_t *buffer, uint32_t size, uint8_t *checksum)
{
	uint32_t v = 0;

#undef NAME
#define NAME "secs_calc_checksum()"

    DEBUG (3, DEBUG_HDR, "Called, size <%d>\n", size);

    for (uint32_t i=0; i<size; i++)
    {
        v += buffer[i];
    }

    DEBUG (3, DEBUG_HDR, "Value <%d>\n", v);

    checksum[0] = (uint8_t)((v >> 8) & 255);
    checksum[1] = (uint8_t)(v & 255);

    DEBUG (3, DEBUG_HDR, "Returning <%.2Xh> <%.2Xh>\n", checksum[0], checksum[1]);
    return;
}

/********************************************/

uint32_t secs_get_system_bytes()
{
#undef NAME
#define NAME "secs_get_system_bytes()"

    DEBUG (3, DEBUG_HDR, "Returning, <%d>\n", saved_system_bytes);
    return saved_system_bytes;
}

/********************************************/

void secs_set_system_bytes(uint32_t value)
{
#undef NAME
#define NAME "secs_set_system_bytes()"

    DEBUG (3, DEBUG_HDR, "Called, <%d>\n", value);
    saved_system_bytes = value;

    return;
}

/********************************************/

uint32_t secs_inc_system_bytes()
{
#undef NAME
#define NAME "secs_inc_system_bytes()"

    if (system_bytes == 0)
    {
        /* Init to time() */
        /* system_bytes = (unsigned int) time(NULL); */
        system_bytes = 1;

    }
    else
    {
        system_bytes++;
    }

    DEBUG (3, DEBUG_HDR, "Returning, <%d>\n", system_bytes);
    return system_bytes;
}



