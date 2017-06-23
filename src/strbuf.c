#include "../include/strbuf.h"

Strbuf *
strbuf_new(void)
{
   Strbuf *buf = calloc(1, sizeof(Strbuf));
   if (!buf) 
     return NULL;

   return buf;
}

void
strbuf_append(Strbuf *buf, const char *string)
{
   ssize_t len;
   if (!buf)
     return;

   len = strlen(string);

   buf->len += len;

   buf->data = realloc(buf->data, buf->len);

   memcpy(&buf->data[buf->len - len], string, len);
}

void
strbuf_append_printf(Strbuf *buf, const char *fmt, ...)
{
   char tmp_buf[4096];
   va_list ap;
 
   va_start(ap, fmt);
   vsnprintf(tmp_buf, sizeof(tmp_buf), fmt, ap);
   va_end(ap);
   strbuf_append(buf, tmp_buf);
}

const char *
strbuf_string_get(Strbuf *buf)
{
   if (!buf || !buf->len) 
     {
        buf->data = strdup("");
        buf->len = 0;
        return buf->data;
     }

   buf->data[buf->len] = '\0';

   return buf->data;
}

void strbuf_trim(Strbuf *buf, ssize_t start)
{
   if (!buf) return;

   if (buf->len < start) return;

   buf->len = start;
   buf->data[start] = '\0';
}

void strbuf_reset(Strbuf *buf)
{
   if (!buf) return;

   if (buf->data)
     {
        free(buf->data);
        buf->data = NULL;
     }
   
   buf->len = 0;
}

void strbuf_free(Strbuf *buf)
{
   if (!buf) return;

   if (buf->data)
     free(buf->data);

   free(buf);
   buf = NULL;
}

