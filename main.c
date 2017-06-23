#include "include/strbuf.h"
#include "include/webcz.h"

int main(void)
{
   Web_Cz *w = WebCz_New();
   cookie_t *c = calloc(1, sizeof(cookie_t));
   c->name = "AL";
   c->value = "Hello";
   w->cookie_add(c);
   
   cookie_t *c2 = calloc(1, sizeof(cookie_t));
   c2->name = "Neil";
   c2->value = "Whiskey!";
   w->cookie_add(c2);
   if (!w->get())
     exit(0);
   const char *offering = w->param("offering");
   int count = atoi(w->param("count"));
   w->content_type("text/plain");
  
   if (offering && count)
     printf("it is %s and %d\n", offering, count);
   
   cookie_t *test = w->cookie("AL");
   if (test) 
     {
        printf("Got value %s\n", test->value);  
     } 
   test = w->cookie("Neil");
   if (test)
     {
        printf("Got value %s\n", test->value);
     }

   w->free();
   exit(0);
}
