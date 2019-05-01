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

#include <string.h>
#include <stdint.h>
#include "include.h"

/********************************************/

void swap(uint8_t *data, uint32_t size)
{
//	char *installed = (char *)NULL;

#undef NAME
#define NAME "swap()"

	if ((*(uint16_t *)"\0\xff" < 0x100)) // if BIG ENDIAN
	{
//	    installed = "@(#)File " __FILE__ " byte swap disabled";
	}
	else
	{
		// We're on a LITTLE ENDIAN machine
		uint8_t tmp[64];
//		installed = "@(#)File " __FILE__ " byte swap enabled";

		DEBUG (3, DEBUG_HDR, "Called, size <%d>\n", size);

		DEBUG (3, DEBUG_HDR, "Data before swap: \n");
		for (uint32_t i=0; i<size; i++)
		{
			DEBUG (3, DEBUG_HDR, "  Data <%.2Xh>\n", data[i]);
		}

		for (uint32_t i=0; i<size; i++)
		{
			tmp[size-i-1] = data[i];
		}

		(void)memcpy(data, tmp, size);

		DEBUG (3, DEBUG_HDR, "Data after swap: \n");
		for (uint32_t i=0; i<size; i++)
		{
			DEBUG (3, DEBUG_HDR, "  Data <%.2Xh>\n", data[i]);
		}
		}

    return;
}


