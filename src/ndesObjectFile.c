/**
 * @file ndesObjectFile.c
 * @brief Implantation de la gestion des files d'objets
 *
 */
#include <stdlib.h>    // Malloc, NULL, ...
#include <assert.h>

#include <ndesObjectFile.h>
#include <motsim.h>

/*
 * @brief Les maillons de la files sont représentés par cette structure
 */
struct ndesObjectFileElt_t {
   struct ndesObject_t        * object;   //< Pointeur sur l'élément

   struct ndesObjectFileElt_t * prev;
   struct ndesObjectFileElt_t * next;
};

struct ndesObjectFileElt_t * ndesObjectFileElt_create(struct ndesObject_t * object)
{
   struct ndesObjectFileElt_t * result ;

   result = (struct ndesObjectFileElt_t *)sim_malloc(sizeof(struct ndesObjectFileElt_t));

   result->object = object;

   return result;
};

void ndesObjectFileElt_free(struct ndesObjectFileElt_t * elt)
{
   free(elt);
};

/*
 * @brief Structure définissant une file d'objets
 */
struct ndesObjectFile_t {
   struct ndesObjectType_t * type; //< Type des éléments
   int           nombre;   //< Nombre d'éléments dans la file

   /* Gestion de la file */
   struct ndesObjectFileElt_t * premier;
   struct ndesObjectFileElt_t * dernier;
};


/** @brief Création d'une file.
 *
 *  @param type permet de définir le type d'objets de la liste
 *  @return Une strut ndesObjectFile_t * allouée et initialisée
 *
 */
struct ndesObjectFile_t * ndesObjectFile_create(struct ndesObjectType_t * type)
{
   printf_debug(DEBUG_FILE, "in\n");

   struct ndesObjectFile_t * result = (struct ndesObjectFile_t *) sim_malloc(sizeof(struct ndesObjectFile_t));
   assert(result);

   result->nombre = 0;
   result->type = type;

   result->premier = NULL;
   result->dernier = NULL;

   printf_debug(DEBUG_FILE, "out\n");

   return result;
}

/*
 * Extraction du premier Ã©lÃ©ment de la file.
 *
 * Retour
 *   pointeur sur l'object qui vient d'Ãªtre extrait
 *   NULL si la file Ã©tait vide
 */
struct ndesObject_t * ndesObjectFile_extract(struct ndesObjectFile_t * file)
{
   struct ndesObject_t * object = NULL;
   struct ndesObjectFileElt_t * premier;

   printf_debug(DEBUG_FILE, " file %p extracting object (out of %d)\n", file, file->nombre);
   //  ndesObjectFile_dump(file);

   if (file->premier) {
      object = file->premier->object;
      premier = file->premier;
      file->premier = premier->next;
      // Si c'Ã©tait le seul
      if (file->dernier == premier) {
         assert(premier->next == NULL);
	 assert(file->nombre == 1);
         file->dernier = NULL;
      } else {
	 assert(file->premier != NULL);
         file->premier->prev = NULL;
      }
      file->nombre --;

      ndesObjectFileElt_free(premier);
   }
   printf_debug(DEBUG_FILE, "out (pdu id %d)\n", object?ndesObject_getId(object):-1);
   //   ndesObjectFile_dump(file);

   return object;
}

struct ndesObject_t * ndesObjectFile_getNextObject(void * file)
{
   return ndesObjectFile_extract((struct ndesObjectFile_t *) file);
}

/**
 * @brief Insertion d'un objet dans la file
 * @param file la file dans laquelle on insère l'objet
 * @param object à insérer à la fin de la file
 *
 */
void ndesObjectFile_insertObject(struct ndesObjectFile_t * file,
                                 struct ndesObject_t * object)
{
   struct ndesObjectFileElt_t * pq;

   printf_debug(DEBUG_FILE, " file %p insert object %d (Length = %d)\n",
                file, ndesObject_getId(object),
                file->nombre);

   assert(object->type == file->type);

   //   ndesObjectFile_dump(file);

   pq = ndesObjectFileElt_create(object);

   pq->next = NULL;
   pq->prev = file->dernier;

   if (file->dernier)
      file->dernier->next = pq;

   file->dernier = pq;

   if (!file->premier)
      file->premier = pq;

   file->nombre++;

   printf_debug(DEBUG_FILE, " END inserting object (Length = %d)\n",
		file->nombre);
}

void ndesObjectFile_insert(struct ndesObjectFile_t * file,
			   void * object)
{
   assert(file->type->getObject(object)->type == file->type);

   ndesObjectFile_insertObject(file, file->type->getObject(object));
}

int ndesObjectFile_length(struct ndesObjectFile_t * file)
{
   return file->nombre;
}

/*
 * Un affichage un peu moche de la file. Peut Ãªtre utile dans des
 * phases de dÃ©bogage.
 */
void ndesObjectFile_dump(struct ndesObjectFile_t * file)
{
  struct ndesObjectFileElt_t * pq;

  printf("DUMP %2d elts (id:size/data) : ", file->nombre);
  for (pq=file->premier; pq != NULL; pq=pq->next){
    printf("[%d] ", ndesObject_getId(pq->object));
  }
  printf("\n");
}

/*
 * @brief Outil d'itération sur une liste
 */
struct ndesObjectFileIterator_t {
   struct ndesObjectFile_t    * ndesObjectFile;
   struct ndesObjectFileElt_t * position;
};

/*
 * @brief Initialisation d'un itérateur
 */
struct ndesObjectFileIterator_t * ndesObjectFile_createIterator(struct ndesObjectFile_t * of)
{
   struct ndesObjectFileIterator_t * ofi;

   ofi = (struct ndesObjectFileIterator_t *)sim_malloc(sizeof(struct ndesObjectFileIterator_t));
   ofi->ndesObjectFile = of;
   ofi->position = of->premier;
}

/*
 * @brief Obtention du prochain élément
 */
struct ndesObjectFile_t * ndesObjectFile_iteratorGetNext(struct ndesObjectFileIterator_t * ofi)
{
   struct ndesObjectFile_t * result ;

   if (ofi->position) {
      result = ofi->position->object;
      ofi->position = ofi->position->next;
   } else {
      result = NULL;
   }
   return result;
}

/*
 * @brief Terminaison de l'itérateur
 */
void ndesObjectFile_deleteIterator(struct ndesObjectFileIterator_t * ofi)
{
   free(ofi);
}
