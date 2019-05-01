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

#ifndef PARSE_H_
#define PARSE_H_

/* Parsed Tokens (PT) */

enum parse_types {
    PT_OTHER=1,
    PT_NUMBER,
    PT_NULL,
    PT_END_OF_LINE,
    PT_NAME,
    PT_STRING,
    PT_CONSTANT,
    PT_POUND_INCLUDE,
    PT_POUND_IFDEF,
    PT_POUND_ELSE,
    PT_POUND_ENDIF,
    PT_POUND_DEFINE,
    PT_COMMENT,
	PT_NOT_ASSIGNED_YET
};

#endif /* PARSE_H_ */
