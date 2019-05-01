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
#include <stdlib.h> 
#include <stdint.h>
#include "include.h"


static uint32_t get_num_list_elements();
static uint8_t set_size_bytes(uint32_t item_length, uint8_t *data);
static uint32_t make_log(const struct binary_buffer *buffer, struct buffer *log);

#define CLEAR_HEADER { item_hdr_data.the_uint32 = 0; }

#define STORE_DATA(secs_data_type) {\
num_length_bytes = set_size_bytes(num_bytes, &item_hdr_data.the_bytes[1]);\
item_hdr_data.the_bytes[0] = (uint8_t)(secs_data_type + num_length_bytes); \
binary_bufncat(buffer, item_hdr_data.the_bytes, (uint32_t)(num_length_bytes+1));\
binary_bufncat(buffer, binary_buffer_data(&data), binary_buffer_num_data(&data)); }


/********************************************/

uint32_t secs_encode(struct binary_buffer *buffer, uint32_t *stream, uint32_t *function,
    uint32_t *device_id, uint32_t *wait_bit, struct buffer *log)
{
	struct buffer token = INIT_BUFFER;
	struct binary_buffer data = INIT_BINARY_BUFFER;
	uint32_t iret = 0;
	uint32_t  num_length_bytes = 0;
	uint32_t num_bytes = 0;
	enum token_types type = TT_NOT_ASSIGNED_YET;
	int  cnt = 0;
	union
	{
		uint8_t  the_bytes[4];
		uint32_t the_uint32;
	} item_hdr_data;

//	uint8_t hdr_data[64];
	char *temp_ptr = (char *)NULL;
	int8_t i1 = 0;
	uint8_t u1 = 0;
	uint32_t u = 0;
	union I2Union i2_union;
	union U2Union u2_union;
	union I4Union i4_union;
	union U4Union u4_union;
	union I8Union i8_union;
	union U8Union u8_union;
	union F4Union f4_union;
	union F8Union f8_union;

#undef NAME
#define NAME "secs_encode()"

    DEBUG (3, DEBUG_HDR, "Called\n");

    while (1)
    {
        get_program_token(&token, &type);
        if (sscanf(bufdata(&token), "S%uF%u", stream, function) != 2)
        {
            ERROR ("Error encoding stream/function from <%s>, ex: S1F1, not s1f1 \n", 
                bufdata(&token));
            iret = 1;
            break;
        } 

        get_program_token(&token, &type);
        if (sscanf(bufdata(&token), "%u", device_id) != 1)
        {
            ERROR ("Error encoding device id from <%s>\n", bufdata(&token));
            iret = 1;
            break;
        } 

        get_program_token(&token, &type);
        if (bufdata(&token)[0] == 'W')
        {
            *wait_bit = 1;
        
        }
        else
        {
            put_program_token();
            *wait_bit = 0;
            put_program_token();
        }

        break;
    }

    DEBUG (4, DEBUG_HDR, "Stream <%d>, function <%d>, device id <%d>, wait bit <%d>\n", 
        *stream, *function, *device_id, *wait_bit); 

    while (1)
    {
        if (iret)
        {
            break;
        }

        get_program_token(&token, &type);
        DEBUG (4, DEBUG_HDR, "Processing token <%s>, type <%s>\n", bufdata(&token), show_type(type));
        if ((type == TT_NULL_TOKEN) || (type == TT_PERIOD))
        {
            break;
        }
        else if (type == TT_LINE_NUMBER)
        {
            set_line_number(bufdata(&token));
            continue;
        }
        else if (type == TT_END_OF_LINE)
        {
            continue;
        }
        else if (type == TT_GREATER_THAN)
        {
        	continue;
        }
        else if (type == TT_LESS_THAN)
        {
        	continue;
        }
        else if (bufdata(&token)[0] == 'L')
        {
            DEBUG (4, DEBUG_HDR, "List\n");
            CLEAR_HEADER
			/*
			 * Remember boys and girls that for the list type the value is the
			 * number of elements in the list.
			 */
			num_bytes = get_num_list_elements();
            num_length_bytes = set_size_bytes(num_bytes, &item_hdr_data.the_bytes[1]);
            item_hdr_data.the_bytes[0] = LIST + (uint8_t)num_length_bytes;
            binary_bufncat(buffer, item_hdr_data.the_bytes, num_length_bytes+1);
        }
        else if (bufdata(&token)[0] == 'A')
        {  /* Ascii */
            DEBUG (4, DEBUG_HDR, "Ascii\n");
            binary_buf_empty(&data);
            CLEAR_HEADER
//            memset(hdr_data, '\0', sizeof(hdr_data));
			num_bytes = 0;
            while (1)
            {
                get_program_token(&token, &type);
                if (type == TT_GREATER_THAN)
                {
                    put_program_token();
                    break;
                }

                binary_bufncat(&data, (uint8_t *)bufdata(&token), bufsize(&token));
                num_bytes += bufsize(&token);
            } 

            STORE_DATA(ASCII)
//            num_length_bytes = set_size_bytes(ne, &hdr_data[1]);
//            hdr_data[0] = ASCII + num_length_bytes;
//            bufncat(buffer, (char *)hdr_data, num_length_bytes+1);
//            bufncat(buffer, bufdata(&data), bufsize(&data));
    
        }
        /*
         * Since we are looking for BO(boolean) before B(binary) we should be
         * okay and not have to worry about problems.
         */
        else if (bufdata(&token)[0] == 'B' && bufdata(&token)[1] == 'O')
		{  /* Boolean */
			DEBUG (4, DEBUG_HDR, "Boolean\n");
			binary_buf_empty(&data);
			CLEAR_HEADER
//                memset(hdr_data, '\0', sizeof(hdr_data));
			num_bytes = 0;
			while (1)
			{
				get_program_token(&token, &type);
				if (type == TT_GREATER_THAN)
				{
					put_program_token();
					break;
				}

				uint8_t a_boolean = 0;
				if ((bufdata(&token)[0] == 'T') || (bufdata(&token)[0] == '1'))
				{
					a_boolean = 1;
					binary_bufncat(&data, &a_boolean, 1);
				}
				else if ((bufdata(&token)[0] == 'F') || (bufdata(&token)[0] == '0'))
				{
					a_boolean = 0;
					binary_bufncat(&data, &a_boolean, 1);
				}
				else
				{
					ERROR ("Invalid boolean value <%s>, must be TRUE or FALSE, or T or F\n",
						bufdata(&token));
					iret = 1;
					break;
				}
				num_bytes++;
			}

			STORE_DATA(BOOLEAN)
//                size_bytes = set_size_bytes(ne, &hdr_data[1]);
//                hdr_data[0] = BOOLEAN + size_bytes;
//                bufncat(buffer, hdr_data, size_bytes+1);
//                bufncat(buffer, bufdata(&data), bufsize(&data));

		}
		else if (bufdata(&token)[0] == 'B')
		{  /* Binary */
			binary_buf_empty(&data);
			CLEAR_HEADER
//                memset(hdr_data, '\0', sizeof(hdr_data));
			num_bytes = 0;
			while (1)
			{
				get_program_token(&token, &type);
				if (type == TT_GREATER_THAN)
				{
					put_program_token();
					break;
				}

				temp_ptr = bufdata(&token);
				while (*temp_ptr != '\0')
				{
					u1 = 0;
					u = my_atobyte(temp_ptr, &cnt);
					if (cnt == -1)
					{
						ERROR ("Invalid U1\n");
						iret = 1;
						break;

					}
					else
					{
						u1 = (unsigned char) u;
						binary_bufncat(&data, &u1, 1);
						num_bytes++;
						temp_ptr += cnt;
					}
				}
#ifdef oldcode
				while (*temp_ptr != '\0') {
					i = u2 = 0;
					if (sscanf(temp_ptr, "%2x", &i) == 1) {
						b = (unsigned char) i;
						bufncat(&data, (char *) &b, 1);
						num_elements++;
						while (*temp_ptr == ' ') temp_ptr++;
						if (strlen(p) > 2) {
							temp_ptr++;
							temp_ptr++;

						} else {
							break;
						}

					} else {
						break;
					}
				}
#endif

			}

			STORE_DATA(BINARY)
//                size_bytes = set_size_bytes(ne, &hdr_data[1]);
//                hdr_data[0] = BINARY + size_bytes;
//                bufncat(buffer, hdr_data, size_bytes+1);
//                bufncat(buffer, bufdata(&data), bufsize(&data));
		}
        else if (bufdata(&token)[0] == 'U')
        {  /* U1, U2, U4, U8 */
        
            if (bufdata(&token)[1] == '1')
            {  /* U1 */
                binary_buf_empty(&data);
                CLEAR_HEADER
//                memset(hdr_data, '\0', sizeof(hdr_data));
				num_bytes = 0;
                while (1)
                {
                    get_program_token(&token, &type);
                    if (type == TT_GREATER_THAN)
                    {
                        put_program_token();
                        break;
                    }

                    temp_ptr = bufdata(&token);
                    while (*temp_ptr != '\0')
                    {
                        u1 = 0;
                        u1 = (uint8_t)my_atou(temp_ptr, &cnt);
                        if (cnt == -1)
                        {
                            ERROR ("Invalid U1\n");
                            iret = 1;
                            break;
                        }
                        else
                        {
                        	binary_bufncat(&data, &u1, 1);
                        	num_bytes++;
                            temp_ptr += cnt;
                        }
                    }
                }

                STORE_DATA(UNSIGNED_1)
//                size_bytes = set_size_bytes(ne, &hdr_data[1]);
//                hdr_data[0] = UNSIGNED_1 + size_bytes;
//                bufncat(buffer, hdr_data, size_bytes+1);
//                bufncat(buffer, bufdata(&data), bufsize(&data));
            }
            else if (bufdata(&token)[1] == '2')
            {  /* U2 */
                binary_buf_empty(&data);
                CLEAR_HEADER
//                memset(hdr_data, '\0', sizeof(hdr_data));
				num_bytes = 0;
                while (1) {
                    get_program_token(&token, &type);
                    if (type == TT_GREATER_THAN)
                    {
                        put_program_token();
                        break;
                    }
   
                    temp_ptr = bufdata(&token);
                    while (*temp_ptr != '\0')
                    {
                        u2_union.the_uint16 = (uint16_t)my_atou(temp_ptr, &cnt);
                        if (cnt == -1)
                        {
                            ERROR ("Invalid U2\n");
                            iret = 1;
                            break;

                        }
                        else
                        {
                            swap(u2_union.the_bytes, 2);
                            binary_bufncat(&data, u2_union.the_bytes, 2);
                            num_bytes += 2;
                            temp_ptr += cnt;
                        }
                    }
                }

                STORE_DATA(UNSIGNED_2)
//                size_bytes = set_size_bytes(ne*2, &hdr_data[1]);
//                hdr_data[0] = UNSIGNED_2 + size_bytes;
//                bufncat(buffer, hdr_data, size_bytes+1);
//                bufncat(buffer, bufdata(&data), bufsize(&data));
            }
            else if (bufdata(&token)[1] == '4')
            {  /* U4 */
                binary_buf_empty(&data);
                CLEAR_HEADER
//                memset(hdr_data, '\0', sizeof(hdr_data));
				num_bytes = 0;
                while (1)
                {
                    get_program_token(&token, &type);
                    if (type == TT_GREATER_THAN)
                    {
                        put_program_token();
                        break;
                    }
   
                    temp_ptr = bufdata(&token);
                    while (*temp_ptr != '\0')
                    {
                        u4_union.the_uint32 = (uint32_t)my_atou(temp_ptr, &cnt);
                        if (cnt == -1) {
                            ERROR ("Invalid U4\n");
                            iret = 1;
                            break;

                        }
                        else
                        {
                            swap(u4_union.the_bytes, 4);
                            binary_bufncat(&data, u4_union.the_bytes, 4);
                            num_bytes += 4;
                            temp_ptr += cnt;
                        }
                    }
                }

                STORE_DATA(UNSIGNED_4)
//                size_bytes = set_size_bytes(ne*4, &hdr_data[1]);
//                hdr_data[0] = UNSIGNED_4 + size_bytes;
//                bufncat(buffer, hdr_data, size_bytes+1);
//                bufncat(buffer, bufdata(&data), bufsize(&data));
            }
            else if (bufdata(&token)[1] == '8')
            {  /* U8 */
                binary_buf_empty(&data);
                CLEAR_HEADER
//                memset(hdr_data, '\0', sizeof(hdr_data));
				num_bytes = 0;
                while (1)
                {
                    get_program_token(&token, &type);
                    if (type == TT_GREATER_THAN)
                    {
                        put_program_token();
                        break;
                    }
   
                    temp_ptr = bufdata(&token);
                    while (*temp_ptr != '\0')
                    {
                        u8_union.the_uint64 = my_atou(temp_ptr, &cnt);
                        if (cnt == -1)
                        {
                            ERROR ("Invalid U8\n");
                            iret = 1;
                            break;
                        }
                        else
                        {
                            swap(u8_union.the_bytes, 8);
                            binary_bufncat(&data, u8_union.the_bytes, 8);
                            num_bytes += 8;
                            temp_ptr += cnt;
                        }
                    }
                }

                STORE_DATA(UNSIGNED_8)
//                size_bytes = set_size_bytes(ne*8, &hdr_data[1]);
//                hdr_data[0] = UNSIGNED_8 + size_bytes;
//                bufncat(buffer, hdr_data, size_bytes+1);
//                bufncat(buffer, bufdata(&data), bufsize(&data));
            }
            else
            {
                ERROR ("Invalid token <%s>, type <%s>, detected\n", 
                    bufdata(&token), show_type(type));
                iret = 1;
                break;
            }
        
        }
        else if (bufdata(&token)[0] == 'I')
        {  /* I1, I2, I4 or I8 */

            if (bufdata(&token)[1] == '1')
            {  /* I1 */
                binary_buf_empty(&data);
                CLEAR_HEADER
//                memset(hdr_data, '\0', sizeof(hdr_data));
				num_bytes = 0;
                while (1)
                {
                    get_program_token(&token, &type);
                    if (type == TT_GREATER_THAN)
                    {
                        put_program_token();
                        break;

                    }
                    else if (type == TT_MINUS)
                    {
                        get_program_token(&token, &type);
                        bufprefix(&token, "-");
                    }
   
                    temp_ptr = bufdata(&token);
                    while (*temp_ptr != '\0')
                    {
                        i1 = (int8_t)my_atoi(temp_ptr, &cnt);
                        if (cnt == -1)
                        {
                            ERROR ("Invalid I1\n");
                            iret = 1;
                            break;

                        }
                        else
                        {
                        	binary_bufncat(&data, (uint8_t *)&i1, 1);
                        	num_bytes++;
                            temp_ptr += cnt;
                        }
                    }
                }

                STORE_DATA(SIGNED_1)
//                size_bytes = set_size_bytes(ne, &hdr_data[1]);
//                hdr_data[0] = SIGNED_1 + size_bytes;
//                bufncat(buffer, hdr_data, size_bytes+1);
//                bufncat(buffer, bufdata(&data), bufsize(&data));
            }
            else if (bufdata(&token)[1] == '2')
            {  /* I2 */
                binary_buf_empty(&data);
                CLEAR_HEADER
//                memset(hdr_data, '\0', sizeof(hdr_data));
				num_bytes = 0;
                while (1)
                {
                    get_program_token(&token, &type);
                    if (type == TT_GREATER_THAN)
                    {
                        put_program_token();
                        break;

                    }
                    else if (type == TT_MINUS)
                    {
                        get_program_token(&token, &type);
                        bufprefix(&token, "-");
                    }
   
                    temp_ptr = bufdata(&token);
                    while (*temp_ptr != '\0')
                    {
                        i2_union.the_int16 = (int16_t)my_atoi(temp_ptr, &cnt);
                        if (cnt == -1)
                        {
                            ERROR ("Invalid I2\n");
                            iret = 1;
                            break;
                        }
                        else
                        {
                            swap(i2_union.the_bytes, 2);
                            binary_bufncat(&data, i2_union.the_bytes, 2);
                            num_bytes += 2;
                            temp_ptr += cnt;
                        }
                    }
                }

                STORE_DATA(SIGNED_2)
//                size_bytes = set_size_bytes(ne*2, &hdr_data[1]);
//                hdr_data[0] = SIGNED_2 + size_bytes;
//                bufncat(buffer, hdr_data, size_bytes+1);
//                bufncat(buffer, bufdata(&data), bufsize(&data));
            }
            else if (bufdata(&token)[1] == '4')
            {  /* I4 */
                binary_buf_empty(&data);
                CLEAR_HEADER
//                memset(hdr_data, '\0', sizeof(hdr_data));
				num_bytes = 0;
                while (1)
                {
                    get_program_token(&token, &type);
                    if (type == TT_GREATER_THAN)
                    {
                        put_program_token();
                        break;

                    }
                    else if (type == TT_MINUS)
                    {
                        get_program_token(&token, &type);
                        bufprefix(&token, "-");
                    }
   
                    temp_ptr = bufdata(&token);
                    while (*temp_ptr != '\0')
                    {
                        i4_union.the_int32 = (int32_t)my_atoi(temp_ptr, &cnt);
                        if (cnt == -1)
                        {
                            ERROR ("Invalid I4\n");
                            iret = 1;
                            break;
                        }
                        else
                        {
                            swap(i4_union.the_bytes, 4);
                            binary_bufncat(&data, i4_union.the_bytes, 4);
                            num_bytes += 4;
                            temp_ptr += cnt;
                        }
                    }
                }

                STORE_DATA(SIGNED_4)
//                size_bytes = set_size_bytes(ne*4, &hdr_data[1]);
//                hdr_data[0] = SIGNED_4 + size_bytes;
//                bufncat(buffer, hdr_data, size_bytes+1);
//                bufncat(buffer, bufdata(&data), bufsize(&data));
            }
            else if (bufdata(&token)[1] == '8')
            {  /* I8 */
                binary_buf_empty(&data);
                CLEAR_HEADER
//                memset(hdr_data, '\0', sizeof(hdr_data));
				num_bytes = 0;
                while (1)
                {
                    get_program_token(&token, &type);
                    if (type == TT_GREATER_THAN)
                    {
                        put_program_token();
                        break;

                    }
                    else if (type == TT_MINUS)
                    {
                        get_program_token(&token, &type);
                        bufprefix(&token, "-");
                    }
   
                    temp_ptr = bufdata(&token);
                    while (*temp_ptr != '\0')
                    {
                        i8_union.the_int64 = my_atoi(temp_ptr, &cnt);
                        if (cnt == -1) {
                            ERROR ("Invalid I8\n");
                            iret = 1;
                            break;
                        }
                        else
                        {
                            swap(i8_union.the_bytes, 8);
                            binary_bufncat(&data, i8_union.the_bytes, 8);
                            num_bytes += 8;
                            temp_ptr += cnt;
                        }
                    }
                }

                STORE_DATA(SIGNED_8)
//                size_bytes = set_size_bytes(ne*8, &hdr_data[1]);
//                hdr_data[0] = SIGNED_8 + size_bytes;
//                bufncat(buffer, hdr_data, size_bytes+1);
//                bufncat(buffer, bufdata(&data), bufsize(&data));
            }
            else
            {
                ERROR ("Invalid token <%s>, type <%s>, detected\n", 
                    bufdata(&token), show_type(type));
                iret = 1;
                break;
            } 
        }
        else if (bufdata(&token)[0] == 'F')
        {
            if (bufdata(&token)[1] == '4')
            {  /* F4 */
                binary_buf_empty(&data);
                CLEAR_HEADER
//                memset(hdr_data, '\0', sizeof(hdr_data));
				num_bytes = 0;
                while (1)
                {
                    get_program_token(&token, &type);
                    if (type == TT_GREATER_THAN)
                    {
                        put_program_token();
                        break;

                    }
                    else if (type == TT_MINUS)
                    {
                        get_program_token(&token, &type);
                        bufprefix(&token, "-");
                    }
   
                    temp_ptr = bufdata(&token);
                    while (temp_ptr != '\0')
                    {
                        /* See if this is in \xhh..hh format */

                    	// TODO may have issues here
                    	/*
                    	 * I think we have a potential problem here.  If the
                    	 * floating point number specified in hex format is
                    	 * not in BIG ENDIAN format that result will be incorrect.
                    	 */
                        if ((*temp_ptr == '\\') && (*(temp_ptr+1) == 'x'))
                        {
                            if (strlen(temp_ptr) != 10)
                            {  /* \xhhhhhhhh */
                                ERROR ("Too few or to many hex values for F4\n");
                                iret = 1;
                                break;
                            }
                            else
                            {
                                for (int i=0; i<4; i++)
                                {
                                	temp_ptr += 2;  /* Skip over the initial \x */
                                    if (sscanf(temp_ptr, "%2x", &u) == 1)
                                    {
                                    	u1 = (uint8_t)u;
                                        binary_bufncat(&data, &u1, 1);
                                    }
                                }
                            }

                            num_bytes += 4;
                            while (*temp_ptr == ' ') temp_ptr++;
                            temp_ptr = strchr(temp_ptr, ' ');
                        }
                        else if (sscanf((char *)temp_ptr, "%f", &f4_union.the_float) == 1)
                        {
                            swap(f4_union.the_bytes, 4);
                            binary_bufncat(&data, f4_union.the_bytes, 4);
                            num_bytes += 4;
                            while (*temp_ptr == ' ') temp_ptr++;
                            temp_ptr = strchr(temp_ptr, ' ');
                        }
                        else
                        {
                            break;
                        }
                    }
                }

                STORE_DATA(FLOAT_4)
//                size_bytes = set_size_bytes(ne*4, &hdr_data[1]);
//                hdr_data[0] = FLOAT_4 + size_bytes;
//                bufncat(buffer, hdr_data, size_bytes+1);
//                bufncat(buffer, bufdata(&data), bufsize(&data));
            }
            else if (bufdata(&token)[1] == '8')
            {  /* F8 */
                binary_buf_empty(&data);
                CLEAR_HEADER
//                memset(hdr_data, '\0', sizeof(hdr_data));
				num_bytes = 0;
                while (1)
                {
                    get_program_token(&token, &type);
                    if (type == TT_GREATER_THAN)
                    {
                        put_program_token();
                        break;

                    }
                    else if (type == TT_MINUS)
                    {
                        get_program_token(&token, &type);
                        bufprefix(&token, "-");
                    }
   
                    temp_ptr = bufdata(&token);
                    while (temp_ptr != '\0')
                    {
                        /* See if this is in \xhh..hh format */

                        // TODO we have the same ENDIAN issue here that
                        // we have above with the F4 type in hex form.
                        if ((*temp_ptr == '\\') && (*(temp_ptr+1) == 'x'))
                        {
                            if (strlen(temp_ptr) != 18)
                            {  /* \xhh...hh */
                                ERROR ("Too few or to many hex values for F8\n");
                                iret = 1;
                                break;

                            }
                            else
                            {
                                for (int i=0; i<8; i++)
                                {
                                	temp_ptr += 2;  /* Skip over the initial \x */
                                    if (sscanf(temp_ptr, "%2x", &u) == 1)
                                    {
                                    	u1 = (uint8_t)u;
                                        binary_bufncat(&data, &u1, 1);
                                    }
                                }
                            }
                            num_bytes += 8;
                            while (*temp_ptr == ' ') temp_ptr++;
                            temp_ptr = strchr(temp_ptr, ' ');
                        }
                        else
                        {
                            f8_union.the_double = atof(temp_ptr);
                            swap(f8_union.the_bytes, 8);
                            binary_bufncat(&data, f8_union.the_bytes, 8);
                            num_bytes += 8;
                            while (*temp_ptr == ' ') temp_ptr++;
                            temp_ptr = strchr(temp_ptr, ' ');
                        
#ifdef oldcode                        
                        /* This does not work */
                        } else if (sscanf(p, "%f", &f8) == 1) { 
                            /* f8 = (double) f4; */
                            swap((char *) &f8, 8);
                            bufncat(&data, (char *) &f8, 8);
                            num_elements++;
                            while (*p == ' ') p++;
                            p = strchr(p, ' ');

                        } else {
                            break;
#endif

                        }
                    }
                }

                STORE_DATA(FLOAT_8)
//                size_bytes = set_size_bytes(ne*8, &hdr_data[1]);
//                hdr_data[0] = FLOAT_8 + size_bytes;
//                bufncat(buffer, hdr_data, size_bytes+1);
//                bufncat(buffer, bufdata(&data), bufsize(&data));
            }
            else
            {
                ERROR ("Invalid token <%s>, type <%s>, detected\n", 
                    bufdata(&token), show_type(type));
                iret = 1;
                break;
           }

        }
        else if (type == TT_FOR)
        {
            if (cmd_for())
            {
                iret = 1;
                break;
            }
        
        }
        else if (type == TT_NEXT)
        {
            if (cmd_next())
            {
                iret = 1;
                break;
            }
        
        }
        else
        {
            ERROR ("Invalid token <%s>, type <%s>, detected\n", 
                bufdata(&token), show_type(type));
            iret = 1;
            break;
        }
    }

    /* Create the sml LOG data from the encoded data */
    char temp_buf[64];
    temp_buf[0] = 0;
    bufcpy(log, "");
    sprintf(temp_buf, "S%uF%u", *stream, *function);
    bufcat(log, temp_buf);
    sprintf(temp_buf, " DEVICE_ID=%u", *device_id);
    bufcat(log, temp_buf);
    if (*wait_bit)
    {
        bufcat(log, " WAIT_BIT=1");
    }
    else
    {
        bufcat(log, " WAIT_BIT=0");
    }
    bufcat(log, "\n");

    make_log(buffer, log); 

    buffree(&token);
    binary_buffree(&data);
    DEBUG (4, DEBUG_HDR, "Returning <%d>\n", iret);
    return iret;
}

/********************************************/

static uint32_t get_num_list_elements()
{
	uint32_t saved_idx = 0;
	uint32_t nest = 0;
	uint32_t cnt = 0;
	enum token_types type = TT_NOT_ASSIGNED_YET;
	struct buffer token = INIT_BUFFER;

#undef NAME
#define NAME "get_num_list_elements()"

    saved_idx = get_program_token_idx();
    nest = cnt = 0;

    while (1)
    {
        get_program_token(&token, &type);
        /* DEBUG (4, DEBUG_HDR, "Processing token <%s>, type <%s>\n", bufdata(&token), show_type(type)); */
        if (type == TT_NULL_TOKEN)
        {
            break;
        }
        else if (type == TT_LESS_THAN)
        {
            if (nest == 0)
            {
                cnt++;
            }
            nest++;

        }
        else if (type == TT_GREATER_THAN)
        {
            if (nest == 0)
            {
                break;
            }
            else
            {
                nest--;
            }
        }
        else if (type == TT_FOR)
        {
            if (cmd_for())
            {
                break;
            }
        }
        else if (type == TT_NEXT)
        {
            if (cmd_next())
            {
                break;
            }
        }
        else
        {
            continue;
        }
    }

    buffree(&token);
    set_program_token_idx(saved_idx);
    DEBUG (4, DEBUG_HDR, "Returning <%d>\n", cnt);
    return cnt;
}

/********************************************/

static uint8_t set_size_bytes(uint32_t item_length, uint8_t *data)
{
	uint32_t size = 0;
	uint8_t  x = 0;

#undef NAME
#define NAME "set_size_bytes()"

    DEBUG (4, DEBUG_HDR, "Called, item_length <%d>\n", item_length);

    size = 0;
    data[0] = data[1] = data[2] = 0;

    if (item_length <= 0xFF)
    {
    	// Only one length byte needed
        data[0] = (uint8_t)item_length;
        size = 1;
    }
    else if (item_length <= 0xFFFF)
    {
    	// Requires 2 length bytes
        x = (uint8_t)(item_length & 0xFF);
        data[1] = x;
        x = (uint8_t)((item_length >> 8) & 0xFF);
        data[0] = x;
        size = 2;

    }
    else
    {
    	// Requires 3 length bytes
        x = (uint8_t)(item_length & 0xFF);
        data[2] = x;
        x = (uint8_t)((item_length >> 8) & 0xFF);
        data[1] = x;
        x = (uint8_t)((item_length >> 16) & 0xFF);
        data[0] = x;
        size = 3;
    }

    DEBUG (4, DEBUG_HDR, "Returning size <%d>, data [0] <%x>, data[1] <%x>, data[2] <%x>\n", 
        size, (unsigned char) data[0], (unsigned char) data[1], (unsigned char) data[2]);

    return (uint8_t)size;
}

/********************************************/

static uint32_t make_log(const struct binary_buffer *buffer, struct buffer *log)
{
	uint32_t iret = 0;
	uint8_t secs_data_type = 0;
	uint32_t num_length_bytes = 0;
	uint32_t size = 0;
	uint32_t len = 0;
	uint32_t l = 0;
	uint32_t e[100], ie[100];
	uint8_t *p = (uint8_t *)NULL;
	union I2Union i2_union;
	union U2Union u2_union;
	union I4Union i4_union;
	union U4Union u4_union;
	union I8Union i8_union;
	union U8Union u8_union;
	union F4Union f4_union;
	union F8Union f8_union;
	char num[64];
	uint8_t b = 0;
	char pad[100];

#undef NAME
#define NAME "make_log()"

    DEBUG (3, DEBUG_HDR, "Called\n");

    // TODO magic number
    for (int i=0; i<100; i++)
    {
        e[i] = ie[i] = 0;
    }

    l = 0;
    p = binary_buffer_data(buffer);
    len = binary_buffer_num_data(buffer);

    while (len)
    {
    	secs_data_type = *p & 0xFC;
    	num_length_bytes = *p & 0x03;
        p++; // increment past type
        len--;
//        for (uint32_t i=0; i<num_length_bits; i++)
//        {
//            size = (size * 256) + *p;
//            p++;
//            len--;
//        }

        /*
         * If the number of length bits is not equal to 1, 2, or 3 we have
         * major problem.  The standard says that it can only be a 1, 2, or 3.
         */
        size = 0x00000000;
        if (num_length_bytes == 1)
        {
        	size = p[0];
        	len--;
        	p++;
        }
        else if (num_length_bytes == 2)
        {
        	size = ((uint32_t)(p[0] << 8) | p[1]);
        	len -= 2;
        	p += 2;
        }
        else
        {
        	size = ((uint32_t)(p[0] << 16) | (uint32_t)(p[1] << 8) | p[2]);
        	len -= 3;
        	p += 3;
        }


        DEBUG (3, DEBUG_HDR, "Type <%.2Xh>, num len bytes <%d>, size <%d>, remaining bytes <%d>\n",
        		secs_data_type, num_length_bytes, size, len);

        ie[l]++;

        /* strcpy(name, prefix); */

        memset(pad, '\0', sizeof(pad));
        memset(pad, ' ', (l+1)*3 + 30);

        if (secs_data_type == LIST)
        {
            DEBUG (3, DEBUG_HDR, "List\n"); 
            bufcat(log, pad);
            bufcat(log, "<L");
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

        if (secs_data_type == ASCII)
        {
            DEBUG (3, DEBUG_HDR, "Ascii\n"); 
            bufcat(log, pad);
            bufcat(log, "<A \"");
            bufncat(log, (char *)p, size);
            bufcat(log, "\">\n");

            p = p + size;
            len = len - size;
 
        }
        else if (secs_data_type == BOOLEAN)
        {
            DEBUG (3, DEBUG_HDR, "Boolean\n"); 
            bufcat(log, pad);
            bufcat(log, "<BOOLEAN");
            for (uint32_t i=0; i<size; i++)
            {
                b = *p;
                if (b == 0)
                {
                    bufcat(log, " FALSE"); 
                }
                else
                {
                    bufcat(log, " TRUE"); 
                }
                p++;
                len--;
            }
            bufcat(log, ">\n");

        }
        else if (secs_data_type == BINARY)
        {
            DEBUG (3, DEBUG_HDR, "Binary\n"); 
            bufcat(log, pad);
            bufcat(log, "<BINARY");
            for (uint32_t i=0; i<size; i++)
            {
                b = *p;
                /* In the log file show in hex format */
                /* sprintf(num, "%.2X (%d)", (unsigned char) b, (unsigned char) b); */
                bufcat(log, " "); 
                /* bufcat(log, num); */
                bufcat(log, my_btoh_fmt1(b)); 
                /* sprintf(num, "%d", (unsigned char) b); */
                p++;
                len--;
            }
            bufcat(log, ">\n");

        }
        else if (secs_data_type == UNSIGNED_1)
        {
            DEBUG (3, DEBUG_HDR, "Unsigned 1\n"); 
            bufcat(log, pad);
            bufcat(log, "<U1");
            for (uint32_t i=0; i<size; i++)
            {
//                memcpy(&u1, p, 1);
//
                /* sprintf(num, "%d", u1); */
                bufcat(log, " "); 
                /* bufcat(log, num); */
                bufcat(log, my_utoa(*p));
                p++;
                len--;
            }
            bufcat(log, ">\n");

        }
        else if (secs_data_type == UNSIGNED_2)
        {
            DEBUG (3, DEBUG_HDR, "Unsigned 2\n"); 
            bufcat(log, pad);
            bufcat(log, "<U2");
            for (uint32_t i=0; i<size/2; i++)
            {
                memcpy(u2_union.the_bytes, p, 2);
                swap(u2_union.the_bytes, 2);
                /* sprintf(num, "%d", u2); */
                bufcat(log, " "); 
                /* bufcat(log, num); */
                bufcat(log, my_utoa(u2_union.the_uint16));
                p = p + 2;
                len = len - 2;
            }
            bufcat(log, ">\n");

        }
        else if (secs_data_type == UNSIGNED_4)
        {
            DEBUG (3, DEBUG_HDR, "Unsigned 4\n"); 
            bufcat(log, pad);
            bufcat(log, "<U4");
            for (uint32_t i=0; i<size/4; i++)
            {
                memcpy(u4_union.the_bytes, p, 4);
                swap(u4_union.the_bytes, 4);
                /* sprintf(num, "%u", u4); */
                bufcat(log, " "); 
                /* bufcat(log, num); */
                bufcat(log, my_utoa(u4_union.the_uint32));
                p = p + 4;
                len = len - 4;
            }
            bufcat(log, ">\n");

        }
        else if (secs_data_type == UNSIGNED_8)
        {
            DEBUG (3, DEBUG_HDR, "Unsigned 8\n"); 
            bufcat(log, pad);
            bufcat(log, "<U8");
            for (uint32_t i=0; i<size/8; i++)
            {
                memcpy(u8_union.the_bytes, p, 8);
                swap(u8_union.the_bytes, 8);
                /* sprintf(num, "%u", u8[1]); */
                bufcat(log, " "); 
                /* bufcat(log, num); */
                bufcat(log, my_utoa(u8_union.the_uint64));
                p = p + 8;
                len = len - 8;
            }
            bufcat(log, ">\n");

        }
        else if (secs_data_type == SIGNED_1)
        {
            DEBUG (3, DEBUG_HDR, "Signed 1\n"); 
            bufcat(log, pad);
            bufcat(log, "<I1");
            for (uint32_t i=0; i<size; i++)
            {
//                memcpy(&i1, p, 1);
                /* sprintf(num, "%d", i1); */
                bufcat(log, " "); 
                /* bufcat(log, num); */
                bufcat(log, my_itoa((int64_t)*p));
                p++;
                len--;
            }
            bufcat(log, ">\n");

        }
        else if (secs_data_type == SIGNED_2)
        {
            DEBUG (3, DEBUG_HDR, "Signed 2\n"); 
            bufcat(log, pad);
            bufcat(log, "<I2");
            for (uint32_t i=0; i<size/2; i++)
            {
                memcpy(i2_union.the_bytes, p, 2);
                swap(i2_union.the_bytes, 2);
                /* sprintf(num, "%d", i2); */
                bufcat(log, " "); 
                /* bufcat(log, num); */
                bufcat(log, my_itoa(i2_union.the_int16));
                p = p + 2;
                len = len - 2;
            }
            bufcat(log, ">\n");

        }
        else if (secs_data_type == SIGNED_4)
        {
            DEBUG (3, DEBUG_HDR, "Signed 4\n"); 
            bufcat(log, pad);
            bufcat(log, "<I4");
            for (uint32_t i=0; i<size/4; i++)
            {
                memcpy(i4_union.the_bytes, p, 4);
                swap(i4_union.the_bytes, 4);
                /* sprintf(num, "%d", i4); */
                bufcat(log, " "); 
                /* bufcat(log, num); */
                bufcat(log, my_itoa(i4_union.the_int32));
                p = p + 4;
                len = len - 4;
            }
            bufcat(log, ">\n");
        }
        else if (secs_data_type == SIGNED_8)
        {
            DEBUG (3, DEBUG_HDR, "Signed 8\n"); 
            bufcat(log, pad);
            bufcat(log, "<I8");
            for (uint32_t i=0; i<size/8; i++)
            {
                memcpy(i8_union.the_bytes, p, 8);
                swap(i8_union.the_bytes, 8);
                /* sprintf(num, "%d", i8[1]); */
                bufcat(log, " "); 
                /* bufcat(log, num); */
                bufcat(log, my_itoa(i8_union.the_int64));
                p = p + 8;
                len = len - 8;
            }
            bufcat(log, ">\n");
        }
        else if (secs_data_type == FLOAT_4)
        {
            DEBUG (3, DEBUG_HDR, "Float 4\n"); 
            bufcat(log, pad);
            bufcat(log, "<F4");
            for (uint32_t i=0; i<size/4; i++)
            {
                memcpy(f4_union.the_bytes, p, 4);
                swap(f4_union.the_bytes, 4);
                sprintf(num, "%G", f4_union.the_float);
                bufcat(log, " "); 
                bufcat(log, num); 
                p = p + 4;
                len = len - 4;
            }
            bufcat(log, ">\n");

        }
        else if (secs_data_type == FLOAT_8)
        {
            DEBUG (3, DEBUG_HDR, "Float 8\n"); 
            bufcat(log, pad);
            bufcat(log, "<F8");
            for (uint32_t i=0; i<size/8; i++)
            {
                memcpy(f8_union.the_bytes, p, 8);
                swap(f8_union.the_bytes, 8);
                sprintf(num, "%G", f8_union.the_double);
                bufcat(log, " "); 
                bufcat(log, num); 
                p = p + 8;
                len = len - 8;
            }
            bufcat(log, ">\n");

        }
        else if (secs_data_type == 0)
        {

        }
        else
        {
            ERROR ("Unknown secs type <%d>\n", secs_data_type);
            iret = 1;
            break;
        } 

        while (e[l] == 0)
        {
            l--;
            memset(pad, '\0', sizeof(pad));
            memset(pad, ' ', (l+1)*3 + 30);
            bufcat(log, pad);
            bufcat(log, ">\n");
        }

    }

    DEBUG (3, DEBUG_HDR, "Returning <%d>\n", iret);
    return iret;
}


