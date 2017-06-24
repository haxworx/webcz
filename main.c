#include "include/strbuf.h"
#include "include/webcz.h"

int main(void)
{
   const char *offering;
   Web_Cz *web_obj;
   int count = 0;

   web_obj = web_cz_new();

   cookie_t *c = web_obj->cookie_new("AL", "Really nice!");
   c->path = "/";
   c->expires = 3600;
   web_obj->cookie_add(c);
   
   cookie_t *c2 = web_obj->cookie_new("Neil", "Whiskey!");
   web_obj->cookie_add(c2);

   web_obj->get();

   offering = web_obj->param("offering");
   const char *tmp = web_obj->param("count");
   if (tmp)
     count = atoi(tmp);

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

   web_obj->free();

   return 0;
}
