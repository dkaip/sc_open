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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "include.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))


/********************************************/

void buffree(struct buffer *buf)
{
#undef NAME
#define NAME "buffree()"

    if (buf->data != (char *)NULL)
    {
        DEBUG (5, DEBUG_HDR, "Freeing all space for buffer\n");
        free(buf->data);
        buf->data = (char *)NULL;
    }

    buf->index = 0;
    buf->size = DEFAULT_SIZE; 
}

/********************************************/

void bufprefix(struct buffer *buf, const char *prefix)
{
	struct buffer x = INIT_BUFFER;
#undef NAME
#define NAME "bufprefix()"

    DEBUG (5, DEBUG_HDR, "Called\n");

    bufcpy(&x, prefix);
    bufcat(&x, bufdata(buf));
    bufcpy(buf, bufdata(&x));

    buffree(&x);
    return;

}

/********************************************/

void bufncat(struct buffer *buf, const char *data, uint32_t len)
{

#undef NAME
#define NAME "bufncat()"

    DEBUG (5, DEBUG_HDR, "Called, data len <%d>\n", len);
    DEBUG (5, DEBUG_HDR, "Available space <%d>\n", (buf->size-buf->index));

    if (buf->data == (char *)NULL)
    {
    	size_t length_to_malloc = MAX((len + 1), buf->size);
        buf->data = (char *)malloc(length_to_malloc);
        if (buf->data == NULL)
        {
            ERROR ("Can't malloc more space, exiting\n");
            exit (1);
        }
        buf->size = (uint32_t)length_to_malloc;
        buf->data[0] = 0;
    }

    if ((buf->index+len+1) > buf->size)
    {
    	size_t length_to_realloc = MAX((buf->size + DEFAULT_SIZE), (buf->index+len+1));;
        DEBUG (5, DEBUG_HDR, "New size <%d>\n", length_to_realloc);
        buf->data = (char *)realloc(buf->data, length_to_realloc);
        if (buf->data == (char *)NULL)
        {
            ERROR ("Can't realloc more space, exiting\n");
            exit (1);
        }
        buf->size = (uint32_t)length_to_realloc;
    }

    memcpy(&buf->data[buf->index], data, len);
    buf->index += len;
    buf->data[buf->index] = 0;

    DEBUG (5, DEBUG_HDR, "Remaining space <%d>\n", (buf->size-buf->index));
    return;
}

/********************************************/

void bufcat(struct buffer *buf, const char *data)
{
#undef NAME
#define NAME "bufcat()"

    DEBUG (5, DEBUG_HDR, "Called, data <%s>\n", data);

    bufncat(buf, data, (uint32_t)strlen(data));
    return;
}

/********************************************/

void bufcpy(struct buffer *buf, const char *data)
{
#undef NAME
#define NAME "bufcpy()"

    DEBUG (5, DEBUG_HDR, "Called, data <%s>\n", data);

    buf->index = 0;
    bufncat(buf, data, (uint32_t)strlen(data));
    return;
}

/********************************************/

void bufncpy(struct buffer *buf, const char *data, uint32_t len)
{
#undef NAME
#define NAME "bufncpy()"

    DEBUG (5, DEBUG_HDR, "Called, data len <%d>\n", len);

    buf->index = 0;
    bufncat(buf, data, len);
    return;
}

/********************************************/

void buf2bufcpy(struct buffer *buf1, const struct buffer *buf2)
{
#undef NAME
#define NAME "buf2buf()"

    /* Copy buf2 to buf1 */
    buf1->index = 0;
    bufncat(buf1, bufdata(buf2), bufsize(buf2));
    return;
}

/********************************************/

void bufnlcpy(struct buffer *buf, const char *data)
{
#undef NAME
#define NAME "bufnlcpy()"

    DEBUG (5, DEBUG_HDR, "Called, data <%s>\n", data);

    bufcpy(buf, "");
    while (1)
    {
        if (*data == '\n')
        {
            break;
        }
        bufncat(buf, data, 1);    
        data++;
    }    
    return;
}

/********************************************/

inline char *bufdata(const struct buffer *buf)
{
    return(buf->data);
}

/********************************************/

inline uint32_t bufsize(const struct buffer *buf)
{
    return(buf->index);
}

/********************************************/

uint32_t buffilecpyread(struct buffer *buf, const char *file)
{
	char tmp[1024];
	char *ret = (char *)NULL;
	FILE *fd = (FILE *)NULL;

#undef NAME
#define NAME "buffilecpyread()"

    DEBUG (3, DEBUG_HDR, "Called, file <%s>\n", file);


    if ((fd = fopen(file, "r")) == (FILE *)NULL)
    {
        ERROR ("Can't open file <%s>\n", file);
        return 1;
    }

    bufcpy(buf, "");
    while (1)
    {
        ret = fgets(tmp, sizeof(tmp), fd);
        if (ret == (char *)NULL)
        {
            break;
        }

        /* DEBUG (5, DEBUG_HDR, "Read <%s>\n", tmp); */
        bufcat(buf, tmp);

    }

    fclose(fd);
    return 0;
}

/********************************************/

int buffilecpywrite(const struct buffer *buf, const char *file)
{
	FILE *fd = (FILE *)NULL;

#undef NAME
#define NAME "buffilecpywrite()"

    DEBUG (5, DEBUG_HDR, "Called\n");


    if ((fd = fopen(file, "w")) == NULL)
    {
        ERROR ("Can't open file <%s>\n", file);
        return 1;
    }

    /* iret = fputs(bufdata(buf), fd); */
    (void)fputs(bufmkprint(buf), fd);
    
    fclose(fd);
    return 0;
}

/********************************************/

int bufcmp(const struct buffer *buf, const char *str)
{

#undef NAME
#define NAME "bufcmp()"

    DEBUG (5, DEBUG_HDR, "Called, comparing %s to %s\n", bufdata(buf), str);

    return strcmp(bufdata(buf), str);

}

/********************************************/

int bufncmp(const struct buffer *buf, const char *str, uint32_t n)
{

#undef NAME
#define NAME "bufncmp()"

    DEBUG (5, DEBUG_HDR, "Called, comparing %s to %s\n", bufdata(buf), str);

    return strncmp(bufdata(buf), str, n);

}

/********************************************/

void bufrtrim(struct buffer *buf)
{
	char *p = (char *)NULL;

#undef NAME
#define NAME "bufrtrim()"

    DEBUG (3, DEBUG_HDR, "Called <%s>, size <%d>\n", &buf->data[0], buf->size);

    p = strchr(&buf->data[0], '\0');       

    while (p--)
    {
    	DEBUG (3, DEBUG_HDR, "<%c>\n", *p);
        if (*p == ' ')
        {
            *p = '\0';
        }
        else
        {
            break;
        }
    }

    DEBUG (3, DEBUG_HDR, "Returning <%s>\n", &buf->data[0]);
}

/********************************************/

void buftoupper(struct buffer *buf)
{

#undef NAME
#define NAME "buftoupper()"

    for (uint32_t i=0; i<buf->size; i++)
    {
        buf->data[i] = (char)toupper(buf->data[i]);
    }
}

/********************************************/

void buftolower(struct buffer *buf)
{

#undef NAME
#define NAME "buftolower()"

    /* printf("\n"); */
    for (uint32_t i=0; i<buf->size; i++)
    {
        buf->data[i] = (char)tolower(buf->data[i]);
    }
}

/********************************************/
// TODO  I don't like this returning a static
const char *bufmkprint(const struct buffer *buf)
{
	static struct buffer tmp_buf = INIT_BUFFER;
	char *p = (char *)NULL;
	char tmp[32];

#undef NAME
#define NAME "bufmkprint()"

    /* If you change this, check strmkprint() below */

    bufcpy(&tmp_buf, "");

    p = bufdata(buf);
    for (uint32_t i=0; i<bufsize(buf); i++)
    {
        if (isprint(*p) || isspace(*p))
        {
            bufncat(&tmp_buf, p, 1);
        }
        else
        {
            if (is_pragma(PRAGMA_SHOW_NON_PRINTING_CHARS_IN_BRACES))
            {
                sprintf(tmp, "{%2.2x}", (unsigned char) *p);

            }
            else
            {
                sprintf(tmp, "\\x%2.2x", (unsigned char) *p); 
            }
            bufcat(&tmp_buf, tmp);
        }
        p++; 
    }

    return bufdata(&tmp_buf);
}

/********************************************/

// TODO  I don't like this returning a static
const char *strmkprint(const char *str, uint32_t len)
{
	static struct buffer buf = INIT_BUFFER;
	char tmp[32];

#undef NAME
#define NAME "strmkprint()"

    /* If you change this, check bufmkprint() above */

    bufcpy(&buf, "");

    for (uint32_t i=0; i<len; i++)
    {
        if (isprint(*str) || isspace(*str))
        {
            bufncat(&buf, str, 1);
        }
        else
        {
            if (is_pragma(PRAGMA_SHOW_NON_PRINTING_CHARS_IN_BRACES))
            {
                sprintf(tmp, "{%2.2x}", (unsigned char) *str);
            }
            else
            {
                sprintf(tmp, "\\x%2.2x", (unsigned char) *str); 
            }
            bufcat(&buf, tmp);
        }
        str++; 
    }

    return bufdata(&buf);
}

/********************************************/

//void bufdump_fmt(struct buffer *buf, struct buffer *fmt)
//{
//	uint32_t i = 0;
//	uint32_t size = 0;
//	char *b = (char *)NULL;
//	char tmp[80];
//	char num[10];
//#undef NAME
//#define NAME "bufdump_fmt()"
//
//    bufcpy(fmt, "");
//
//    b = bufdata(buf);
//    size = bufsize(buf);
//    i = 0;
//    memset(tmp, ' ', sizeof(tmp));
//    tmp[sizeof(tmp)-1] = '\0';
//
//    while (size)
//    {
//        if (i == 16)
//        {
//            bufcat(fmt, tmp);
//            bufcat(fmt, "\n");
//            i = 0;
//            memset(tmp, ' ', sizeof(tmp));
//            tmp[sizeof(tmp)-1] = '\0';
//            continue;
//
//        }
//        else
//        {
//            sprintf(num, "%.2X", (unsigned char) *b);
//            memcpy(&tmp[i*3], num, 2);
//            if (isprint((unsigned char) *b))
//            {
//                tmp[i+60] = *b;
//            }
//            else
//            {
//                tmp[i+60] = '.';
//            }
//        }
//
//        i++;
//        b++;
//        size--;
//    }
//
//    bufcat(fmt, tmp);
//    bufcat(fmt, "\n");
//}
