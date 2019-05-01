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

#ifndef COMMANDS_H_
#define COMMANDS_H_


#define SC_CMD_HELP \
    "sc commands:\n" \
    "help                   - Show sc commands\n" \
    "debug <value>          - Set debug level (1 - 5)\n" \
    "dump                   - Dump program data variables (see dump.out)\n" \
    "dump <*var*>           - Dump all matching variables\n" \
    "event <event ...>      - Send <event> to sc\n" \
    "event <label> <arg>    - Jump to label w/ vars\n"\
    "kill                   - Terminate sc\n" \
    "list                   - List program file (see program.list)\n" \
    "load <file>            - Load program into sc (same as read)\n" \
    "logging                - Show logging\n" \
    "logging <name> <value> - Change logging, name is from the open statement\n" \
    "ping                   - Ping the server\n" \
    "read <file>            - Load program into sc (same as load)\n" \
    "tokens                 - Dump program as tokens (see program.tokens)\n" \
    "version                - Show version\n"

#endif /* COMMANDS_H_ */
