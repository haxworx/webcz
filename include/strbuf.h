#ifndef __STRBUF_H__
#define __STRBUF_H__
#define _POSIX_C_SOURCE 200809L

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>

typedef struct _Strbuf {
   char *data;
   ssize_t len;
} Strbuf; 

Strbuf * strbuf_new(void);

void strbuf_append(Strbuf *buf, const char *string);

void strbuf_append_printf(Strbuf *buf, const char *fmt, ...);

const char * strbuf_string_get(Strbuf *buf);

void strbuf_trim(Strbuf *buf, ssize_t start);

void strbuf_reset(Strbuf *buf);

void strbuf_free(Strbuf *buf);

#endif 
