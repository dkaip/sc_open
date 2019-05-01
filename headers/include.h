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

#ifndef INCLUDE_H_
#define INCLUDE_H_


#include <stdint.h>

#include "buffer.h"
#include "commands.h"
#include "debug.h"
#include "error.h"
#include "pp_error.h"
#include "warning.h"
#include "fatal.h"
#include "log.h"
#include "parse.h"
#include "proto.h"
#include "sc.h"
#include "secs.h"
#include "hsms.h"
#include "server.h"
#include "version.h"
#include "tokens.h"
#include "variables.h"
#include "reference.h"
#include "externs.h"
#include "trace.h"
#include "pragma.h"
#include "binary_buffer.h"


union I2Union
{
	int16_t the_int16;
	uint8_t the_bytes[sizeof(int16_t)];
};

union U2Union
{
	uint16_t the_uint16;
	uint8_t  the_bytes[sizeof(uint16_t)];
};

union I4Union
{
	int32_t the_int32;
	uint8_t the_bytes[sizeof(int32_t)];
};

union U4Union
{
	uint32_t the_uint32;
	uint8_t  the_bytes[sizeof(uint32_t)];
};

union I8Union
{
	int64_t the_int64;
	uint8_t the_bytes[sizeof(int64_t)];
};

union U8Union
{
	uint64_t the_uint64;
	uint8_t  the_bytes[sizeof(uint64_t)];
};

union F4Union
{
	float the_float;
	uint8_t the_bytes[sizeof(float)];
};

union F8Union
{
	double the_double;
	uint8_t the_bytes[sizeof(double)];
};

#endif /* INCLUDE_H_ */

