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

#ifndef VERSION_H_
#define VERSION_H_

#define SC_VERSION "2.6.0"
#define SC_DATE "05/01/2019"

#define SC_HISTORY \
"Revision History \n"\
"2.6.0 05/01/2019\n"\
"- Worked through code and cleaned up so that it would compile with the standard set to gnu18.\n"\
"- \n"\
"2.5.1 12/25/2016\n"\
"- Worked through code and cleaned up so that it would compile with the standard set to c11.\n"\
"- Fixed various minor bugs.\n"\
"- \n"\
"2.2.5 2/15/06\n"\
"- Still don't have the substr() function working properly.  This version \n"\
"  should fix this. \n"\
"- \n"\
"2.2.4 2/14/06\n"\
"- Fixed a bug introduced in 2.2.3 with the esc char removing esc-ed char\n"\
"- \n"\
"2.2.3 2/9/06\n"\
"- Fixed a bug where within a double quoted string, and you have an <esc>nl, \n"\
"  you got the \"Missing quote\" error msg.  Now its ok to have this.  \n"\
"- Fixed a bug with substr(str, idx, cnt), if the cnt is greater then the length\n"\
"  of the str, it was grabbing garbage chars.  Now it is limited to the str data \n"\
"- \n"\
"2.2.2 2/6/06\n"\
"- Fixed bug with sc not running under unix properly \n"\
"- Fixed 2 typo's in log messages \n"\
"- Fixed bug in secs message, where the tmp var WAIT_BIT had an \"=\" sign in its value\n"\
"- Fixed buf in hsms message, where the tmp var SXFY=DEVICE_ID is changed to the actual stream and function \n"\
"  ex: was: SXFY.DEVICE_ID now: S1F3.DEVICE_ID \n"\
"- \n"\
"2.2.1 1/25/06\n"\
"- Modified how tmp variables are initially created, old way created one a a time\n"\
"  New way, creates 1000 at a time \n"\
"- Modified how hsms msgs are read, old way was one char at a time, \n"\
"  New way attempts to read up to 1k blocks\n"\
"- Fixed bug with get_tmp_var() function, where var's with vals equal to \"\", were not returned \n"\
"  The \"dump\" cmd worked OK\n"\
"- Added a LOG msg when reading hsms msgs after the read is complete and before decoding happens\n"\
"- \n"\
"2.2.0\n"\
"- Modified the encoding and decoding of secs messages for performance\n"\
"- Modified sc to support the following preprocessing program statements: \n"\
"  #include (Include file)\n"\
"  #define name (define a name)\n"\
"  #define name=value (define a name with a value)\n"\
"  #ifdef name, #else, and  #endif (Conditionally include program statements)\n"\
"- Modifed the sc read command to include preprocessing options: \n"\
"  -P (turn on preprocessing)\n"\
"  -D name (define a name)\n"\
"  -D name=value (define a name with a value)\n"\
"  -I path (define a path for include files)\n"\
"- Modifed the sc commands to allow for abbreviated usage: read, list, kill, etc...\n"\
"- Supports C style comments in sc programs (/*... */) \n"\
"- Added sc check for missing closing (\")s and (')s\n"\
"- \n"\
"2.1.1\n"\
"- Changed how internal sc debugging works\n"\
"- Allows the F8 data types in secs messages to use \\xhhh..hh format\n"\
"- \n"\
"2.1\n"\
"- See the release notes in the User's Guide\n"\
"- \n"\
"2.0.6\n"\
"- Added a cpu usage LOG statement when sc goes idle\n"\
"- Fixed a bug with secs/hsms msgs and the W bit flag where sc was waiting for a reply \n"\
"  even when the W bit was not set \n"\
"- Fixed a bug with the unary +\n"\
"- Added the ~ one's complement unary operator\n"\
"- Fixed a bug where the command line arguments were generating errors if not enough supplied\n"\
"- \n"\
"2.0.5\n"\
"- Fixed a bug where function calls within fucntion calls were not working.  This bug was introduced\n"\
"  a few versions ago. \n"\
"- \n"\
"2.0.4\n"\
"- New command \"load\" that works just like the old \"read\" command (included in version 2.0.3)\n"\
"- New operand \"^\" for bitwise xor-ing\n"\
"- Fixed bug with the \"open\" command and the \"error=<>, no_error\", this did not work before\n"\
"- \n"\
"2.0.3\n"\
"- Mod so that tmp variables are kept separate form others.  Did this for performance reasons.\n"\
"- Fixed an old bug where is_tmp_var() returned true when the tmp variable did not exist.\n"\
"- Fixed core dump for \"sc -i -c \" , the -c option has no argument\n"\
"- Fixed bug with no_error within a gosub, and not being able to return\n"\
"- \n"\
"2.0.2\n"\
"- Fixing sc looping problem with connected client\n"\
"- Mod the disconnect routine in server.c.  This had some bugs\n"\
"  handling clients and servers that terminate.\n"\
"- Mod how the hsms sends messages, This was causing some problems with tools\n"\
"- \n"\
"2.0.1\n"\
"- Fixed bug for program files that have the <cr><nl>.  This is treated as a single <nl>\n"\
"- Fixed a sc -r  mistake about the break/break_loop statments\n"\
"- Now supports the // style comments\n"\
"- Fixed a bug in the htof() function\n"\
"- \n"\
"2.0\n"\
"- Open Source version\n"\
"- \n"\
"\n"

#endif /* VERSION_H_ */

