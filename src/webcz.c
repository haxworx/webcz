#include "../include/webcz.h"

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

   _parse_request(buffer);

   if (buffer)
     free(buffer);

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

   if (!cookie->name || !cookie->name[0] ||
       !cookie->value || !cookie->value[0])
     return NULL;

   c = self->cookies;
   if (!c)
     {
        self->cookies = c = calloc(1, sizeof(cookie_t));
        memcpy(c, cookie, sizeof(cookie_t));
        c->next = NULL;

        return self->cookies;
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

   return c;
}

static char *
_time_to_str(unsigned int secs)
{
   char buf[256];
   struct timeval tv;
   int seconds = secs;

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
        printf("Set-Cookie: ");
        printf(" %s=%s;", c->name, c->value);
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

        printf("\r\n");
        c = c->next;
     }

   printf("Content-type: %s\r\n\r\n", type);
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

Web_Cz *
web_cz_new(void)
{
   Web_Cz *self = calloc(1, sizeof(Web_Cz));
   if (!self)
     return NULL;

   self->get = web_cz_get;
   self->param = web_cz_param;
   self->cookie = web_cz_cookie;
   self->cookie_add = web_cz_cookie_add;
   self->content_type = web_cz_content_type;
   self->free = web_cz_free;

   self->cookie_new = web_cz_cookie_new;

   _web_cz_global_object = self;

   return self;
}

