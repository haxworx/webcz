#include "include/webcz.h"
 
int main(void)
{
   Web_Cz *w_obj;
   const char *count, *offering;
   bool session;
   
   w_obj = web_cz_new();

   cookie_t *c = w_obj->cookie.new("AL", "Really nice!");
   c->path = "/";
   c->expires = 3600;
   w_obj->cookie.add(c);

   w_obj->cookie.add(w_obj->cookie.new("Ed", "Skateboard!"));

//   w_obj->session.destroy("netstar");
   w_obj->session.new("netstar", 3600);

   offering = w_obj->cgi.param("offering");

   count = w_obj->cgi.param("count");

   w_obj->cookie.remove("Random");
   
   session = w_obj->session.check("netstar");

   /* BEGIN output */
   w_obj->headers_display("text/plain");

   if (offering && count)
     printf("it is %s and %d\n", offering, atoi(count));

   cookie_t *test = w_obj->cookie.get("Ed");
   if (test)
     {
        printf("Got value %s\n", test->value);
     }
   test = w_obj->cookie.get("AL");
   if (test)
     {
        printf("Got value %s\n", test->value);
     }

   printf("session is %s\n", (session ? "OKAY" : "BAD"));

   w_obj->free();

   return 0;
}
