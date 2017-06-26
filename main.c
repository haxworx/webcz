#include "include/strbuf.h"
#include "include/webcz.h"

int main(void)
{
   Web_Cz *web_obj;
   char *tmp;
   int count = 0;

   web_obj = web_cz_new();

   cookie_t *c = web_obj->cookie_new("AL", "Really nice!");
   c->path = "/";
   c->expires = 3600;
   web_obj->cookie_add(c);
   
   cookie_t *c2 = web_obj->cookie_new("Neil", "Whiskey!");
   web_obj->cookie_add(c2);
   web_obj->session_destroy("random_session");
   web_obj->session_new("netstar", 3600);

   const char *offering = web_obj->param("offering");

   tmp = web_obj->param("count");
   if (tmp)
     count = atoi(tmp);

   web_obj->cookie_remove("Random");
   int session = web_obj->session_check("netstar");
  
   web_obj->content_type("text/plain");
   if (offering && count)
     printf("it is %s and %d\n", offering, count);

   cookie_t *test = web_obj->cookie("Neil");
   if (test) 
     {
        printf("Got value %s\n", test->value);  
     } 
   test = web_obj->cookie("AL");
   if (test)
     {
        printf("Got value %s\n", test->value);
     }

   printf("session is %s\n", (session ? "OKAY" : "BAD"));

   web_obj->free();

   return 0;
}
