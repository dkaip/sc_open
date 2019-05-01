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

#include "include.h"
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

static char *btoh_lookup[] = {
    "00", "01", "02", "03", "04", "05", "06", "07", "08", "09", "0A", "0B", "0C", "0D", "0E", "0F",
    "10", "11", "12", "13", "14", "15", "16", "17", "18", "19", "1A", "1B", "1C", "1D", "1E", "1F",
    "20", "21", "22", "23", "24", "25", "26", "27", "28", "29", "2A", "2B", "2C", "2D", "2E", "2F",
    "30", "31", "32", "33", "34", "35", "36", "37", "38", "39", "3A", "3B", "3C", "3D", "3E", "3F",
    "40", "41", "42", "43", "44", "45", "46", "47", "48", "49", "4A", "4B", "4C", "4D", "4E", "4F",
    "50", "51", "52", "53", "54", "55", "56", "57", "58", "59", "5A", "5B", "5C", "5D", "5E", "5F",
    "60", "61", "62", "63", "64", "65", "66", "67", "68", "69", "6A", "6B", "6C", "6D", "6E", "6F",
    "70", "71", "72", "73", "74", "75", "76", "77", "78", "79", "7A", "7B", "7C", "7D", "7E", "7F",
    "80", "81", "82", "83", "84", "85", "86", "87", "88", "89", "8A", "8B", "8C", "8D", "8E", "8F",
    "90", "91", "92", "93", "94", "95", "96", "97", "98", "99", "9A", "9B", "9C", "9D", "9E", "9F",
    "A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8", "A9", "AA", "AB", "AC", "AD", "AE", "AF",
    "B0", "B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8", "B9", "BA", "BB", "BC", "BD", "BE", "BF",
    "C0", "C1", "C2", "C3", "C4", "C5", "C6", "C7", "C8", "C9", "CA", "CB", "CC", "CD", "CE", "CF",
    "D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7", "D8", "D9", "DA", "DB", "DC", "DD", "DE", "DF",
    "E0", "E1", "E2", "E3", "E4", "E5", "E6", "E7", "E8", "E9", "EA", "EB", "EC", "ED", "EE", "EF",
    "F0", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "FA", "FB", "FC", "FD", "FE", "FF"
};

/********************************************/

// TODO yuck, another static
const char *my_itoa(int64_t num)
{
#define BUF_SIZE 32
static char buf[BUF_SIZE];
	char *p = (char *)NULL;
	bool sign = false;

    p = buf + BUF_SIZE - 1;
    *p = '\0';
    p--;

    if (num < 0)
    {
        sign = true;
        num = -num;
    }

    do
    {
        *--p = (char)((num % 10) + '0');
        num /= 10;
    } while (num);

    if (sign == true)
    {
        *--p = '-';
    }

    /* printf("my_itoa returning <%s>\n", p); */
    return p;
}

/********************************************/

// TODO yuck another static
const char *my_utoa(uint64_t num)  /* Unsigned to ascii */
{
#define BUF_SIZE 32
static char buf[BUF_SIZE];

	char *p;

    p = buf + BUF_SIZE - 1;
    *p = '\0';
    p--;
    do
    {
        *--p = (char)((num % 10) + '0');
        num /= 10;
    } while (num);

    /* printf("my_utoa returning <%s>\n", p); */
    return p;
}

/********************************************/

char *my_btoh(unsigned char num)  /* Unsigned char to hex, 00 -> FF  */
{

    return btoh_lookup[num];

}

/********************************************/

char *my_btoh_fmt1(unsigned char num)  /* Unsigned char to hex, "00 (0)" -> "FF (255)"  */
{
#define BUF_SIZE 32
static char buf[BUF_SIZE];

    strcpy(buf, "");
    strcat(buf, btoh_lookup[num]); 
    strcat(buf, " (");
    strcat(buf, my_utoa(num)); 
    strcat(buf, ")");
    return buf; 
}


/********************************************/

const char *my_itoa_fmt1(int64_t num)  /* Integer to ascii enclosed in []'s */
{
#define BUF_SIZE 32
static char buf[BUF_SIZE];
char *p;

    /* Enclose the converted num within []'s */

    p = buf;
    *p++ = '[';
    strcpy(p, my_itoa(num));
    strcat(buf, "]");
    return buf;
}


/********************************************/

uint8_t my_atobyte(const char *p, int *cnt)  /* Ascii to byte */
{
	uint8_t i = 0;
	uint8_t j = 0;
	int c1 = 0;
	int c2 = 0;

    while (*p == ' ')
    {  /* Consume leading white spaces */
        p++;
        c1++;
    }

    j = 2;
    while (j--)
    {
        if ((*p >= '0') && (*p <= '9'))
        {
            i = (uint8_t)((i * 16) + (*p - '0'));
            p++;
            c2++;

        }
        else if ((*p >= 'A') && (*p <= 'F'))
        {
            i = (uint8_t)((i * 16) + (*p - 55));
            p++;
            c2++;

        }
        else
        {
            break;
        }
    }

    if (c2) {
        *cnt = c1 + c2;

    } else {
        *cnt = -1;
    }

    return i;
}

/********************************************/

int64_t my_atoi(const char *p, int *cnt)  /* Ascii to integer */
{
int i, sign;
int c1, c2;

    c1 = c2 = sign = i = 0;

    while (*p == ' ') {  /* Consume leading white spaces */
        p++;
        c1++;
    }

    if (*p == '-') {
        sign = 1;
        c1++;
        p++;
    }

    while ((*p >= '0') && (*p <= '9')) {
        i = (i * 10) + (*p - '0');
        p++;
        c2++;
    }

    if (sign) {
        i = -i;
    }

    if (c2) {
        *cnt = c1 + c2;

    } else {
        *cnt = -1;
    }

    return i;
}

/********************************************/

uint64_t my_atou(const char *p, int *cnt)  /* Ascii to integer */
{
	int32_t c1 = 0;
	int32_t c2 = 0;
	uint64_t i = 0;

    while (*p == ' ')
    {  /* Consume leading white spaces */
        p++;
        c1++;
    }

    while ((*p >= '0') && (*p <= '9'))
    {
        i = (uint64_t)((i * 10) + (uint64_t)(*p - '0'));
        p++;
        c2++;
    }

    if (c2)
    {
        *cnt = c1 + c2;

    }
    else
    {
        *cnt = -1;
    }

    return i;
}

/********************************************/

/* #define test */

#ifdef test
main() {
int i, cnt;
unsigned int ui, u1, u2;
unsigned char c;
char s[32], buf[32];




#ifdef t_my_btoh
    printf("Starting my_btoh for 0 to 255\n");
    for (i=0; i<256; i++) {
        sprintf(s, "%2.2X", i);
        if (strcmp(s, my_btoh(i)) == 0) {
            printf("OK <%s> <%2.2X> <%d>\n", my_btoh(i), i, i);
        } else {
            printf("OK <%s> <%2.2X> <%d>\n", my_btoh(i), i, i);
            printf("!!!!!!!!!!!!!!!! NOT OK !!!!!!!!!!!!!!!\n");
            exit(0);
        }
    }
    printf("Done\n");
#endif




#ifdef t_my_btoh_fmt1
    printf("Starting my_btoh_fmt1 for 0 to 255\n");
    for (i=0; i<256; i++) {
        sprintf(s, "%2.2X (%d)", (unsigned char) i, (unsigned char) i); 
        if (strcmp(s, my_btoh_fmt1(i)) == 0) {
            printf("OK <%s> == <%s>\n", my_btoh_fmt1(i), s);

        } else {
            printf("<%s> != <%s>\n", s, my_btoh_fmt1(i));
            printf("!!!!!!!!!!!!!!!! NOT OK !!!!!!!!!!!!!!!\n");
            exit(0);
        }
    }
    printf("Done\n");
#endif




#ifdef t_my_utoa
    printf("Starting my_utoa for 0 to 4000000000\n");
    ui = 0u;
    while (ui < 4000000000u) {
        sprintf(s, "%u", ui); 
        if (strcmp(s, my_utoa(ui)) == 0) {
            printf("OK <%s> == <%s>\n", s, my_utoa(ui)); 

        } else {
            printf("<%s> != <%s>\n", s, my_utoa(ui));
            printf("!!!!!!!!!!!!!!!! NOT OK !!!!!!!!!!!!!!!\n");
            exit(0);
        }
        ui = ui + 100000u; 
    }
    printf("Done\n");
#endif




#ifdef t_my_itoa
    printf("Starting my_itoa for -2000000000 to 2000000000\n");
    i = -2000000000;
    while (i < 2000000000) {
        sprintf(s, "%d", i); 
        if (strcmp(s, my_itoa(i)) == 0) {
            printf("OK <%s> == <%s>\n", s, my_itoa(i)); 

        } else {
            printf("<%s> != <%s>\n", s, my_itoa(i));
            printf("!!!!!!!!!!!!!!!! NOT OK !!!!!!!!!!!!!!!\n");
            exit(0);
        }
        i = i + 100000; 
    }
    printf("Done\n");
#endif




#ifdef t_my_itoa_fmt1
    printf("Starting my_itoa_fmt1 for 0 to 1000\n");
    i = 0;
    while (i < 1000) {
        sprintf(s, "[%d]", i); 
        if (strcmp(s, my_itoa_fmt1(i)) == 0) {
            printf("OK <%s> == <%s>\n", s, my_itoa_fmt1(i)); 

        } else {
            printf("<%s> != <%s>\n", s, my_itoa_fmt1(i));
            printf("!!!!!!!!!!!!!!!! NOT OK !!!!!!!!!!!!!!!\n");
            exit(0);
        }
        i++;
    }
    printf("Done\n");
#endif




#ifdef t_my_atoi
    printf("Starting my_atoi\n");
    while(1) {
        printf("Enter number - ");
        gets(buf);
        i = my_atoi(buf, &cnt);
        printf("len <%d>, value <%d>, cnt <%d>\n", strlen(buf), i, cnt); 
    }
#endif



#define t_my_atou    
#ifdef t_my_atou
    printf("Starting my_atou\n");
    while(1) {
        printf("\nEnter number - ");
        gets(buf);
        u1 = my_atou(buf, &cnt);
        printf("len <%d>, value <%u>, cnt <%d>\n", strlen(buf), u1, cnt); 
    }
#endif
  


 
#define t_my_atob 
#ifdef t_my_atob
    printf("Starting my_atob\n");
    printf("<%d>\n", 'A');
    while(1) {
        printf("\nEnter number - ");
        gets(buf);
        c = my_atobyte(buf, &cnt);
        printf("len <%d>, value hex <%X> decimal <%d>, cnt <%d>\n", strlen(buf), c, c, cnt); 
    }
#endif
    

}
#endif
