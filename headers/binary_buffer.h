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

/*
 * binary_buffer.h
 *
 *  Created on: Dec 29, 2016
 *      Author: Douglas Kaip
 */

#ifndef BINARY_BUFFER_H_
#define BINARY_BUFFER_H_



#include <stdint.h>

struct binary_buffer
{
    uint8_t *data;
    uint32_t current_in_use;
    uint32_t current_size;
};

#define BINARY_BUFFER_DEFAULT_SIZE 2048
#define INIT_BINARY_BUFFER {(uint8_t *)NULL, 0, 0}


#endif /* BINARY_BUFFER_H_ */
