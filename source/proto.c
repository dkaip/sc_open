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
#include "include.h"

#define MAX_TABLE 100

/* 
   This table is for names and protos, names are unique. 
   Elements are only added, overwritten, and refereced, never deleted 
*/

/********************************************/

enum protos lookup_proto(char *str)
{
enum protos proto;
#undef NAME
#define NAME "lookup_proto()"

    DEBUG (3, DEBUG_HDR, "Called, str <%s>\n", str);
  
    proto = PROTO_UNKNOWN;

    if (strcmp(str, "commands") == 0) {
        proto = PROTO_COMMANDS;

    } else if (strcmp(str, "standard") == 0) {
        proto = PROTO_STANDARD;

    } else if (strcmp(str, "xml") == 0) {
        proto = PROTO_XML;

    } else if (strcmp(str, "hsms") == 0) {
        proto = PROTO_HSMS;

    } else if (strcmp(str, "secs") == 0) {
        proto = PROTO_SECS;

    } else if (strcmp(str, "mbx") == 0) {
        proto = PROTO_MBX;

    } else if (strcmp(str, "mbx_vfei_2.0") == 0) {
        proto = PROTO_MBX_VFEI_20;

    } else {
        ERROR ("Invalid proto <%s>\n", str);
    }

    DEBUG (3, DEBUG_HDR, "Returning, <%d>, <%s>\n", proto, show_proto(proto));
    return proto;
}

/********************************************/

const char *show_proto(enum protos proto)

{
    /* Make them all the same lengh as it looks nicer in the log file */

    if (proto == PROTO_COMMANDS)    return "cmd ";
    if (proto == PROTO_STANDARD)    return "std ";
    if (proto == PROTO_XML)         return "xml ";
    if (proto == PROTO_HSMS)        return "hsms";
    if (proto == PROTO_SECS)        return "secs";
    if (proto == PROTO_MBX)         return "mbx ";
    if (proto == PROTO_MBX_VFEI_20) return "vfei";
    if (proto == PROTO_UNKNOWN)     return "Unknown";

    return "It's really dead Jim";
#ifdef oldcode
    if (proto == PROTO_COMMANDS) return "Command";
    if (proto == PROTO_STANDARD) return "Standard";
    if (proto == PROTO_XML) return "Xml";
    if (proto == PROTO_HSMS) return "Hsms";
    if (proto == PROTO_SECS) return "Secs";
    if (proto == PROTO_MBX) return "Mbx";
    if (proto == PROTO_MBX_VFEI_20) return "Mbx vfei 2.0";
    if (proto == PROTO_UNKNOWN) return "Unknown";
#endif
}

/********************************************/


