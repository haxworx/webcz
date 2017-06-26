#ifndef __WEB_CZ__
#define __WEB_CZ__
#define _POSIX_C_SOURCE  200809L
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <fcntl.h>

typedef struct _param_t param_t;
struct _param_t {
   char *name;
   char *value;
   param_t *next;
};

#define CHUNK 256 

typedef struct _cookie_t cookie_t;
struct _cookie_t {
   char *name;
   char *value;
   char *domain;
   char *path;
   bool delete;
   bool is_session;
   time_t expires;
   cookie_t *next;
};

typedef void (*wcz_fn_free)(void);
typedef bool (*wcz_fn_get)(void);
typedef char *(*wcz_fn_param)(const char *); 
typedef cookie_t *(*wcz_fn_cookie)(const char *);
typedef cookie_t *(*wcz_fn_cookie_add)(cookie_t *);
typedef cookie_t *(*wcz_fn_cookie_new)(const char *, const char *);
typedef void (*wcz_fn_cookie_remove)(const char *);
typedef void (*wcz_fn_content_type)(const char *);
typedef void (*wcz_fn_session_new)(const char *, unsigned long);
typedef void (*wcz_fn_session_destroy)(const char *);
typedef bool (*wcz_fn_session_check)(const char *);

typedef struct _Web_Cz {
   wcz_fn_get get;   
   wcz_fn_param param;
   wcz_fn_cookie cookie;  
   wcz_fn_cookie_add cookie_add;
   wcz_fn_content_type content_type;
   wcz_fn_free free;

   wcz_fn_cookie_new cookie_new;
   wcz_fn_cookie_remove cookie_remove;

   wcz_fn_session_new session_new;
   wcz_fn_session_destroy session_destroy;
   wcz_fn_session_check session_check;

   param_t *parameters;
   cookie_t *cookies;

   bool have_request;
} Web_Cz;

cookie_t *web_cz_cookie(const char *name);

cookie_t *web_cz_cookie_add(cookie_t *cookie);

char *web_cz_param(const char *name);

bool web_cz_get(void);

void web_cz_content_type(const char *type);

Web_Cz *web_cz_new(void);

Web_Cz *web_cz_global_object_get();

void web_cz_free(void);

#endif
