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

#ifndef LOG_H_
#define LOG_H_

#define LOG \
if (g_syntax_check == 0) log_clear_fp(); \
if (g_syntax_check == 0) log_msg_hdr

#define FD_LOG0(_name) \
if (get_logging(_name) == 0) log_set_fp(_name); \
if (get_logging(_name) == 0) log_msg

#define FD_LOG1(_name) \
if (get_logging(_name) >= 1) log_set_fp(_name); \
if (get_logging(_name) >= 1) log_msg_hdr("(%s) ", show_proto(get_proto(_name))); \
if (get_logging(_name) >= 1) log_msg 

#define FD_LOG2(_name) \
if (get_logging(_name) >= 2) log_set_fp(_name); \
if (get_logging(_name) >= 2) log_msg_hdr("(%s) ", show_proto(get_proto(_name))); \
if (get_logging(_name) >= 2) log_msg 

#define FD_LOG3(_name) \
if (get_logging(_name) >= 3) log_set_fp(_name); \
if (get_logging(_name) >= 3) log_msg_hdr("(%s) ", show_proto(get_proto(_name))); \
if (get_logging(_name) >= 3) log_msg 

#define LOG_DATA(_name) \
log_set_fp(_name); \
log_data

#endif /* LOG_H_ */

