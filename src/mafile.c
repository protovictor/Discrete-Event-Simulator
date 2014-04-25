/*----------------------------------------------------------------------*/
/*      Un peu de quincaillerie de files                                */
/*----------------------------------------------------------------------*/
#include <stdlib.h>    // Malloc, NULL, ...
#include <assert.h>

#include <motsim.h>

struct t_file_elt {
   void * data;
   struct t_file_elt * suivant;
   struct t_file_elt * precedent;
};

struct t_file {
   int nombre;
   struct t_file_elt * premier;
   struct t_file_elt * dernier;
};

struct t_file * creerFileVide()
{
   struct t_file * result = (struct t_file *) sim_malloc(sizeof(struct t_file));

   result->nombre = 0;
   result->premier = NULL;
   result->dernier = NULL;

   return result;
}

void insererFile(struct t_file * file, void * data)
{
   struct t_file_elt * pq = (struct t_file_elt *)sim_malloc(sizeof(struct t_file_elt));

   printf_debug(DEBUG_FILE, "IN\n");

   pq->data = data;

   pq->suivant = NULL;
   pq->precedent = file->dernier;

   if (file->dernier)
      file->dernier->suivant = pq;

   file->dernier = pq;

   if (!file->premier)
      file->premier = pq;

   file->nombre++;
}

void * extraireFile(struct t_file * file)
{
   void * data = NULL;
   struct t_file_elt * premier;

   if (file->premier) {
      data = file->premier->data;
      premier = file->premier;
      file->premier = premier->suivant;
      // Si c'était le seul
      if (file->dernier == premier) {
         assert(premier->suivant == NULL);
	 assert(file->nombre == 1);
         file->dernier = NULL;
      } else {
	 assert(file->premier != NULL);
         file->premier->precedent = NULL;
      }
      file->nombre --;
   }
   return data;
}

int tailleFile(struct t_file * file)
{
   return file->nombre;
}
