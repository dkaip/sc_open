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
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>
#include "include.h"

/*******************************************/

uint32_t parse(struct buffer *token, const char *buf, enum parse_types *type,
    const char *no_parse, bool comment_flag, bool pp_flag)
{
	char c = 0;
	uint32_t cnt = 0;

#undef NAME
#define NAME "parse()"

/* comment_flag, 0=process as data, 1=treat as a comment and ignore */

    /* DEBUG (5, DEBUG_HDR, "Called <%s>\n", buf); */
    DEBUG (5, DEBUG_HDR, "Called no_parse <%s>, comment_flag <%d>, pp_flag <%d>\n", 
        no_parse, comment_flag, pp_flag); 
 
    cnt = 0;
    bufcpy(token, "");
    *type = PT_NULL;  // Needs to be PT_NULL for the case where *buf = '\0'

    while (*buf != '\0')
    {
        DEBUG (5, DEBUG_HDR, "<%c>\n", *buf);

        /* Valid PT_NUMBER? */
        if (isdigit(*buf))
        {
            *type = PT_NUMBER;
            bufncat(token, buf, 1);
            cnt++;
            buf++;
            while (*buf != '\0')
            {
                if ((isdigit(*buf)) || 
                    (*buf == '.'))
                {
                    bufncat(token, buf, 1);
                    cnt++;
                    buf++;

                }
//                else if ((strncmp(buf, "e+", 2) == '\0') ||
//                         (strncmp(buf, "E+", 2) == '\0') ||
//                         (strncmp(buf, "e-", 2) == '\0') ||
//                         (strncmp(buf, "E-", 2) == '\0'))
                else if ((buf[0] == 'e' && buf[1] == '+') ||  // This is far faster
                         (buf[0] == 'E' && buf[1] == '+') ||
						 (buf[0] == 'e' && buf[1] == '-') ||
						 (buf[0] == 'E' && buf[1] == '-'))
                {
                    bufncat(token, buf, 2);
                    cnt += 2;
                    buf += 2;
 
                }
                else if (strchr(no_parse, *buf) != (char *)NULL)
                {
                    bufncat(token, buf, 1);
                    cnt++;
                    buf++;

                }
                else
                {
                    break;
                }
            }            
        }
        /* Valid PT_NAME characters?*/
        else if ((isalnum(*buf)) || (strchr("$_.{}[]", *buf) != (char *)NULL))
        {
            *type = PT_NAME;
            bufncat(token, buf, 1);
            cnt++;
            buf++;
            while (*buf != '\0')
            {
                if ((isalnum(*buf)) || (strchr("$_.{}[]", *buf) != (char *)NULL))
                {
                    bufncat(token, buf, 1);
                    cnt++;
                    buf++;
                 }
                else
                {
                    break;
                }
            }
        }
        else if (*buf == '\"')
        {
            DEBUG (5, DEBUG_HDR, "Found STRING\n");
            *type = PT_STRING;
            cnt++;
            buf++;
            while (*buf != '\0')
            {
                if (*buf == '\"')
                {
                    cnt++;
                    break;
                }
                else if (*buf == '\\')
                {
                    cnt++;
                    buf++;
                    if (*buf == 'n')
                    {
                        bufcat(token, "\\n");
                        buf++;
                        cnt++;
                        continue;
                    }
                    else if (*buf == 'r')
                    {
                        bufcat(token, "\\r");
                        buf++;
                        cnt++;
                        continue;
                    }
                    else if (*buf == 'x')
                    {
                    	uint32_t i = 0;
                        buf++;
                        cnt++;
                        sscanf(buf, "%2x", &i);
                        c = (char)i;
                        bufncat(token, &c, 1);
                        buf += 2;
                        cnt += 2;
                        continue;
                    }
                    else
                    {  /* Just add this char */
                        bufncat(token, buf, 1);
                        buf++;
                        cnt++;
                        continue;
                    }
                } 

                if (*buf == '\n')
                {
                    PP_ERROR ("Missing closing \"\n");
                }

                bufncat(token, buf, 1);
                buf++;
                cnt++;
            }
            break;

        }
        else if (*buf == '\'')
        {
            *type = PT_CONSTANT;
            cnt++;
            buf++;
            while (*buf != '\0')
            {
                if (*buf == '\'')
                {
                    cnt++;
                    break;
                }
                else if (*buf == '\\')
                {  /* Handle the esc char */
                    bufncat(token, buf, 1);
                    cnt++;
                    buf++;
                    bufncat(token, buf, 1);
                    buf++;
                    cnt++;
                    continue;
                }
               
                bufncat(token, buf, 1);
                buf++;
                cnt++;

                if (*buf == '\n')
                {
                    PP_ERROR ("Missing closing \'\n");
                }
            }
            break;

        /* Check for custom no parse chars */ 

        }
        else if (strchr(no_parse, *buf) != (char *)NULL)
        {
            bufncat(token, buf, 1);
            /* Something other than PT_NULL, or PT_END_OF_LINE */
            *type = PT_OTHER;
            buf++;
            cnt++;

        /* Look for #include */
        }
//        else if ((strncmp(buf, "#include ", strlen("#include ")) == 0) && pp_flag)
        else if ((strncmp(buf, "#include ", (sizeof("#include ")-1)) == 0) && pp_flag == true)
        {
            *type = PT_POUND_INCLUDE;
            DEBUG (5, DEBUG_HDR, "Found #include\n");
            while (*buf != ' ') { buf++; cnt++; } /* Skip over the #include */
            while (*buf == ' ') { buf++; cnt++; } /* Skip over all blanks */
            /* Get the data */
            while ((*buf != '\n') && (*buf != ' '))
            {
                if (strchr("<>\"", *buf) == (char *)NULL)
                {  /* Ignore these chars */
                    bufncat(token, buf, 1);
                }
                buf++;
                cnt++;
            }
            while (*buf != '\n') { buf++; cnt++; } /* Ignore the rest  */
            break;

        /* Look for #define */
        }
//        else if ((strncmp(buf, "#define ", strlen("#define ")) == 0) && pp_flag)
        else if ((strncmp(buf, "#define ", (sizeof("#define ")-1)) == 0) && pp_flag == true)
        {
            *type = PT_POUND_DEFINE;
            DEBUG (5, DEBUG_HDR, "Found #define\n");
            while (*buf != ' ') { buf++; cnt++; } /* Skip over the #define */
            while (*buf == ' ') { buf++; cnt++; } /* Skip over all blanks */
            /* Get all the data */
            while (*buf != '\n')
            {
                if (*buf == '\\')
                {
                    bufncat(token, buf, 1);
                    buf++;
                    cnt++;
                } 

                bufncat(token, buf, 1);
                buf++;
                cnt++;
            }
            DEBUG (5, DEBUG_HDR, "Found #define <%s>\n", bufdata(token));
            break;

        /* Look for #ifdef */
        }
//        else if ((strncmp(buf, "#ifdef ", strlen("#ifdef ")) == 0) && pp_flag)
        else if ((strncmp(buf, "#ifdef ", (sizeof("#ifdef ")-1)) == 0) && pp_flag == true)
        {
            *type = PT_POUND_IFDEF;
            DEBUG (5, DEBUG_HDR, "Found #ifdef\n");
            // TODO we are searching for a specific string "#ifdef " who should be able to just +7
            while (*buf != ' ') { buf++; cnt++; } /* Skip over the #ifdef */
            while (*buf == ' ') { buf++; cnt++; } /* Skip over all blanks */
            while ((*buf != '\n') && (*buf != ' ')) { bufncat(token, buf, 1); buf++; cnt++; }
            while (*buf != '\n') { buf++; cnt++; } /* Ignore the rest  */
            break;

        /* Look for #else */
        }
//        else if ((strncmp(buf, "#else\n", strlen("#else\n")) == 0) && pp_flag)
        else if ((strncmp(buf, "#else\n", (sizeof("#else\n")-1)) == 0) && pp_flag == true) // Note \n in search string
        {
//            buf = buf + strlen("#else");
//            cnt = cnt + (uint32_t)strlen("#else");
            buf += (sizeof("#else")-1);
            cnt += (uint32_t)(sizeof("#else")-1);
            *type = PT_POUND_ELSE;
            DEBUG (5, DEBUG_HDR, "Found #else\n");
            break;
        }
//        else if ((strncmp(buf, "#else ", strlen("#else ")) == 0) && pp_flag)
        else if ((strncmp(buf, "#else ", (sizeof("#else ")-1)) == 0) && pp_flag == true)
        {
//            buf = buf + strlen("#else");
//            cnt = cnt + (uint32_t)strlen("#else");
            buf += (sizeof("#else")-1);
            cnt += (uint32_t)(sizeof("#else")-1);
            *type = PT_POUND_ELSE;
            DEBUG (5, DEBUG_HDR, "Found #else\n");
            break;

        /* Look for #endif */
        }
//        else if ((strncmp(buf, "#endif\n", strlen("#endif\n")) == 0) && pp_flag)
        else if ((strncmp(buf, "#endif\n", (sizeof("#endif\n")-1)) == 0) && pp_flag == true) // Note \n in search string
        {
            *type = PT_POUND_ENDIF;
//            buf = buf + strlen("#endif");
//            cnt = cnt + (uint32_t)strlen("#endif");
            buf += (sizeof("#endif")-1);
            cnt += (uint32_t)(sizeof("#endif")-1);
            DEBUG (5, DEBUG_HDR, "Found #endif\n");
            break;

        }
//        else if ((strncmp(buf, "#endif ", strlen("#endif ")) == 0) && pp_flag)
        else if ((strncmp(buf, "#endif ", (sizeof("#endif ")-1)) == 0) && pp_flag == true)
        {
            *type = PT_POUND_ENDIF;
//            buf = buf + strlen("#endif");
//            cnt = cnt + (uint32_t)strlen("#endif");
            buf += (sizeof("#endif")-1);
            cnt += (uint32_t)(sizeof("#endif")-1);
            DEBUG (5, DEBUG_HDR, "Found #endif\n");
            break;

        /* Comment char # */
        }
        else if ((*buf == '#') && comment_flag == true)
        {
           bufcat(token, "");
           *type = PT_END_OF_LINE;
            while (*buf != '\0')
            {
                buf++;
                cnt++;
                if (*buf == '\n')
                {
                    buf++;
                    cnt++;
                    break;
                }
            }
            break;

        /* Comment char // */
        }
        else if ((*buf == '/') && (*(buf+1) == '/') && comment_flag == true)
        {
            bufcat(token, "");
            *type = PT_END_OF_LINE;
            while (*buf != '\0')
            {
                buf++;
                cnt++;
                if (*buf == '\n')
                {
                    buf++;
                    cnt++;
                    break;
                }
            }
            break;

        /* C style comments */
        }
        else if ((*buf == '/') && (*(buf+1) == '*') && comment_flag == true)
        {
            bufcat(token, "");
            bufncat(token, buf, 2);
            buf++; cnt++; buf++; cnt++;
            *type = PT_COMMENT;
            while (*buf != '\0')
            {
                if ((*buf == '/') && (*(buf+1) == '*'))
                {
                    PP_ERROR ("Imbedded comment detected\n");
                }

                if ((*buf == '*') && (*(buf+1) == '/'))
                {
                    break;
                }
                bufncat(token, buf, 1);
                buf++;
                cnt++;
            }
            bufncat(token, buf, 2);
            buf++; cnt++; buf++; cnt++;
            break;

        }
        else if (strchr("<>=!", *buf) != (char *)NULL)
        {
            /* Possible 2 char token, "?=" */
            if (bufsize(token) == 0)
            {
                *type = PT_OTHER;
                bufncat(token, buf, 1);
                cnt++;
                buf++;
                if (*buf == '=')
                {
                    bufncat(token, buf, 1);
                    cnt++;
                }
                else
                {
                    buf--;
                }
                break;
            }
            else
            {
                break;
            }

        }
        else if (strchr("(),$-+/*&|^:.~", *buf) != (char *)NULL)
        {
            if (bufsize(token))
            {
                break;
            }
            else
            {
                *type = PT_OTHER;
                bufncat(token, buf, 1);
                cnt++;
                break;
            }

        /* Look for a \n, \r, or \xhh chars */

        }
        else if (*buf == '\\')
        {
            if (bufsize(token))
            {
                break;
            }
            else
            {
                *type = PT_OTHER;
                bufncat(token, buf, 1);
                cnt++;
                buf++;
                if (strchr("nr", *buf) != (char *)NULL)
                {
                    bufncat(token, buf, 1);
                    cnt++;
                    buf++;
                }
                else if (*buf == 'x')
                {
                    bufncat(token, buf, 1);
                    cnt++;
                    buf++;
                    if (isxdigit(*buf))
                    {
                        c = *buf;
                        cnt++;
                        buf++;
                        if (isxdigit(*buf))
                        {
                            bufncat(token, &c, 1);
                            bufncat(token, buf, 1);
                            cnt++;
                            buf++;
                    
                        }
                        else
                        {
                            bufncat(token, "0", 1);
                            bufncat(token, buf, 1);
                            cnt--;
                            buf--;
                        }

                    }
                    else
                    {
                        buf--;
                        cnt--;
                    }
                }
                break;
            }
        }
        else if (strchr("\n\r;", *buf) != (char *)NULL)
        {  /* End of line */
            if (bufsize(token))
            {
                break;
            }
            else if ((*buf == '\r') && (*(buf+1) == '\n'))
            {  /* <cr><nl> */
//                cnt++;
//                cnt++;
//                bufncat(token, "\\n", 2);
            	char tempo = '\n';
            	bufncat(token, &tempo, 1);
            	cnt++;
            }
            else if (*buf == '\n')
            {
                cnt++;
//                bufncat(token, "\\n", 2);
                bufncat(token, buf, 1);
            }
            else if (*buf == ';')
            {
                cnt++;
                bufncat(token, ";", 1);
            }
            *type = PT_END_OF_LINE;
            break;

        }
        else
        {
            if (bufsize(token) == 0)
            {
                buf++;
                cnt++;
            }
            else
            {
                break;
            }
        }
    }

    DEBUG (3, DEBUG_HDR, "Returning count <%d>, token <%s>, type <%s>>\n", 
        cnt, bufdata(token), show_parse_type(*type));

//    LOG ("Returning count <%d>, token <%s>, type <%s>>\n",
//        cnt, bufdata(token), show_parse_type(*type));
    return cnt;
}

/*******************************************/

const char *show_parse_type(enum parse_types type)
{
#undef NAME
#define NAME "show_parse_type()"

    if (type == PT_NAME) return "Name";
    if (type == PT_OTHER) return "Other";
    if (type == PT_NUMBER) return "Number";
    if (type == PT_NULL) return "Null";
    if (type == PT_STRING) return "String";
    if (type == PT_CONSTANT) return "Constant";
    if (type == PT_END_OF_LINE) return "End of line";
    if (type == PT_POUND_INCLUDE) return "#include";
    if (type == PT_POUND_DEFINE) return "#define";
    if (type == PT_POUND_IFDEF) return "#ifdef";
    if (type == PT_POUND_ELSE) return "#else";
    if (type == PT_POUND_ENDIF) return "#endif";
    if (type == PT_COMMENT) return "comment";


    FATAL ("Parse type <%d>, NOT DEFINED\n", type);

    return "It's dead Jim";
}

