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

#include <termios.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include "include.h"

/********************************************/

uint32_t open_port(const char *name, const char *device, enum protos proto, const char *options, int *ret_fd)
{
	uint32_t iret = 0;
	int fd = 0;
	int value;
	struct termios buf;
	speed_t baud = (speed_t)0;

#undef NAME
#define NAME "open_port()"

    DEBUG (3, DEBUG_HDR, "Called\n");

    while (1)
    {
        DEBUG (3, DEBUG_HDR, "Opening port <%s>\n", device);
        if ((fd = open(device, O_RDWR)) == -1)
        {
            ERROR ("Can't open device <%s>\n", device);
            iret = 1;
            break;
        } 

        LOG ("Port opened, name <%s>, fd <%d>, device <%s>, proto <%d> <%s>, options <%s>\n", 
            name, fd, device, proto, show_proto(proto), options);

        DEBUG (3, DEBUG_HDR, "Port opened\n");

        if (tcgetattr(fd, &buf) < 0)
        {
            ERROR ("Can't get port attributes <%s>\n", device);
            iret = 1;
            break;
        } 

        buf.c_lflag &= (tcflag_t)~(ECHO | ICANON | IEXTEN | ISIG);
        buf.c_iflag &= (tcflag_t)~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
        buf.c_cflag &= (tcflag_t)~(CSIZE | PARENB);
        buf.c_cflag |= (tcflag_t)(CS8 | CLOCAL);
        buf.c_oflag &= (tcflag_t)~(OPOST);
        buf.c_cc[VMIN] = 1;
        buf.c_cc[VTIME] = 0;

        if (tcsetattr(fd, TCSAFLUSH, &buf) < 0)
        {
            ERROR ("Can't set port attributes <%s>\n", device);
            iret = 1;
            break;
        }

        value = atoi(get_option("baud=", name));
 
        if (value == 300)
        {
            baud = B300;
        }
        else if (value == 600)
        {
            baud = B600;
        }
        else if (value == 1200)
        {
            baud = B1200;
        }
        else if (value == 2400)
        {
            baud = B2400;
        }
        else if (value == 4800)
        {
            baud = B4800;
        }
        else if (value == 9600)
        {
            baud = B9600;
        }
        // Added 1/2/17
        else if (value == 19200)
        {
            baud = B19200;
        }
        else if (value == 38400)
        {
            baud = B38400;
        }
        else
        {
            ERROR ("Invalid baud value <%s>\n", value);
            iret = 1;
            break;
        }
    
        if (cfsetispeed(&buf, baud) < 0)
        {
            ERROR ("Can't set io speed for port <%s>\n", device);
            iret = 1;
            break;
        }

        if (cfsetospeed(&buf, baud) < 0)
        {
            ERROR ("Can't set io speed for port <%s>\n", device);
            iret = 1;
            break;
        }

        if (tcsetattr(fd, TCSAFLUSH, &buf) < 0)
        {
            ERROR ("Can't set io speed for port <%s>\n", device);
            iret = 1;
            break;
        }
        break;
    }

    *ret_fd = fd;

    DEBUG (3, DEBUG_HDR, "Returning, fd <%d>, error <%d>\n", *ret_fd, iret);
    return iret;
}

/********************************************/

