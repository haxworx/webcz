#include "../include/webcz.h"
#include "../include/strbuf.h"
#include <openssl/md5.h>

Web_Cz *_web_cz_global_object = NULL;

Web_Cz *
web_cz_global_object_get(void)
{
   return _web_cz_global_object;
}

static void *
_param_add(const char *name, const char *value)
{
   param_t *c;
   Web_Cz *self = web_cz_global_object_get();

   if (!name || !name[0] || !value || !value[0])
     return self->parameters;

   c = self->parameters;

   if (!c)
     {
        self->parameters = c = calloc(1, sizeof(param_t));
        c->name = strdup(name);
        c->value = strdup(value);
        c->next = NULL;
        return self->parameters;
     }

   for (c = self->parameters; c->next; c = c->next);

   if (!c->next)
     {
        c->next = calloc(1, sizeof(param_t));
        c = c->next;
        c->name = strdup(name);
        c->value = strdup(value);
        c->next = NULL;
        return self->parameters;
     }

   return NULL;
}

char *
web_cz_param(const char *name)
{
   param_t *c;
   Web_Cz *self = web_cz_global_object_get();
   if (!self)
     return NULL;

   if (!self->cgi.have_request)
     {
        self->cgi.get();
     }
   c = self->parameters;
   while (c)
     {
        if (!strcmp(c->name, name))
          return c->value;
        c = c->next;
     }
   return NULL;
}

static void
_parse_request(char *req)
{
   char *pos, *start, *end;

   pos = req;

   while (1)
     {
        start = pos;
        if (!start)
          break;
        char *name = start;
        pos = strchr(start, '=');
        if (pos)
          *pos = '\0';
        pos++;
        char *value = pos;
        end = strchr(pos, '&');
        if (end)
          *end = '\0';
        _param_add(name, value);
        if (!end)
          break;
        pos = end + 1;
     }
}

bool web_cz_get(void)
{
   Web_Cz *self;
   ssize_t buffer_size, len;
   char buf[CHUNK];
   const char *method;
   char *buffer = NULL;

   method = getenv("REQUEST_METHOD");
   if (!method)
     return false;

   self = web_cz_global_object_get();
   if (!self)
     return false;

   buffer_size = 0;

   if (!strcasecmp(method, "POST"))
     {
        while (1)
          {
             len = read(STDIN_FILENO, buf, sizeof(buf));
             if (len <= 0)
               break;
        
             buffer_size += len;

             buffer = realloc(buffer, 1 + buffer_size * sizeof(char));
             memcpy(&buffer[buffer_size - len], buf, len);
          }
       
          buffer[buffer_size] = '\0';
          close(STDIN_FILENO);
     }
   else if (!strcasecmp(method, "GET"))
     {
        buffer = strdup(getenv("QUERY_STRING"));
     }
   else
    {
       return false;
    }

   if (buffer[0] && buffer[1])
     _parse_request(buffer);

   if (buffer)
     free(buffer);

   self->cgi.have_request = true;

   return true;
}

char *
_cookie_optional(char *start, const char *key)
{
   char *value, *found;
   ssize_t size;

   found = strstr(start, key);
   if (found)
     {
        found += strlen(key);
        start = found;

        while (found[0] != ';' && found[0] != '\0')
          found++;

        size = found - start;
        value = malloc(size + 1);
        memcpy(value, start, size);
        value[size] = '\0';

        return value;
     }

   return NULL;
}

cookie_t *
web_cz_cookie(const char *name)
{
   char *cookies, *found, *start;
   cookie_t *cookie;
   char key[4096];
   ssize_t size;

   if (!getenv("HTTP_COOKIE"))
     return NULL;

   cookies = strdup(getenv("HTTP_COOKIE"));
   if (!cookies) 
     return NULL;

   cookie = calloc(1, sizeof(cookie_t));
   snprintf(key, sizeof(key), "%s=", name);

   found = strstr(cookies, key);
   if (!found)
     return NULL;

   start = found + strlen(key);

   while (found[0] != ';' && found[0] != '\0')
     found++;

   cookie->name = strdup(name);

   size = found - start;
   cookie->value = malloc(size + 1);
   memcpy(cookie->value, start, size);
   cookie->value[size] = '\0';

   free(cookies);

   return cookie;
}

cookie_t *web_cz_cookie_add(cookie_t *cookie)
{
   Web_Cz *self;
   cookie_t *c;

   self = web_cz_global_object_get();
   if (!self)
     return NULL;

   if (!cookie->name || !cookie->value)
     return NULL;

   c = self->cookies;
   if (!c)
     {
        self->cookies = c = calloc(1, sizeof(cookie_t));
        memcpy(c, cookie, sizeof(cookie_t));
        c->next = NULL;

        return self->cookies;
     }

   for (c = self->cookies; c; c = c->next)
      {
         if (!strcmp(c->name, cookie->name))
           {
              cookie_t *next = c->next;
              memcpy(c, cookie, sizeof(cookie_t));
              if (next)
                c->next = next;

              return self->cookies;
           }
      }

   for (c = self->cookies; c->next; c = c->next);

   if (!c->next)
     {
        c->next = calloc(1, sizeof(cookie_t));
        c = c->next;

        memcpy(c, cookie, sizeof(cookie_t));

        c->next = NULL;
        return self->cookies;
     }

   return NULL;
}

cookie_t *web_cz_cookie_new(const char *name, const char *value)
{
   cookie_t *c = calloc(1, sizeof(cookie_t));
   c->name = strdup(name);
   c->value = strdup(value);
   c->self = c;

   return c;
}

void web_cz_cookie_remove(const char *name)
{
   cookie_t *c = web_cz_cookie_new(name, "");
   c->delete = true;
   web_cz_cookie_add(c);
}

static char *
_time_to_str(unsigned int secs)
{
   char buf[256];
   struct timeval tv;
   int seconds = secs;

   if (secs)
     seconds += time(NULL);

   tv.tv_sec = seconds;
   tv.tv_usec = 0;

   strcpy(buf, ctime((time_t *) &tv));
   buf[strlen(buf) -1 ] = '\0';

   return strdup(buf);
}

void web_cz_content_type(const char *type)
{
   Web_Cz *self;
   cookie_t *c;

   self = web_cz_global_object_get();
   if (!self)
     return;

   c = self->cookies;
   while (c)
     {
        printf("Set-Cookie: %s=%s;", c->name, c->value);

        if (c->path)
          printf(" path=%s;", c->path);

        if (c->expires != 0)
          {
             char *t = _time_to_str(c->expires);
             printf(" expires=%s;", t);
             free(t);
          }

        if (c->domain)
          printf(" domain=%s;", c->domain);

        if (c->delete)
          {
             char *t = _time_to_str(0);
             printf(" expires=%s;", t);
             free(t);
          }

        printf("\r\n");
        c = c->next;
     }

   printf("Content-type: %s\r\n\r\n", type);
}

#define SESSION_FILE_FORMAT "%s\t%lu\n"

void
web_cz_session_new(const char *name, unsigned long duration)
{
   Web_Cz *self;
   cookie_t *session_cookie;
   unsigned long expiration, time_now;
   struct stat st;
   Strbuf *path;
   FILE *f;
   unsigned char key[MD5_DIGEST_LENGTH];
   char key_plaintext[MD5_DIGEST_LENGTH * 2 + 1];
   MD5_CTX ctx;
   int i, j;
   const char *secret = "HASH_SECRET";

   self = web_cz_global_object_get();
   if (!self)
     return;

   /* Exists and is valid */
   if (self->session.check(name))
     return;

   MD5_Init(&ctx);

   time_now = time(NULL);

   MD5_Update(&ctx, secret, strlen(secret));
   MD5_Update(&ctx, name, strlen(name));
   MD5_Update(&ctx, &time_now, sizeof(unsigned long));
   MD5_Final(key, &ctx);

   j = 0;
   for (i = 0; i < MD5_DIGEST_LENGTH; i++)
     {
       snprintf(&key_plaintext[j], sizeof(key_plaintext), "%02x", (unsigned int) key[i]);
       j += 2;
     }

   key_plaintext[j] = 0;

   session_cookie = self->cookie.new(name, key_plaintext);
   session_cookie->expires = duration;

   expiration = duration + time_now;

   self->cookie.add(session_cookie);

   // store a copy on disk

   path = strbuf_new();

   strbuf_append_printf(path, "sessions/%s", name);

   if (stat(strbuf_string_get(path), &st) != -1)
     {
        unlink(strbuf_string_get(path));
     }

   f = fopen(strbuf_string_get(path), "w");

   fprintf(f, SESSION_FILE_FORMAT, key_plaintext, expiration);

   fclose(f);

   strbuf_free(path);
}

void
web_cz_session_destroy(const char *name)
{
   Web_Cz *self;
   cookie_t *session_cookie;
   struct stat st;
   Strbuf *path;

   self = web_cz_global_object_get();
   if (!self)
     return;

   session_cookie = self->cookie.get(name);
   if (!session_cookie) { }

   self->cookie.remove(name);

   // remove saved copy
   path = strbuf_new();

   strbuf_append_printf(path, "sessions/%s", name);

   if (stat(strbuf_string_get(path), &st) != -1)
     {
        unlink(strbuf_string_get(path));
     }

   strbuf_free(path);
}

static const char *
_parse_session_cookie(char *buf, unsigned long *expires)
{
   char *local_key = buf;
   char *local_key_end = strchr(buf, '\t');
   *local_key_end = '\0';

   *expires = atol(local_key_end + 1);

   return local_key;
}

bool
web_cz_session_check(const char *name)
{
   Web_Cz *self;
   cookie_t *session_cookie;
   struct stat st;
   Strbuf *path;
   char buf[8192];
   FILE *f;
   int lines = 0;
   bool status = false;

   self = web_cz_global_object_get();
   if (!self)
     return false;

   session_cookie = self->cookie.get(name);
   if (!session_cookie)
     return false;

   path = strbuf_new();
   strbuf_append_printf(path, "sessions/%s", name);

   if (stat(strbuf_string_get(path), &st) == -1)
     return false;

   f = fopen(strbuf_string_get(path), "r");

   while ((fgets(buf, sizeof(buf), f)) != NULL)
     {
        lines++;
     }

   fclose(f);
   buf[strlen(buf) -1 ] = '\0';

   if (lines != 1)
     {
        // broken session file
        goto out;
     }

   unsigned long expires = 0;
   const char *local_key = _parse_session_cookie(buf, &expires);
   if ((!strcmp(local_key, session_cookie->value) &&
       (expires > (unsigned long)time(NULL))))
     {
        status = true;
     }
out:
   strbuf_free(path);

   return status;
}

void
web_cz_free(void)
{
   cookie_t *old, *c;
   Web_Cz *self;

   self = web_cz_global_object_get();
   if (!self)
     return;

   c = self->cookies;
   while (c)
     {
        if (c->name)
          free(c->name);
        if (c->value)
          free(c->value);

        old = c;
        c = c->next;
        free(old); 
     }

   param_t *last, *p = self->parameters;
   while (p)
     {
        if (p->name)
          free(p->name);
        if (p->value)
          free(p->value);
 
        last = p;
        p = p->next;
        free(last);
     }

   free(self);
   _web_cz_global_object = NULL;
}

void
web_cz_init(Web_Cz *w)
{
   Web_Cz *self = w;

   self->cgi.get = web_cz_get;
   self->cgi.param = web_cz_param;

   self->cookie.new = web_cz_cookie_new;
   self->cookie.add = web_cz_cookie_add;
   self->cookie.remove = web_cz_cookie_remove;
   self->cookie.get = web_cz_cookie;

   self->session.new = web_cz_session_new;
   self->session.destroy = web_cz_session_destroy;
   self->session.check = web_cz_session_check;

   self->headers_display = web_cz_content_type;

   self->free = web_cz_free;

   _web_cz_global_object = self;
}

Web_Cz *
web_cz_new(void)
{
   Web_Cz *self = calloc(1, sizeof(Web_Cz));
   if (!self)
     return NULL;

   web_cz_init(self);

   return self;
}

