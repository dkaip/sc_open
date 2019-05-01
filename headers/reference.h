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

#ifndef REFERENCE_H_
#define REFERENCE_H_


#define SC_REFERENCE_1 "\
\n\
-- SC Command line options --\n\
\n\
-c <command>   Any SC command       \n\
-d <level>     Debug level (0-5)\n\
-h             Get help\n\
-i             Interactive mode\n\
-n <name>      Define a name\n\
-r             Show reference\n\
-s <service>   An sc_talk service\n\
-v             Show version\n\
-x             Run syntax checker\n\
\n\
Examples:\n\
    sc -i -c \"read prog1\"\n\
    sc -n driver -s sockets/s.driver -c \"read driver\" >> driver.log\n"

#define SC_REFERENCE_2 "\
-- SC Commands --\n\
\n\
help                   Show help\n\
debug <level>          Set debug level (0-5)\n\
dump                   Dump SC contents\n\
dump *<var>*           Dump SC variable(s) using a wildcard\n\
event <label>          Jump to label\n\
event <label> <arg>    Jump to label w/ vars\n\
kill                   Kill SC\n\
list                   List program\n\
load <program>         Read a program (same as read)\n\
logging                Show current logging\n\
logging <name> <value> Change logging (name is from the open statement)\n\
ping                   Ping SC\n\
read <program>         Read a program (same as load)\n\
set <name> <value>     Create/assign global variable (equiv to: glet name=value)\n\
tokens                 List program as tokens\n\
version                Get version\n"

#define SC_REFERENCE_3 "\
-- Preprocessing  --\n\
\n\
read -P <name>          Enable preprocessing\n\
read -D <name>          Define a name\n\
read -D <name>=<value>  Define a name with a value \n\
read -I <path>          Define an include file path \n\
    sc -c \"read -P prog1\"\n\
    sc -c \"read -I /usr/local/include -D OLD_CODE -D TOOL=NOVA prog1\"\n\
    sc -c \"read -D TOOL='\\\"NOVA 01\\\"'  prog1\"\n\
#include <path>         Define an include path\n\
#define <name>          Define a name\n\
#define <name>=<value>  Define a name with a value \n\
#ifdef <name>           If <name> is defined, then...  \n\
#else                   Optional else for #ifdef \n\
#endif                  End of #ifdef \n"

#define SC_REFERENCE_4 "\
-- Token delimiters --\n\
\n\
Number    0-9 e|E +|-\n\
Name      A-z $ _ . { } [ ]\n\
Misc      = <= >= == != ( ) + - * / & |\n\
\n\
-- Program Statements --\n\
\n\
after <label>                   Define a label\n\
begin <name>                    Begin a subprogram\n\
break                           Stop running\n\
break_loop                      Stop running while/for loops\n\
continue                        Continue within a for/while loop\n\
define <var>                    Define a permanent variable\n\
define <var>=<expr>                 that can't be changed\n\
define_array <var>=<expr1>, <expr2>...)  Define an array\n\
define_xref <var>=<expr1>, <expr2>       Define a cross reference variable\n\
end                             End a subprogram\n\
error=<label>                   Jump to label if statement errors\n\
exit                            Exit SC\n\
export(<file>, <mode>, <expr>)  Write a file\n\
for <var>=<expr> to <expr>      For loop\n\
  [by <expr>]\n\
  next <var>\n\
glet <var> = <expr>             Global assignment\n\
gosub <label>                   Jump to label\n\
gosub <arg1>, <arg2>...		Jump to label with arguments\n\
goto <label>                    Jump to label\n\
if (<expr>)                     If then else statement\n\
  else_if (<expr>)\n\
  else\n\
  end_if\n\
import(<file>, <mode>, <var>)   Read a file\n\
label <label>                   Define label\n\
label <label> <arg1>, <arg2>... Define label used by a gosub with arguments\n\
let <var>=<expr>                Assignment\n\
local_var <var>                 Define a local variable\n\
local_var <var>=<expr>\n\
no_error                        Ignores error if statement errors\n\
pragma <pragma>                 Set a pragma\n\
print <string>                  Print string\n\
return                          Return from a gosub\n\
send name=<name> <msg>          Send a message to a connection\n\
send_reply name=<name> <msg>    Send a reply message \n\
set_program <name>              Run a subprogram\n\
sleep <seconds>                 Sleep for seconds\n\
while <expr>                    While loop\n\
  end_while\n"

#define SC_REFERENCE_5 "\
-- Timers, Starting and stopping program timers --\n\
\n\
start_timer timer=<0->9> seconds=<seconds> handler=<label>\n\
stop_timer timer=<0->9>\n\
Example:\n\
   start_timer timer=1 seconds=10 handler=loop \n\
\n\
-- SC connections --\n\
\n\
open {syntax}        Open a connection\n\
close name=<name>    Close a connection\n\
\n\
{syntax} - Syntax of the open\n\
open {type} name=<name> proto={proto} {connection} logfile={logfile} logging={logging} options={options}\n\
\n\
{type} - Types of connection\n\
    pipe            Unix named pipe\n\
    port            Device port\n\
    socket_client   Client socket connect\n\
    socket_server   Listening socket\n\
\n\
{proto} - Protos to use\n\
    commands    SC commands\n\
    standard    Simple messages\n\
    secs        SECS via SECSI\n\
    hsms        SECS via HSMS\n\
\n\
{connection} - Connections to open\n\
    device=<device>                  Serial device\n\
    host=<host> service=<service>    Remote socket\n\
    local_name=<local_name>          Local socket\n\
\n\
{logging} - Sets the logging level\n\
    logging=0  Turn off\n\
    logging=1  One line summary\n\
    logging=2  Full message (default)\n\
    logging=3  Protocal details\n\
\n\
{logfile} - Logfile for logging\n"

#define SC_REFERENCE_6 "\
Example: Scanner connected to a serial port on the host\n\
    open port name=scanner proto=standard device=\"/dev/tty0p0\" options={standard options} \n\
\n\
Example: Secs equipment connected to xyplex port 1\n\
    open socket_client name=equipment\n\
        proto=secs host=xyplex_1 service=2101\n\
        options={secs options}\n\
\n\
Example: Driver's listening connection for host commands\n\
    open socket_server name=driver\n\
        local_name=\"sockets/.driver\" proto=standard \n\
        options={standard options}\n\
\n\
{standard options}\n\
    baud=300|600|1200|2400|4800|9600|19200|38400       \n\
    flush=<time>       \n\
    no_parse=<str>     \n\
    rcv_eol=<str>      \n\
    timeout_msg=<str>  \n\
    timeout=<time>     \n\
    wait_for_reply=no|yes    Default is yes\n\
    xmt_eol=<str>      \n\
\n\
Example:\n\
    options=\"baud=9600 flush=5 no_parse=\"-\" rcv_eol=\"\\r\" \n\
        timeout_msg=\"TIMEOUT\" \n\
        timeout=30 wait_for_reply=yes xmt_eol=\"\\r\" \"\n\
\n\
{secs options}\n\
    baud=300|600|1200|2400|4800|9600|19200|38400       \n\
    arrays=yes|no    Default is yes \n\
    flush=<time>       \n\
    rbit=<value>       \n\
    retry=<value>       \n\
    secsI_logging=off|on    Default is on  (Old, use logging=x instead) \n\
    t1=<time>       \n\
    t2=<time>       \n\
    t3=<time>       \n\
    t4=<time>       \n\
    timeout_msg=str       \n\
\n\
Example:\n\
   options=\"baud=9600 flush=5 rbit=0 secsI_logging=off\n\
       t1=3 t2=3 t3=30 t4=30 timeout_msg=\"TIMEOUT\" \"\n\
\n\
{hsms options}\n\
   flush=<time>       \n\
   arrays=yes|no    Default is yes \n\
   hsms_logging=off|on    Default is off (Old, use logging=x instead)  \n\
   t3=<time>       \n\
   t6=<time>       \n\
   t7=<time>       \n\
   t8=<time>       \n\
   timeout_msg=<str>       \n\
\n\
Example:\n\
    options=\"flush=5 hsms_logging=off t3=30 t6=10 t7=10 t8=5\n\
        timeout_msg=\"TIMEOUT\" \"\n"

#define SC_REFERENCE_7 "\
-- secs messages --\n\
\n\
send name=<name> s<stream>f<function> <device id> [w] {body}.\n\
\n\
Example of S2F41:\n\
    send name=tool S2F41 0 W \n\
    <L \n\
       <A START>\n\
       <L>\n\
    >. \n\
\n\
Example of S2F37 using a for/next loop:\n\
    send name=tool S2F37 0 W \n\
    <L \n\
       <BO TRUE>\n\
       <L for i=1 to 10\n\
           <U4 $ceid[$i]>\n\
       next i\n\
       >\n\
    >. \n\
\n\
{secs data types} \n\
    L                 List\n\
    A                 Ascii\n\
    B                 Binary, 00-FF\n\
    BO                Boolean, TRUE|FALSE\n\
    U1, U2, U4, U8    Unsigned integers\n\
    I1, I2, I4, I8    Signed integers\n\
    F4, F8            Floats\n"

#define SC_REFERENCE_8 "\
-- Functions -- FALSE=0, TRUE=None 0\n\
\n\
abs(value)                    Returns the absolute value\n\
btoi(value)                   Returns binary value as integer\n\
dump()                        Same as the dump command\n\
file_close(fd)                File close\n\
file_open(file, mode)         Returns fd.  See 'C' fopen(). Ex: fd=file_open(\"data\", \"w+\")  \n\
file_owc(file, mode, data)    File open, write, close\n\
file_read(fd)                 Returns data or EOF       \n\
file_write(fd, data)          Write data to file\n\
format(str, fmt)              Format str, fmt is %<fmt>g or %<fmt>s by default, or user defined\n\
get_env(var)                  Returns environment var\n\
get_tmp_var()                 Returns all temporary variables\n\
get_tmp_var(var)              Returns temporary variable\n\
get_tmp_var(var, default)     Returns temporary variable or default\n\
get_version()                 Returns the current version of sc\n\
htoi(value)                   Returns hex value as integer       \n\
instring(str1, str2)          Returns idx of str2 in str1 or 0\n\
is_label(label)               Returns TRUE if label exists\n\
is_tmp_var(var)               Returns TRUE if temporary var exists\n\
is_var(var)                   Returns TRUE if var exists\n\
itoa(value)                   Returns printable ascii char or ? \n\
itob(value, fmt)              Returns integer value as binary\n\
itoh(value, fmt)              Returns integer value as hex\n\
itoo(value, fmt)              Returns integer value as octal\n\
mod(x, y)                     Returns remainder of x/y\n\
otoi(value)                   Returns octal value as integer\n\
parse(str, del, data)         Returns cnt of data[x] created from str parsed by delimiters in del\n\
parse(str, del)               Returns cnt of items only\n\
pow(x, y)                     Returns x raise to y\n\
scan(str, fmt)                Returns scan or str and fmt\n\
set_tmp_var(var, value)       Sets temporary var to value\n\
sqrt(value)                   Returns quare root of value\n\
strftime(fmt, [value])        Returns formatted time\n\
strlen(str)                   Returns string length\n\
substr(str, idx, cnt)         Returns sub string         \n\
system(cmd)                   Returns results of 'C' popen()\n\
system(cmd, no_parse)         Same as above, but includes a no_parse string\n\
system_shell(cmd)             Returns results of 'C' system()\n\
t_parse(str, del, data)       Returns cnt of temp data[x] created from str parsed by delimiters in del\n\
test(str)                     Returns results of 'C' test()\n\
time()                        Returns time integer\n\
tolower(str)                  Returns lower case of str\n\
toupper(str)                  Returns upper case of str\n\
trace(0|1)                    Turns tracing on and off, 0=Off, 1=On\n"

#define SC_REFERENCE_9 "\
-- Operators --\n\
\n\
{Math}\n\
    +      Addition\n\
    -      Subtraction\n\
    *      Multiplication\n\
    /      Division\n\
{Numeric   Relational}\n\
    ==     Equal to\n\
    !=     Not equal to\n\
    >      Greater than\n\
    <      Less than\n\
    >=     Greater than or equal to\n\
    <=     Less than or equal to\n\
{String Relational}\n\
    eq     Equal to\n\
    ne     Not equal to\n\
{Boolean}\n\
    and    Boolean and\n\
    or     Boolean or\n\
{Bitwise}\n\
    &      Bitwise and-ing\n\
    |      Bitwise or-ing\n\
    ^      Bitwise xor-ing\n\
{Unary}\n\
    -      Minus\n\
    +      Plus\n\
    ~      One's Complement\n\
\n\
-- Constants --\n\
\n\
Whole number      100, 200, -6\n\
Floating point    1.2, 5.12\n\
Exponential       -6.12e+12, 3.3E-2\n\
Strings           \"This is a string\"                  \n\
                  \"The value of x is <\" . $x . \">\"  \n\
                  \"Ring the bell \\x07\"               \n\
                  \"Print an extra new line \\n\"       \n\
                  \"Print a carriage return \\r\"       \n\
\n\
    String Concatenation\n\
    \"str1\" . \"str2\"    Append str2 to str1\n\
    \"str1\" : \"str2\"    Append str2 to str1 separated by a space\n\
\n\
-- Pragmas --\n\
\n\
    LOG_CPU_USAGE                        Logs cpu time used\n\
    SHOW_NON_PRINTING_CHARS_IN_BRACES    Prints non-printing chars as {hh}\n\
    NO_BLOCK_ON_SLEEP                    Don't block on the sleep command\n\
    NO_SOURCE_CODE_LINE_NUMBERS          Don't show source code filename and line numbers\n\
"

#endif /* REFERENCE_H_ */

