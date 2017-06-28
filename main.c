#include "include/webcz.h"

int main(void)
{
   Web_Cz *web_obj;
   char *tmp;
   int count = 0;

   web_obj = web_cz_new();

   cookie_t *c = web_obj->cookie.new("AL", "Really nice!");
   c->path = "/";
   c->expires = 3600;
   web_obj->cookie.add(c);

   cookie_t *c2 = web_obj->cookie.new("Ed", "Skateboard!");
   web_obj->cookie.add(c2);

//   web_obj->session.destroy("netstar");
   web_obj->session.new("netstar", 3600);

   const char *offering = web_obj->cgi.param("offering");

   tmp = web_obj->cgi.param("count");
   if (tmp)
     count = atoi(tmp);

   web_obj->cookie.remove("Random");
   int session = web_obj->session.check("netstar");

   /* BEGIN output */
   web_obj->headers_display("text/plain");

   if (offering && count)
     printf("it is %s and %d\n", offering, count);

   cookie_t *test = web_obj->cookie.get("Ed");
   if (test)
     {
        printf("Got value %s\n", test->value);
     }
   test = web_obj->cookie.get("AL");
   if (test)
     {
        printf("Got value %s\n", test->value);
     }

   printf("session is %s\n", (session ? "OKAY" : "BAD"));

   web_obj->free();

   return 0;
}
