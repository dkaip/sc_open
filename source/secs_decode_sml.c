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
#include <stdbool.h>

#include "include.h"

/********************************************/

uint32_t secs_decode(const char *prefix, const struct binary_buffer *buffer, struct buffer *log)
{
	uint32_t iret = 0;
	bool array = false;
	uint32_t size = 0;
	uint32_t num_length_bits = 0;
	uint32_t length = 0;
	uint32_t array_size = 0;
	uint32_t secs_data_type = 0;
	uint32_t l, e[100], ie[100]/*, jnum, jsize*/;
	uint8_t *p = (uint8_t *)NULL;
	uint8_t *tmp_p = (uint8_t *)NULL;
	const char *tmp_ptr = (char *)NULL;
	char num[64];
	uint8_t b;
	int8_t i1 = 0;
	uint8_t u1 = 0;
	union I2Union i2_union;
	union U2Union u2_union;
	union I4Union i4_union;
	union U4Union u4_union;
	union I8Union i8_union;
	union U8Union u8_union;
	union F4Union f4_union;
	union F8Union f8_union;

char pad[256], base_name[256], name[256]/*, fmt[256]*/;
/*char suffix[100];*/
struct buffer value = INIT_BUFFER;
#undef NAME
#define NAME "secs_decode()"

    DEBUG (3, DEBUG_HDR, "Called\n");

    for (int i=0; i<100; i++)
    {
        e[i] = ie[i] = 0;
    }

    l = 0;
    p = binary_buffer_data(buffer);
    length = binary_buffer_num_data(buffer);

    while (length)
    {
        secs_data_type = *p & 0xFC;
        num_length_bits = *p & 0x03;
        array = false;
        array_size = 0;
        p++;
        length--;

//        size = 0;
//        for (int i=0; i<num_length_bits; i++)
//        {
//            size = (size * 256) + (unsigned char) *p;
//            p++;
//            len--;
//        }

        /*
         * If the number of length bits is not equal to 1, 2, or 3 we have
         * major problem.  The standard says that it can only be a 1, 2, or 3.
         */
        size = 0x00000000;
        if (num_length_bits == 1)
        {
        	size = p[0];
        	length--;
        	p++;
        }
        else if (num_length_bits == 2)
        {
        	size = ((uint32_t)(p[0] << 8) | p[1]);
        	length -= 2;
        	p += 2;
        }
        else
        {
        	size = ((uint32_t)(p[0] << 16) | (uint32_t)(p[1] << 8) | p[2]);
        	length -= 3;
        	p += 3;
        }

        DEBUG (3, DEBUG_HDR, "Type <%.2Xh>, num_length_bits <%d>, size <%d>, len <%d>\n",
            secs_data_type, num_length_bits, size, length);

        ie[l]++;

        (void)strcpy(base_name, prefix);
        for(uint32_t i=0; i<=l; i++)
        {
            strcat(base_name, ".");
            /* sprintf(num, "%d", ie[i]); */
            /* strcat(base_name, num); */
            strcat(base_name, my_itoa(ie[i])); 
        }
        /* sprintf(fmt, "%%-%ds", (l+1)*3 + 30); */
        /* printf("????? fmt <%s>\n", fmt); */
        /* sprintf(pad, fmt, base_name); */
        /* memset(pad, '\0', sizeof(pad)); */
        memset(pad, ' ', (l+1)*3 + 30);
        pad[(l+1)*3 + 30] = '\0';
        memcpy(pad, base_name, strlen(base_name));

        if (secs_data_type == LIST)
        {
            DEBUG (3, DEBUG_HDR, "List\n"); 
            bufcat(log, pad);
            bufcat(log, "<L");
            /* sprintf(num, "%d", size); */
            tmp_ptr = my_itoa(size);
            bufcat(log, " "); 
            /* bufcat(log, num);  */
            bufcat(log, tmp_ptr);

            /* tmp_variable_add(base_name, num, -1); */
            tmp_variable_add(base_name, tmp_ptr, -1);

            if (size)
            {
                bufcat(log, "\n");
            }
            else
            {
                bufcat(log, ">\n");
            }
        }

        if (secs_data_type == LIST)
        {
            e[l]--;
            l++;
            e[l] = size;
            ie[l] = 0;            
            if (size)
            {
            	;
            }
            else
            {
                l--;
            }

        }
        else
        {
            e[l]--;
        }

        /* Work on each data type */

        if (secs_data_type == ASCII)
        {
            DEBUG (3, DEBUG_HDR, "Ascii\n"); 
            bufcat(log, pad);
            bufcat(log, "<A \"");
            
            /* There may be non-printing chars, so change them to .'s */ 
            /* for(i=0; i<size; i++) if (!isprint(p[i])) p[i] = '.'; */

            bufncat(log, (char *)p, size);
            bufcat(log, "\">\n");

            tmp_variable_add(base_name, (char *)p, (int)size);

            p += size;
            length -= size;
        }
        else if (secs_data_type == BOOLEAN)
        {
            DEBUG (3, DEBUG_HDR, "Boolean\n"); 
            bufcat(log, pad);
            bufcat(log, "<BOOLEAN");
            array = false;
            if (size == 0)
            {
                strcpy(name, base_name);
                tmp_variable_add(name, "", -1);
            }
            else if (size > 1)
            {
                array = true;
            }

            array_size = size;
            for (uint32_t i=0; i<array_size; i++)
            {
                strcpy(name, base_name);
                if (array)
                {
                    /* sprintf(suffix, "[%d]", i+1); */
                    /* strcat(name, suffix); */
                    strcat(name, my_itoa_fmt1(i+1)); 
                }

                b = (unsigned char) *p;
                if (b == 0)
                {
                    bufcat(log, " FALSE"); 
                    tmp_variable_add(name, "0", -1);
                }
                else
                {
                    bufcat(log, " TRUE"); 
                    tmp_variable_add(name, "1", -1);
                }
                p++;
                length--;
            }

            bufcat(log, ">\n");

        }
        else if (secs_data_type == BINARY)
        {
            DEBUG (3, DEBUG_HDR, "Binary\n"); 
            bufcat(log, pad);
            bufcat(log, "<BINARY");
            array = false;
            if (size == 0)
            {
                strcpy(name, base_name);
                tmp_variable_add(name, "", -1);
            }
            else if (size > 1)
            {
                array = true;
            }

            tmp_p = p;
            array_size = size;
            for (uint32_t i=0; i<array_size; i++)
            {
                strcpy(name, base_name);
                if (array)
                {
                    /* sprintf(suffix, "[%d]", i+1); */
                    /* strcat(name, suffix); */
                    strcat(name, my_itoa_fmt1(i+1)); 
                }

                b = *p;
                /* In the log file show in hex format */
                /* sprintf(num, "%.2X (%d)", (unsigned char) b, (unsigned char) b); */
                bufcat(log, " "); 
                /* bufcat(log, num); */
                bufcat(log, my_btoh_fmt1(b));
                /* Assign in event as decimal value */
                /* sprintf(num, "%d", (unsigned char) b); */
                tmp_variable_add(name, my_itoa(b), -1);
                p++;
                length--;
            }

            if (array)
            {
                /* This variable, xx[]= includes all the data */
                strcpy(name, base_name);
                strcat(name, "[]");
                bufcpy(&value, "");
                p = tmp_p;
                for (uint32_t i=0; i<size; i++)
                {
                    b = *p;
                    /* sprintf(num, "%2.2X", (unsigned char) b); */
                    /* bufcat(&value, num); */
                    bufcat(&value, my_btoh(b));
                    p++;
                }
                tmp_variable_add(name, bufdata(&value), -1);
            }
            bufcat(log, ">\n");

        }
        else if (secs_data_type == UNSIGNED_1)
        {
            DEBUG (3, DEBUG_HDR, "Unsigned 1\n"); 
            bufcat(log, pad);
            bufcat(log, "<U1");
            array = false;

            if (size == 0)
            {
                strcpy(name, base_name);
                tmp_variable_add(name, "", -1);
            }
            else
            {
                array = true;
            }

            array_size = size;
            for (uint32_t i=0; i<array_size; i++)
            {
                strcpy(name, base_name);
                if (array)
                {
                    /* sprintf(suffix, "[%d]", i+1); */
                    /* strcat(name, suffix); */
                    strcat(name, my_itoa_fmt1(i+1)); 
                }

//                memcpy(&u1, p, 1);
                u1 = *p;
                tmp_ptr = my_utoa(u1);  /* sprintf(num, "%d", u1); */
                bufcat(log, " "); 
                bufcat(log, tmp_ptr); /* bufcat(log, num); */
                tmp_variable_add(name, tmp_ptr, -1);  /* tmp_variable_add(name, num, -1); */
                p++;
                length--;
            } 

            bufcat(log, ">\n");

        }
        else if (secs_data_type == UNSIGNED_2)
        {
            DEBUG (3, DEBUG_HDR, "Unsigned 2\n"); 
            bufcat(log, pad);
            bufcat(log, "<U2");
            array = false;
            if (size == 0)
            {
                strcpy(name, base_name);
                tmp_variable_add(name, "", -1);
            }
            else if (size > 2)
            {
                array = true;
            }

            array_size = size/2;
            for (uint32_t i=0; i<array_size; i++)
            {
                strcpy(name, base_name);
                if (array)
                {
                    /* sprintf(suffix, "[%d]", i+1); */
                    /* strcat(name, suffix); */
                    strcat(name, my_itoa_fmt1(i+1));
                }

                memcpy(u2_union.the_bytes, p, 2);
                swap(u2_union.the_bytes, 2);
                tmp_ptr = my_utoa(u2_union.the_uint16);  /* sprintf(num, "%d", u2); */
                bufcat(log, " "); 
                bufcat(log, tmp_ptr); /* bufcat(log, num); */
                tmp_variable_add(name, tmp_ptr, -1); /* tmp_variable_add(name, num, -1); */
                p += 2;
                length -= 2;
            }

            bufcat(log, ">\n");

        }
        else if (secs_data_type == UNSIGNED_4)
        {
            DEBUG (3, DEBUG_HDR, "Unsigned 4\n"); 
            bufcat(log, pad);
            bufcat(log, "<U4");
            array = false;
            if (size == 0)
            {
                strcpy(name, base_name);
                tmp_variable_add(name, "", -1);
            }
            else if (size > 4)
            {
                array = 1;
            }

            array_size = size/4;
            for (uint32_t i=0; i<array_size; i++)
            {
                strcpy(name, base_name);
                if (array)
                {
                    /* sprintf(suffix, "[%d]", i+1); */
                    /* strcat(name, suffix); */
                    strcat(name, my_itoa_fmt1(i+1));  /* sprintf(suffix, "[%d]", i+1); */
                }

                memcpy(u4_union.the_bytes, p, 4);
                swap(u4_union.the_bytes, 4);
                tmp_ptr = my_utoa(u4_union.the_uint32);  /* sprintf(num, "%u", u4); */
                bufcat(log, " "); 
                /* bufcat(log, num); */
                bufcat(log, tmp_ptr);
                /* tmp_variable_add(name, num, -1); */
                tmp_variable_add(name, tmp_ptr, -1);
                p += 4;
                length -= 4;
            }

            bufcat(log, ">\n");

        }
        else if (secs_data_type == UNSIGNED_8)
        {
            DEBUG (3, DEBUG_HDR, "Unsigned 8\n"); 
            bufcat(log, pad);
            bufcat(log, "<U8");
            array = false;
            if (size == 0)
            {
                strcpy(name, base_name);
                tmp_variable_add(name, "", -1);
            }
            else if (size > 8)
            {
                array = true;
            }

            array_size = size/8;
            for (uint32_t i=0; i<array_size; i++)
            {
                strcpy(name, base_name);
                if (array)
                {
                    /* sprintf(suffix, "[%d]", i+1); */
                    /* strcat(name, suffix); */
                    strcat(name, my_itoa_fmt1(i+1));
                }

                memcpy(u8_union.the_bytes, p, 8);
                swap(u8_union.the_bytes, 8);
                tmp_ptr = my_utoa(u8_union.the_uint64);  /* sprintf(num, "%u", u8[1]); */
                bufcat(log, " "); 
                /* bufcat(log, num); */
                bufcat(log, tmp_ptr);
                /* tmp_variable_add(name, num, -1); */
                tmp_variable_add(name, tmp_ptr, -1);
                p += 8;
                length -= 8;
            }

            bufcat(log, ">\n");

        }
        else if (secs_data_type == SIGNED_1)
        {
            DEBUG (3, DEBUG_HDR, "Signed 1\n"); 
            bufcat(log, pad);
            bufcat(log, "<I1");
            array = false;
            if (size == 0)
            {
                strcpy(name, base_name);
                tmp_variable_add(name, "", -1);
            }
            else if (size > 1)
            {
                array = true;
            }

            array_size = size;
            for (uint32_t i=0; i<array_size; i++)
            {
                strcpy(name, base_name);
                if (array)
                {
                    /* sprintf(suffix, "[%d]", i+1); */
                    /* strcat(name, suffix); */
                    strcat(name, my_itoa_fmt1(i+1)); 
                }

//                memcpy(&i1, p, 1);
                i1 = (*(int8_t *)p);
                tmp_ptr = my_itoa(i1);  /* sprintf(num, "%d", i1); */
                bufcat(log, " "); 
                /* bufcat(log, num); */
                bufcat(log, tmp_ptr);
                /* tmp_variable_add(name, num, -1); */
                tmp_variable_add(name, tmp_ptr, -1);
                p++;
                length--;
            }

            bufcat(log, ">\n");

        }
        else if (secs_data_type == SIGNED_2)
        {
            DEBUG (3, DEBUG_HDR, "Signed 2\n"); 
            bufcat(log, pad);
            bufcat(log, "<I2");
            array = false;
            if (size == 0)
            {
                strcpy(name, base_name);
                tmp_variable_add(name, "", -1);
            }
            else if (size > 2)
            {
                array = true;
            }

            array_size = size/2;
            for (uint32_t i=0; i<array_size; i++)
            {
                strcpy(name, base_name);
                if (array)
                {
                    /* sprintf(suffix, "[%d]", i+1); */
                    /* strcat(name, suffix); */
                    strcat(name, my_itoa_fmt1(i+1)); 
                }

                memcpy(i2_union.the_bytes, p, 2);
                swap(i2_union.the_bytes, 2);
                tmp_ptr = my_itoa(i2_union.the_int16);  /* sprintf(num, "%d", i2); */
                bufcat(log, " "); 
                /* bufcat(log, num); */
                bufcat(log, tmp_ptr);
                /* tmp_variable_add(name, num, -1); */
                tmp_variable_add(name, tmp_ptr, -1);
                p += 2;
                length -= 2;
            }

            bufcat(log, ">\n");

        }
        else if (secs_data_type == SIGNED_4)
        {
            DEBUG (3, DEBUG_HDR, "Signed 4\n"); 
            bufcat(log, pad);
            bufcat(log, "<I4");
            array = false;
            if (size == 0)
            {
                strcpy(name, base_name);
                tmp_variable_add(name, "", -1);
            }
            else if (size > 4)
            {
                array = true;
            }

            array_size = size/4;
            for (uint32_t i=0; i<array_size; i++)
            {
                strcpy(name, base_name);
                if (array)
                {
                    /* sprintf(suffix, "[%d]", i+1); */
                    /* strcat(name, suffix); */
                    strcat(name, my_itoa_fmt1(i+1));
                }

                memcpy(i4_union.the_bytes, p, 4);
                swap(i4_union.the_bytes, 4);
                tmp_ptr = my_itoa(i4_union.the_int32);  /* sprintf(num, "%d", i4); */
                bufcat(log, " "); 
                /* bufcat(log, num); */
                bufcat(log, tmp_ptr);
                /* tmp_variable_add(name, num, -1); */
                tmp_variable_add(name, tmp_ptr, -1);
                p += 4;
                length -= 4;
            }

            bufcat(log, ">\n");

        }
        else if (secs_data_type == SIGNED_8)
        {
            DEBUG (3, DEBUG_HDR, "Signed 8\n"); 
            bufcat(log, pad);
            bufcat(log, "<I8");
            array = false;
            if (size == 0)
            {
                strcpy(name, base_name);
                tmp_variable_add(name, "", -1);
            }
            else if (size > 8)
            {
                array = true;
            }

            array_size = size/8;
            for (uint32_t i=0; i<array_size; i++)
            {
                strcpy(name, base_name);
                if (array)
                {
                    /* sprintf(suffix, "[%d]", i+1); */
                    /* strcat(name, suffix); */
                    strcat(name, my_itoa_fmt1(i+1)); 
                }
                memcpy(i8_union.the_bytes, p, 8);
                swap(i8_union.the_bytes, 8);
                tmp_ptr = my_itoa(i8_union.the_int64);  /* sprintf(num, "%d", i8[1]); */
                bufcat(log, " "); 
                /* bufcat(log, num); */
                bufcat(log, tmp_ptr);
                /* tmp_variable_add(name, num, -1); */
                tmp_variable_add(name, tmp_ptr, -1);
                p += 8;
                length -= 8;
            }

            bufcat(log, ">\n");

        }
        else if (secs_data_type == FLOAT_4)
        {
            DEBUG (3, DEBUG_HDR, "Float 4\n"); 
            bufcat(log, pad);
            bufcat(log, "<F4");
            if (size <= 4)
            {
                array = false;
            }
            else
            {
                array = true;
            }

            array_size = size/4;
            for (uint32_t i=0; i<array_size; i++)
            {
                strcpy(name, base_name);
                if (array)
                {
                    /* sprintf(suffix, "[%d]", i+1); */
                    /* strcat(name, suffix); */
                    strcat(name, my_itoa_fmt1(i+1)); 
                }
                memcpy(f4_union.the_bytes, p, 4);
                swap(f4_union.the_bytes, 4);
                sprintf(num, "%G", f4_union.the_float);
                bufcat(log, " "); 
                bufcat(log, num); 
                tmp_variable_add(name, num, -1);
                p += 4;
                length -= 4;
            }

            bufcat(log, ">\n");

        }
        else if (secs_data_type == FLOAT_8)
        {
            DEBUG (3, DEBUG_HDR, "Float 8\n"); 
            bufcat(log, pad);
            bufcat(log, "<F8");
            if (size <= 8)
            {
                array = false;
            }
            else
            {
                array = true;
            }

            array_size = size/8;
            for (uint32_t i=0; i<array_size; i++)
            {
                strcpy(name, base_name);
                if (array)
                {
                    /* sprintf(suffix, "[%d]", i+1); */
                    /* strcat(name, suffix); */
                    strcat(name, my_itoa_fmt1(i+1)); 
                }
                memcpy(f8_union.the_bytes, p, 8);
                swap(f8_union.the_bytes, 8);
                sprintf(num, "%G", f8_union.the_double);
                bufcat(log, " "); 
                bufcat(log, num); 
                tmp_variable_add(name, num, -1);
                p += 8;
                length -= 8;
            }

            bufcat(log, ">\n");

        }
        else if (secs_data_type == 0)
        {
        	;
        }
        else
        {
            ERROR ("Unknown secs type <%d>\n", secs_data_type);
            iret = 1;
            break;
        } 

        if (array)
        {
            tmp_variable_add(base_name, "ARRAY", -1);
            strcpy(name, base_name);
            strcat(name, "[0]");
            /* sprintf(num, "%d", array_size); */
            /* tmp_variable_add(name, num, -1); */
            tmp_variable_add(name, my_itoa(array_size), -1); 
        }

        while (e[l] == 0)
        {
            l--;
            memset(pad, '\0', sizeof(pad));
            memset(pad, ' ', (l+1)*3 + 30);
            bufcat(log, pad);
            bufcat(log, ">\n");
        }
    } // End while (len)

    buffree(&value);
    DEBUG (3, DEBUG_HDR, "Returning <%d>\n", iret);
    return iret;
}


