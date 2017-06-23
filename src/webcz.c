#include "../include/webcz.h"
Web_Cz *_web_cz_global_object = NULL;

static Web_Cz *
webcz_global_object_get(void)
{
   return _web_cz_global_object;
}

static void *
_param_add(const char *name, const char *value)
{
   Web_Cz *self = webcz_global_object_get();

   param_t *cursor = self->paramaters;

   if (!name || !name[0] || !value || !value[0])
     return self->paramaters;

   if (!cursor)
     {
        self->paramaters = cursor = calloc(1, sizeof(param_t));
        cursor->name = strdup(name);
        cursor->value = strdup(value);
        return self->paramaters;
     }

   for (cursor = self->paramaters; cursor->next; cursor = cursor->next);

   if (!cursor->next)
     {
        cursor->next = calloc(1, sizeof(param_t));
        cursor = cursor->next;
        cursor->name = strdup(name);
        cursor->value = strdup(value);
     }

   return self->paramaters;
}
/* ********************************************* */

char *
web_cz_param(const char *name)
{
   Web_Cz *self = _web_cz_global_object;

   param_t *cursor = self->paramaters->next;
   while (cursor)
     {
        if (!strcmp(cursor->name, name))
          return cursor->value;
        cursor = cursor->next;
     }
   return NULL;
}

bool web_cz_get(void)
{
   char *buffer = NULL;
   ssize_t buffer_size, len;
   char buf[CHUNK];
   Web_Cz *self = _web_cz_global_object;
   const char *method =  getenv("REQUEST_METHOD");
   if (!method) return false;

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

   char *p, *start, *end;

   self->paramaters = calloc(1, sizeof(param_t));
   p = buffer;
   while (1)
     {
        start = p;
        p = strchr(start, '=');
        if (p)
          *p = '\0';
        char *name = start;
        p++;
        end = strchr(p, '&');
        if (end)
          *end = '\0';
        char *value = p;
        self->paramaters = _param_add(name, value);
        if (!end) break;
        p = end + 1;
     }

   return true;
}

char *
_cookie_optional(char *start, const char *key)
{
   char value[4096];
   char *found;

   found = strstr(start, key);
   if (found)
     {
        found += strlen(key);
        start = found;
        while (found[0] != ';' && found[0] != '\0')
          found++;

        ssize_t size = found - start;
        memcpy(value, start, size);
        value[size] = '\0';
        return strdup(value); 
     }

   return NULL;
}

cookie_t *
web_cz_cookie(const char *name)
{
   char *cookies;
   char *found;
   char key[4096];
   char value[4096];
   cookie_t *cookie = NULL; 
   if (!getenv("HTTP_COOKIE")) return NULL;

   cookies = strdup(getenv("HTTP_COOKIE"));
   if (!cookies) 
     return NULL;
  
   cookie = calloc(1, sizeof(cookie_t));

   snprintf(key, sizeof(key), "%s=", name);

   found = strstr(cookies, key);
   if (!found)
     return NULL;

   cookies = found;

   char *start = found + strlen(key);

   while (found[0] != ';' && found[0] != '\0')
     found++;

   ssize_t size = found - start;
   memcpy(value, start, size);
   value[size] = '\0';

   cookie->name = strdup(key);
   cookie->value = strdup(value);

   cookie->expiry = _cookie_optional(cookies, "expiry=");
   cookie->domain = _cookie_optional(cookies, "domain=");
   cookie->path = _cookie_optional(cookies, "path=");

   free(cookies);

   return cookie;
}

cookie_t *web_cz_cookie_add(cookie_t *cookie)
{
   Web_Cz *self = _web_cz_global_object;

   cookie_t *cursor = self->cookies;
   if (!cursor)
     {
        if (!cookie->name || !cookie->name[0] ||
            !cookie->value || !cookie->value[0]) return NULL;

        self->cookies = cursor = calloc(1, sizeof(cookie_t));

        cursor->name = strdup(cookie->name);
        cursor->value = strdup(cookie->value);
        if (cookie->expiry)
          cursor->expiry = strdup(cookie->expiry);
        if (cookie->domain)
          cursor->domain = strdup(cookie->domain);
        if (cookie->path)
          cursor->path = strdup(cookie->path);

        cursor->next = NULL;
        return self->cookies;
     }

   for (cursor = self->cookies; cursor->next; cursor = cursor->next);

   if (!cursor->next)
     {
        cursor->next = calloc(1, sizeof(cookie_t));
        cursor = cursor->next;
        cursor->name = strdup(cookie->name);
        cursor->value = strdup(cookie->value);
        if (cookie->expiry)
          cursor->expiry = strdup(cookie->expiry);
        if (cookie->domain)
          cursor->domain = strdup(cookie->domain);
        if (cookie->path)
          cursor->path = strdup(cookie->path);

        cursor->next = NULL;
        return self->cookies;
     }

   return NULL;
}

void web_cz_content_type(const char *type)
{
   Web_Cz *self = _web_cz_global_object;
   cookie_t *c = self->cookies;
   while (c)
     {
        printf("Set-Cookie: ");
        printf("%s=%s;", c->name, c->value);
        if (c->path)
          printf("path=%s;", c->path);
        if (c->domain)
          printf("domain=%s;", c->domain);

        printf("\r\n");
        c = c->next;
     }
   printf("Content-Type: %s\r\n\r\n", type);

}
void
web_cz_free(void)
{
   Web_Cz *self = _web_cz_global_object;
   if (!self)
     return;

   cookie_t *old, *c = self->cookies;
   while (c)
     {
        if (c->name)
          free(c->name);
        if (c->path)
          free(c->path);
        if (c->domain)
          free(c->domain);
        if (c->expiry)
          free(c->expiry);

        old = c;
        c = c->next;
        free(old); 
     }

   param_t *last, *p = self->paramaters;
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
WebCz_New(void)
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

   _web_cz_global_object = self;
 
   self->paramaters = calloc(1, sizeof(param_t *));
   return self;
}

