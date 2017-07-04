#include "include/webcz.h"
#include "include/strbuf.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>

static void
_trim(char *word)
{
   char *p = word;

   while (*p++)
     {
        if (p[0] == '\r' || p[0] == '\n')
          {
             p[0] = '\0';
             return;
          }
     }
}

static int
_word_check(char ** words, const char *word)
{
   const char *p = word;
   int i, j;

   if (words)
     {
        for (i = 0; words[i]; i++)
          {
             if (!strcasecmp(word, words[i]))
               return 0;
             /* We can reuse wirds after ichunks of 8 */
             if (i == 8)
               for (j = 0; j < 8; j++)
                 {
                    free(words[j]);
                    words[j] = NULL;
                 }
           }
     }

   while (*p++)
     {
        if (p[0] == '.' || p[0] == ',' || p[0] == '\"' || p[0] == '\'' || p[0] == ';')
          return 0;
        if (*p == ':')
          return 0;
     }

   return 1;
}

int *
_generate_sequence(const char *offering,  int count, int total)
{
   FILE *f;
   char buf[77];
   unsigned int seed;

   int *s = malloc(count * sizeof(int));

   f = fopen("/dev/urandom", "r");
   if (!f) return NULL;

   for (int i = 0; i < count; i++)
     {
        seed = 0;
        fread(buf, 1, sizeof(buf), f);

        for (int j = 0; j < sizeof(buf); j++)
          seed += (unsigned int) buf[j];

        for (int i = 0; i < strlen(offering); i++)
          seed += (offering[i] & 0x07);

        srandom(seed);

        s[i] = (unsigned int) (total * (random() / (RAND_MAX + 1.0)));
     }

   return s;
}

char *
_oracle(const char *path, const char *offering, int count)
{
   char *map, *pos;
   char *html, *word, **words;
   unsigned long total_bytes;
   struct stat st;
   int *sequence, fd, i = 0;
   Strbuf *buf;

   buf = strbuf_new();

   if (stat(path, &st) == -1)
     return NULL;

   total_bytes = st.st_size;

   sequence = _generate_sequence(offering, count, total_bytes);

   fd = open(path, 0444);
   
   map = (char *) mmap(NULL, total_bytes, PROT_READ, MAP_PRIVATE, fd, 0);

   close(fd);

   words = calloc(1, 1 + count * sizeof(char *));

   strbuf_append(buf, "<h1>Oracle says:</h1>");

   while (i < count)
     {
        char *start, *end;
        pos = start = map + sequence[i];
        while (*start && *start++ != ' ');
        if (!start) { sequence[i] /= 2; continue; }
        end = strchr(start, ' ');
        if (!end) { sequence[i] /= 2; continue; }
        size_t len = end - start;      
        word = malloc(len);
        memcpy(word, start, len);
        word[len] = '\0';
       _trim(word);
       if (_word_check(words, word))
         {
            words[i] = strdup(word);
            if (i != (count - 1))
              strbuf_append_printf(buf, "%s ", word);
            else
              strbuf_append_printf(buf, "%s.", word);
            i++;
         }
       else
         {
            sequence[i] /= 3;
            continue;
         }
       free(word); 
     }

   for (i = 0; i < count; i++)
     free(words[i]);
         
   free(sequence);

   munmap(map, total_bytes);
   strbuf_append(buf, "\n<p><a href='javascript:location.reload();'class='btn btn-sm btn-info'><span class='fa fa-refresh'></span>Generate again using previous data?</a></p>");
   html = strdup(strbuf_string_get(buf));

   strbuf_free(buf);

   return html;
}
 
int main(void)
{
   Web_Cz *w_obj;
   const char *length, *offering;
   char *result;
   const char *html =
   "<html>\n" \
   "<head>\n" \
   "<link href='https://fonts.googleapis.com/css?family=Rokkitt' rel='stylesheet' type='text/css'></>\n" \
   "<style> body { font-family: Rokkitt, Sans; }</style>\n" \
   "</head>\n" \
   "<body>\n" \
   "%s\n" \
   "<p><a href='../oracle'>Go back to form entry.</a></p>\n" \
   "</body>\n" \
   "</html>\n";
 
   w_obj = web_cz_new();

   offering = w_obj->cgi.param("offering");
   length = w_obj->cgi.param("length");

   if (offering && length)
     {
        int count = atoi(length);
        if (strlen(offering) < 10)
          result = "<h1>Please!</h1><p>This offering is too small (greater than 10 characters please!)</p>";
        else if (count <= 0 || count > 22)
          result = "<h1>Please!</h1>";
        else
          result = _oracle("bible.txt", offering, count);
     } 
   else
     result = "<h1>User Error</h1>\n<p>Missing fields!</p>";

   w_obj->headers_display("text/html");

   printf(html, result);

   w_obj->free();

   return 0;
}
