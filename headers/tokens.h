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

#ifndef TOKENS_H_
#define TOKENS_H_

enum token_types
{
    TT_SPECIAL=1, 
    TT_NULL_TOKEN, 
    TT_END_OF_LINE, 
    TT_VARIABLE, 
    TT_IDENTIFIER, 
    TT_STRING, 
    TT_CONSTANT, 
    TT_OPERATOR, 
    TT_NUMBER, 
    TT_LINE_NUMBER, 
    TT_POUND_INCLUDE, 
    TT_POUND_DEFINE, 
    TT_POUND_IFDEF, 
    TT_POUND_ELSE, 
    TT_POUND_ENDIF, 
    TT_COMMENT, 
    TT_UNKNOWN,

    TT_AFTER,
    TT_BREAK,
    TT_BREAK_LOOP,
    TT_BEGIN,
    TT_CONTINUE,
    TT_CLOSE,
    TT_DEFINE,
    TT_DEFINE_ARRAY,
    TT_DEFINE_XREF,
    TT_LOCAL_VAR,
    TT_DELETE,
    TT_END,
    TT_ELSE,
    TT_ELSE_IF,
    TT_END_IF,
    TT_END_WHILE,
    TT_ERROR,
    TT_EXIT,
    TT_FOR,
    TT_GLET,
    TT_GOSUB,
    TT_GOTO,
    TT_IF,
    TT_LABEL,
    TT_LET,
    TT_NEXT,
    TT_NO_ERROR,
    TT_OPEN,
    TT_PRAGMA,
    TT_PRINT,
    TT_RETURN,
    TT_SEND,
    TT_SEND_REPLY,
    TT_SET_PROGRAM,
    TT_SLEEP,
    TT_START_TIMER,
    TT_STEP,
    TT_STOP_TIMER,
    TT_THEN,
    TT_TO, 
    TT_WHILE, 

    TT_EQUAL,
    TT_ESC,
    TT_ASSIGN,
    TT_BANG,
    TT_COLON,
    TT_GREATER_THAN,
    TT_GREATER_THAN_EQUAL,
    TT_LESS_THAN,
    TT_LESS_THAN_EQUAL,
    TT_NOT_EQUAL,
    TT_BITWISE_AND,
    TT_CLOSE_PAREN,
    TT_COMMA,
    TT_DIVIDE,
    TT_LOGICAL_AND, 
    TT_LOGICAL_OR,
    TT_MINUS,
    TT_UNARY_MINUS,
    TT_MULT,
    TT_OPEN_PAREN,
    TT_BITWISE_OR,
    TT_BITWISE_XOR,
    TT_UNARY_ONES_COMPLEMENT,
    TT_PLUS,
    TT_UNARY_PLUS,
    TT_PERIOD,
    TT_STR_EQUAL,
    TT_STR_NOT_EQUAL,

    TT_X,
    TT_FUNCTION,  /* This must go outside the TT_FUNCTION_MIN and TT_FUNCTION_MAX */

    /* All functions go between TT_FUNCTION_MIN and TT_FUNCTION_MAX */

    TT_FUNCTION_MIN,

    TT_SCAN,
    TT_SUBSTR,
    TT_SQRT,
    TT_STRFTIME,
    TT_STRLEN,
    TT_GET_TMP_VAR,
    TT_GET_VERSION,
    TT_FILE_CLOSE,
    TT_FILE_OPEN,
    TT_FILE_OWC,
    TT_FILE_READ,
    TT_FILE_WRITE,
    TT_FORMAT,
    TT_DUMP,
    TT_TRACE,
    TT_DEL_TMP_VAR,
    TT_SORT_TMP_VAR,
    TT_ADD_TMP_VAR,
    TT_SET_TMP_VAR,
    TT_HTOI,
    TT_OTOI,
    TT_ITOH,
    TT_IMPORT,
    TT_FTOH,
    TT_HTOF,
    TT_SWAP,
    TT_BTOI,
    TT_ITOA,
    TT_ITOB,
    TT_ABS,
    TT_ITOO,
    TT_INSTRING,
    TT_IS_LABEL,
    TT_IS_TMP_VAR,
    TT_IS_VAR,
    TT_GET_CLIENT,
    TT_GET_ENV,
    TT_SYSTEM,
    TT_SYSTEM_SHELL,
    TT_TEST,
    TT_TIME,
    TT_TOLOWER, 
    TT_TOUPPER, 
    TT_MOD,
    TT_T_PARSE,
    TT_PARSE,
    TT_POW,
    TT_EXPORT,

    TT_FUNCTION_MAX,

	TT_NOT_ASSIGNED_YET,
    TT_END_OF_LIST
};

#endif /* TOKENS_H_ */

