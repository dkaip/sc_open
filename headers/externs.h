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

#include <setjmp.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#include "binary_buffer.h"


#ifndef EXTERNS_H_
#define EXTERNS_H_


/* External global variables */

extern uint32_t g_debug;
extern uint32_t g_trace;
extern bool g_syntax_check;
extern char g_name[];

/* Tune parameters */

extern uint32_t t_tv_sec;
extern uint32_t t_tv_usec;

/* buffer.c */
extern void buffree(struct buffer *);
extern void bufncpy(struct buffer *, const char *, uint32_t);
extern void bufcpy(struct buffer *buf, const char *data);
extern void buf2bufcpy(struct buffer *, const struct buffer *);
extern void bufncat(struct buffer *buf, const char *data, uint32_t len);
extern void bufcat(struct buffer *buf, const char *data);
extern char *bufdata(const struct buffer *);
//extern void bufdump_hex(const struct buffer *);
//extern void bufdump_fmt(struct buffer *, struct buffer *);
extern uint32_t bufsize(const struct buffer *);
extern uint32_t buffilecpyread(struct buffer *, const char *);
extern int buffilecpywrite(const struct buffer *, const char *);
extern void bufprefix(struct buffer *, const char *);
extern void bufnlcpy(struct buffer *, const char *);
extern int bufcmp(const struct buffer *, const char *);
extern int bufncmp(const struct buffer *, const char *, uint32_t);
extern void buftoupper(struct buffer *buf);
extern void buftolower(struct buffer *buf);
extern void bufrtrim(struct buffer *buf);
extern const char *bufmkprint(const struct buffer *buf);
extern const char *strmkprint(const char *, uint32_t);

/* error.c */
extern void log_error(const char *, ...);
extern void log_error_hdr(const char *, ...);

/* pp_error.c */
extern void log_pp_error(const char *, ...);
extern void log_pp_error_hdr(const char *, ...);

/* warning.c */
extern void log_warning(const char *, ...);
extern void log_warning_hdr(const char *, ...);

/* debug.c */
extern void log_debug(const char *fmt, ...);
extern void log_debug_hdr(const char *fmt, ...);
extern void debug_init();
//extern void debug(int, char *, char *, char *, int, const char *fmt, ...);
extern void debug(uint32_t level, const char *id, const char *filename, const char *name, int line_number, const char *fmt, ...);

/* tokens.c */
extern void set_token(struct buffer *);
extern void get_token(struct buffer *, enum token_types *, bool);
extern void get_token_eval(struct buffer *, enum token_types *);
extern void get_token_toupper(struct buffer *, enum token_types *);
extern char *get_token_ptr();
extern void set_token_ptr(char *);
extern void put_token();
extern bool is_operator(enum token_types);
extern bool is_function(enum token_types);
extern void evaluate_token(struct buffer *);
extern char *show_type(enum token_types);
extern void show_all_types();
/* extern enum token_types get_token_type(char *); */

/* proto.c */
extern enum protos lookup_proto(char *);
extern const char *show_proto(enum protos);

/* commands.c */
extern uint32_t commands(struct buffer *buffer);
//extern int commands(struct buffer *);

/* execute.c */
extern uint32_t execute(struct buffer *);
//extern uint32_t get_error_token(int *);
extern const char *get_line_number();
extern void set_line_number(const char *);

/* expression.c */
extern uint32_t expression(struct buffer *);

/* for.c */
extern uint32_t cmd_for();
extern uint32_t cmd_next();
extern void for_reset(bool);

/* gosub.c */
extern uint32_t cmd_gosub();
extern uint32_t cmd_return();
extern void gosub_reset(bool);

/* goto.c */
extern uint32_t cmd_goto();

/* if.c */
extern uint32_t cmd_if();
extern uint32_t cmd_else();
extern uint32_t cmd_else_if();

/* while.c */
extern uint32_t cmd_while();
extern uint32_t cmd_end_while();
extern void while_reset(bool);
extern void while_break();

/* let.c */
extern uint32_t cmd_let();
extern uint32_t cmd_glet();

/* pragma.c */
extern uint32_t cmd_pragma();
extern void dump_pragma(struct buffer *);
extern int is_pragma(int);

/* label.c */
extern int label_build_idx_tabel();
extern char *label_get(uint32_t);

/* fatal.c */
extern void log_fatal(const char *, ...);
extern void log_fatal_hdr(const char *, ...);

/* handler.c */
extern uint32_t handler(int s_fd, int c_fd);
//extern uint32_t handler(int fd, int fd);
extern void init_handler();
extern char *get_option(const char *, const char *);
extern char *get_name(int);
extern FILE *get_logfile_fp(const char *);
extern int get_logging(const char *);
extern void set_logging(const char *, int);
extern void show_logging(struct buffer *);
extern void set_fd_info(char *, int, char *, enum protos, int, FILE *);
extern void override_options(char *, char *);
extern int get_fd(const char *);
extern void dump_handler (struct buffer *);
extern char *get_opt(const char *, const char *);
extern enum protos get_proto(const char *);
extern void update_fd_info(const char *, int);
extern int get_client_fd(char *);

/* io.c */
extern uint32_t my_read(const char *, int, char *, uint32_t, uint32_t *);
extern uint32_t my_readc(const char *name, int fd, uint8_t *buf);
//extern uint32_t my_readc(const char *, int, uint8_t *);
extern int64_t my_readx(const char *, int, uint8_t *, uint32_t);
extern uint32_t my_write(const char *name, int fd, const uint8_t *buf, uint32_t size);
extern uint32_t my_writec(const char *, int, uint8_t);

/* log.c */
extern void log_set_fp(const char *);
extern void log_clear_fp();
extern void log_msg(const char *fmt, ...);
extern void log_msg_hdr(const char *fmt, ...);
//extern void log_data(const char *, const uint8_t *, uint32_t);
extern void log_data(const char *msg, const uint8_t *data, uint32_t size);

/* open.c */
extern uint32_t cmd_open();
extern uint32_t cmd_close();

/* sockets.c */
extern void init_sockets();
extern int open_socket_client(char *, char *);
extern int open_socket_server(char *, char *);
extern int open_comm_server(char *, char *);
extern int open_socket_server_local(char *);
extern int open_socket_server_local(char *);

/* parse.c */
extern uint32_t parse(struct buffer *token, const char *buf, enum parse_types *type,
    const char *no_parse, bool comment_flag, bool pp_flag);
//extern int parse(struct buffer *, char *, enum parse_types *, char *, int, int);
extern const char *show_parse_type(enum parse_types type);

/* port.c */
extern uint32_t open_port(const char *, const char *, enum protos, const char *, int *);

/* print.c */
extern uint32_t cmd_print();

/* program.c */
extern uint32_t read_program(const char *buf);
extern uint8_t *ubyte_type(enum token_types type);
extern uint32_t cmd_set_program();
extern char *get_program_name();
extern char *get_default_program_name();
extern void list_program(struct buffer *buffer);
extern void dump_program_tokens(struct buffer *);
extern void get_program_token(struct buffer *, enum token_types *);
extern void get_program_token_no_eval(struct buffer *, enum token_types *);
extern void put_program_token();
extern uint32_t get_program_token_idx();
extern void set_program_token_idx(uint32_t);

/* resume.c */
//extern uint32_t resume(const char *, uint32_t);
extern uint32_t resume_via_program_label(const char *program_label);
extern uint32_t resume_via_program_idx(uint32_t program_index);

/* variables.c */
extern void variable_put(const char *, const char *, uint32_t, enum var_types);
extern uint32_t variable_get(const char *, struct buffer *, enum var_types);
extern void variable_delete(const char *);
extern void variable_dump(struct buffer *, char *);
extern void variable_dump_all(struct buffer *);
extern void variable_delete_all();

/* tmp_variables.c */
extern void tmp_variable_put(const char *, const char *, uint32_t);
extern void tmp_variable_add(const char *, const char *, int);
extern void tmp_variable_sort();
extern uint32_t tmp_variable_get(const char *, struct buffer *);
extern void tmp_variable_dump(struct buffer *, const char *);
extern void tmp_variable_dump_all(struct buffer *);
extern void tmp_variable_delete_all();
//extern int tmp_variable_get_all(struct buffer *);
extern void tmp_variable_get_all(struct buffer *);
extern void make_tmp_variables(const char *, const char *, uint32_t);

/* timers.c */
extern uint32_t cmd_start_timer();
extern uint32_t cmd_stop_timer();
extern void timer_handler();
//extern int start_timer(int, time_t, int, const char *, const char *);
extern uint32_t start_timer(int timer_idx, time_t seconds, uint32_t program_idx, const char *msg, const char *comment);
extern void stop_timer(uint32_t);

/* swap.c */
extern void swap(uint8_t *data, uint32_t size);

/* sleep.c */
extern uint32_t cmd_sleep(bool *);

/* break.c */
extern int cmd_break();
//extern int cmd_break_loop();
extern void cmd_break_loop();

/* server.c */
extern void server();
extern void init_server();
extern void set_server(int, bool);
extern uint32_t unset_server(int fd);
extern void kill_server();
extern char *get_name(int);
//extern int get_fd(char *);
extern void dump_server(struct buffer *);
extern void dump_server(struct buffer *);
extern void set_server_timeout(uint32_t value);
extern void client_disconnect(const char *, int);

/* send_secs.c */
extern uint32_t send_secs(const char *, int, bool *);
extern uint32_t send_secs_reply(const char *, int);

/* send_hsms.c */
extern uint32_t send_hsms(const char *, int, bool *);
extern uint32_t send_hsmsI(const char *name, int fd, uint8_t *header, const uint8_t *body, uint32_t size);
extern uint32_t send_hsms_reply(const char *name, int fd);

/* send.c */
extern uint32_t cmd_send(bool *);
extern uint32_t cmd_send_reply();

/* hsms.c */
extern uint32_t hsms_client_connect(const char *, int fd);

/* secs.c */
extern void secs_calc_checksum(const uint8_t *, uint32_t, uint8_t *);
extern uint32_t secs_get_system_bytes();
extern void secs_set_system_bytes(uint32_t);
extern uint32_t secs_inc_system_bytes();
//extern void open_trans_table_add(int, unsigned int, int, int);
extern void open_trans_table_add(uint32_t program_idx, uint32_t system_bytes, uint32_t timer_idx, uint32_t seconds);
extern uint32_t open_trans_table_del(uint32_t, uint32_t *);

/* proto_timers.c */
//extern int start_proto_timer(int, int, sigjmp_buf *);
#ifdef __CYGWIN__
extern uint32_t start_proto_timer(uint32_t seconds, uint32_t idx, sigjmp_buf *env_alrm);
#else
extern uint32_t start_proto_timer(uint32_t seconds, uint32_t idx, jmp_buf *env_alrm);
#endif
//extern int start_proto_timer(int, int, jmp_buf *);
extern uint32_t stop_proto_timer(uint32_t);
extern void init_proto_timers();

/* recv_secs.c */
//extern uint32_t recv_secs(const char *, int, bool, int *, bool *);
extern uint32_t recv_secs(const char *name, int fd, bool flag, uint32_t *program_idx, bool *primary_msg_flag);
#ifdef __CYGWIN__
extern int do_the_setsigjump(sigjmp_buf the_jump_buffer, int save_mask);
#else
extern int do_the_setsigjump(jmp_buf the_jump_buffer, int save_mask);
#endif

/* recv_hsms.c */
//extern uint32_t recv_hsms(const char *, int fd, uint32_t *, bool *);
extern uint32_t recv_hsms(const char *name, int fd, uint32_t *program_idx, bool *primary_msg_flag);
extern uint32_t recv_hsmsI(const char *name, int fd, uint8_t *header, struct binary_buffer *body);

/* secs_encode_sml.c */
extern uint32_t secs_encode(struct binary_buffer *, uint32_t *, uint32_t *, uint32_t *, uint32_t *, struct buffer *);

/* secs_decode_sml.c */
extern uint32_t secs_decode(const char *, const struct binary_buffer *, struct buffer *);

/* secs_decode_sml_no_arrays.c */
extern uint32_t secs_decode_no_arrays(const char *, const struct binary_buffer *, struct buffer *);

/* send_standard.c */
extern uint32_t send_standard(const char *, int);
extern uint32_t send_standard_reply(const char *, int);
extern uint32_t recv_standard(const char *, int, struct buffer *);

/* send_xml.c */
extern uint32_t send_xml(const char *, int);
extern uint32_t send_xml_reply(const char *, int);
extern uint32_t recv_xml(const char *, int, struct buffer *);

/* convert_data.c */
extern void char_to_data(struct buffer *, const char *);
extern void data_to_char(struct buffer *, const char *, uint32_t);

/* env.c */
extern char *add_sockets_path(char *);

/* tune.c */
extern void tune();

/* stubs.c */
extern uint32_t mbx_open_server(const char *, const char *, enum protos);
extern uint32_t mbx_open_client(const char *, const char *, enum protos);
extern uint32_t mbx_open(const char *, const char *, enum protos);
extern void dump_mbx(struct buffer *);
extern uint32_t mbx_send(const char *, enum protos);
extern uint32_t mbx_send_reply(const char *, enum protos);
extern void user_sigusr1_handler();
extern void user_sigusr1_setup();

/* syntax.c */
extern int syntax_check(char *);

/* sc.c */

/* utility.c */
extern const char *my_itoa(int64_t);
extern const char *my_itoa_fmt1(int64_t);
extern int64_t   my_atoi(const char *, int *);
extern uint64_t  my_atou(const char *, int *);
extern uint8_t my_atobyte(const char *, int *);
extern const char *my_utoa(uint64_t);
extern char *my_btoh(unsigned char);
extern char *my_btoh_fmt1(unsigned char);

/* binary_buffer.c */
extern void binary_buffree(struct binary_buffer *buffer);
extern void binary_bufncat(struct binary_buffer *binbuf, const uint8_t *data, uint32_t length);
extern void binary_bufncpy(struct binary_buffer *buffer, const uint8_t *data, uint32_t length);
extern void binary_buf_empty(struct binary_buffer *buffer);
extern uint8_t *binary_buffer_data(const struct binary_buffer *buffer);
extern uint32_t binary_buffer_num_data(const struct binary_buffer *buffer);
extern void binary_bufdump_hex(const struct binary_buffer *binbuf);


#endif /* EXTERNS_H_ */

