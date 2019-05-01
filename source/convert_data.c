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
#include <ctype.h>
#include <stdint.h>
#include "include.h"

/********************************************/

void data_to_char(struct buffer *buf_out, const char *buf_in, uint32_t cnt)
{
	char s[64];
	char c = 0;

#undef NAME
#define NAME "data_to_char()"

    DEBUG (3, DEBUG_HDR, "Called, cnt <%d>\n", cnt);

    /* Replace non-printing chars with ^x */
    for (uint32_t i=0; i<cnt; i++)
    {
        c = buf_in[i];
        if (isprint(c))
        {
            bufncat(buf_out, &c, 1);
        }
        else
        {
            if (c == '\n')
            {
               bufcat(buf_out, "\\n");
            }
            else if (c == '\r')
            {
               bufcat(buf_out, "\\r");
            }
            else
            {
                sprintf(s, "\\x%2.2x", (uint8_t)c);
                bufcat(buf_out, s);
            }
        }
    }

    DEBUG (3, DEBUG_HDR, "Returning, buffer out <%s>\n", bufdata(buf_out));
    return;
}

/********************************************/

void char_to_data(struct buffer *buf_out, const char *buf_in)
{
	uint32_t i = 0;
	char v = 0;

#undef NAME
#define NAME "char_to_data()"

    DEBUG (3, DEBUG_HDR, "Called, buffer <%s>\n", buf_in);

    i = 0;
    uint32_t in_buf_length = (uint32_t)strlen(buf_in);
    while (i < in_buf_length)
    {
        if ((buf_in[i] == '\\') && (buf_in[i+1] == 'n'))
        {
            v = '\n';
            bufncat(buf_out, (char *) &v, 1);
            i = i + 2;
            continue;
        }
        else if ((buf_in[i] == '\\') && (buf_in[i+1] == 'r'))
        {
            v = '\r';
            bufncat(buf_out, (char *) &v, 1);
            i = i + 2;
            continue;
        }
        else if ((buf_in[i] == '\\') && (buf_in[i+1] == 'x'))
        {
        	uint32_t iv = 0;
        	char sbuf[16];
            strncpy(sbuf, &buf_in[i+2], 2);
            sscanf(sbuf, "%x", &iv);
            v = (char)iv;
            i = i + 4;
            bufncat(buf_out, &v, 1);
            continue;
        }
        else
        {
            bufncat(buf_out, &buf_in[i], 1);
        }

        i++;
    }
    DEBUG (3, DEBUG_HDR, "Returning\n");
    return;
}

/********************************************/


#ifdef oldcode
main()
{
int iret;
int i;
short v;
char buf[128], *ptr, sbuf[16];

    printf ("Starting\n");

    strcpy(buf, "Hello");
    strcat(buf, "\\xFF");
    strcat(buf, "\\x00");
    strcat(buf, "\\x0F");
    strcat(buf, "goodbye");

    printf ("buf <%s>\n", buf);

    i = 0;
    while (i < strlen(buf)) {
        if ((buf[i] == '\\') && (buf[i+1] == 'x')) {
            printf ("Found back slash x\n");
            strncpy(sbuf, &buf[i+2], 2);
            printf ("sbuf <%s>\n", sbuf);
            sscanf(sbuf, "%hx", &v);
            printf ("Value <%d>\n", v);
            i = i + 4;
            continue;

        } else {
            printf ("Char <%c>\n", buf[i]);
        }
        i++;
    }

    printf ("Done\n");

}
#endif
