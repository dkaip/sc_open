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

#ifndef SECS_H_
#define SECS_H_

#include <stdint.h>

#define LIST        (uint8_t)0x00
#define ASCII       (uint8_t)0x40
#define BINARY      (uint8_t)0x20
#define BOOLEAN     (uint8_t)0x24
#define UNSIGNED_1  (uint8_t)0xA4
#define UNSIGNED_2  (uint8_t)0xA8
#define UNSIGNED_4  (uint8_t)0xB0
#define UNSIGNED_8  (uint8_t)0xA0
#define SIGNED_1    (uint8_t)0x64
#define SIGNED_2    (uint8_t)0x68
#define SIGNED_4    (uint8_t)0x70
#define SIGNED_8    (uint8_t)0x60
#define FLOAT_4     (uint8_t)0x90
#define FLOAT_8     (uint8_t)0x80

#define ENQ (uint8_t)0x05
#define EOT (uint8_t)0x04
#define ACK (uint8_t)0x06
#define NAK (uint8_t)0x15

#define DEFAULT_T1 10
#define DEFAULT_T2 10
#define DEFAULT_T3 45 
#define DEFAULT_T4 45 
#define DEFAULT_BAUD 9600
#define DEFAULT_RETRIES 3 
#define DEFAULT_RBIT 0 

#endif /* SECS_H_ */

