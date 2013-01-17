#include <stdlib.h>    // Malloc, NULL, ...
#include <assert.h>

#include <file_pdu.h>
#include <motsim.h>
/*
struct filePDU_t_elt {
   struct PDU_t         * PDU;
   motSimDate_t           date_in;
   struct filePDU_t_elt * suivant;
   struct filePDU_t_elt * precedent;
};
*/
struct filePDU_t {
   int           nombre;
   unsigned long size;       // Le volume réel
   int           nbOverflow; // Nombre de pertes par dépassement

   enum filePDU_dropStrategy dropStrategy;

   int           maxLength;  // Nombre maximal d'éléments
   int           maxSize ;   // Le volume maximal

   /* Gestion de la file */
   struct PDU_t * premier;
   struct PDU_t * dernier;

   /* Mesure des débits d'entrée et sortie */
   struct probe_t * throuhputIn;
   struct probe_t * throuhputOut;

   /* Gestion de la sortie */
   void * destination; // L'objet auquel sont destinées les PDUs
   processPDU_t destProcessPDU; // La fonction permettant d'envoyer la PDU

   /* Les sondes */
   struct probe_t * insertProbe;
   struct probe_t * extractProbe;
   struct probe_t * dropProbe;
   struct probe_t * sejournProbe;
};

/*
 * Un affichage un peu moche de la file. Peut être utile dans des
 * phases de débogage.
 */
void filePDU_dump(struct filePDU_t * file)
{
  struct PDU_t * pq;

  printf("DUMP %2d elts (id:size/data) : ", file->nombre);
  for (pq=file->premier; pq != NULL; pq=pq->next){
    printf("[%d:%d/%d] ", PDU_id(pq->data), PDU_size(pq->data), (int)((struct PDU_t *)pq->data)->data);
  }
  printf("\n");
}

/*
 * Extraction du premier élément de la file.
 *
 * Retour
 *   pointeur sur la PDU qui vient d'être extraite
 *   NULL si la file était vide
 */
struct PDU_t * filePDU_extract(struct filePDU_t * file)
{
   void * PDU = NULL;
   struct PDU_t * premier;

   printf_debug(DEBUG_FILE, " extracting PDU (out of %d) at %6.3f\n", file->nombre, motSim_getCurrentTime());
   //  filePDU_dump(file);

   if (file->premier) {
      PDU = file->premier->data;
      premier = file->premier;
      file->premier = premier->next;
      // Si c'était le seul
      if (file->dernier == premier) {
         assert(premier->next == NULL);
	 assert(file->nombre == 1);
         file->dernier = NULL;
      } else {
	 assert(file->premier != NULL);
         file->premier->prev = NULL;
      }
      file->nombre --;
      file->size -= PDU_size(premier->data);

      /* Gestion des sondes */
      if (file->extractProbe) {
         probe_sample(file->extractProbe, PDU_size(premier->data));
      }
      if (file->sejournProbe) {
         if(motSim_getCurrentTime() < premier->creationDate){
	    printf_debug(DEBUG_WARN, "Attention, quand on purge, il ne faut pas mettre dans les sondes\n");
         } else {
            probe_sample(file->sejournProbe, motSim_getCurrentTime() - premier->creationDate);
	 }
      }
      PDU_free(premier);
   }
   printf_debug(DEBUG_FILE, "Apres :\n");
   //   filePDU_dump(file);
   return PDU;
}

/*
 * Définition d'une capacité maximale en octets. Une valeur nulle
 * signifie pas de limite.
 */
void filePDU_setMaxSize(struct filePDU_t * file, unsigned long maxSize)
{
   printf_debug(DEBUG_ALWAYS, "Taille max %ld bytes\n", maxSize);
   file->maxSize = maxSize; 
}

unsigned long filePDU_getMaxSize(struct filePDU_t * file)
{
  return file->maxSize;
}

void filePDU_setMaxLength(struct filePDU_t * file, unsigned long maxLength)
{
   file->maxLength = maxLength; 
}

/*
 * Choix de la stratégie de perte en cas d'insersion dans une file
 * pleine. Attention, insérer une PDU de taille t dans une file de
 * capacité max < t n'est pas une erreur, mais engendre simplement un
 * événement d'overflow.
 */
void filePDU_setDropStrategy(struct filePDU_t * file, enum filePDU_dropStrategy dropStrategy)
{
   file->dropStrategy = dropStrategy; 
}

/*
 * Réinitialisation dans un état permettant de lancer une nouvelle
 * simulation. Ici il suffit de vider la file de tous ses éléments.
 */
void filePDU_reset(struct filePDU_t * file)
{
   struct PDU_t * PDU;

   PDU = filePDU_extract(file);

   while (PDU) {
      PDU_free(PDU);
      PDU = filePDU_extract(file);
   }

   file->nbOverflow = 0;

   assert(file->premier == NULL);
   assert(file->dernier == NULL);
   assert(file->nombre == 0);
   assert(file->size == 0);
}

struct filePDU_t * filePDU_create(void * destination,
                                  processPDU_t destProcessPDU)
{
   printf_debug(DEBUG_FILE, "in\n");

   struct filePDU_t * result = (struct filePDU_t *) sim_malloc(sizeof(struct filePDU_t));
   assert(result);

   result->nombre = 0;
   result->size = 0;
   result->maxSize = 0;
   result->maxLength = 0;
   result->nbOverflow = 0;
   result->dropStrategy = filePDU_dropTail;

   //   result->throughputIn = ;
   //   result->throughputOut = ;

   result->premier = NULL;
   result->dernier = NULL;

   result->destProcessPDU = destProcessPDU;
   result->destination = destination;

   // A priori, pas de sonde
   result->insertProbe = NULL;
   result->extractProbe = NULL;
   result->sejournProbe = NULL;

   // Ajout à la liste des choses à réinitialiser avant une prochaine simu
   motsim_addToResetList(result, (void (*)(void *))filePDU_reset);

   printf_debug(DEBUG_FILE, "out\n");

   return result;
}

void filePDU_insert(struct filePDU_t * file, struct PDU_t * PDU)
{
   struct PDU_t * pq;
 
   printf_debug(DEBUG_FILE, " insert PDU %d size %d at %6.3f (Length = %d/%d, size = %lu/%d, strat %d)\n",
                PDU_id(PDU), PDU_size(PDU), motSim_getCurrentTime(),
                file->nombre, file->maxLength, file->size, file->maxSize, file->dropStrategy );

   //   filePDU_dump(file);

   // S'il s'agit d'une "drop head", on fait la place si besoin est !
   if (file->dropStrategy == filePDU_dropHead) {
      while ((((file->maxLength) && (file->maxLength < file->nombre + 1))    // Trop de PDUs
             ||            
	    ((file->maxSize) && (file->maxSize < file->size + PDU_size(PDU))) // Trop de volume
	    ) && (file->nombre)) {
 	 printf_debug(DEBUG_FILE, "need some room, head droping ...\n");

         /* Gestion des sondes */
         if (file->dropProbe) {
            probe_sample(file->dropProbe, PDU_size(PDU));
         }
         PDU_free(filePDU_extract(file));

         file->nbOverflow++;
      }
   }

   // S'il s'agit d'une drop tail
   if (((file->maxSize == 0)||(file->size + PDU_size(PDU) <= file->maxSize))
       && ((file->maxLength == 0)||(file->nombre + 1 <= file->maxLength))) {
      pq = PDU_create(0, PDU);

      pq->next = NULL;
      pq->prev = file->dernier;

      if (file->dernier)
         file->dernier->next = pq;

      file->dernier = pq;

      if (!file->premier)
         file->premier = pq;

      file->nombre++;
      file->size += PDU_size(PDU);

      /* Gestion des sondes */
      if (file->insertProbe) {
         probe_sample(file->insertProbe, PDU_size(PDU));
      }

      /* WARNING : on le fait toujours ou pour le premier ? */
      if (file->destProcessPDU && file->destination) {
         file->destProcessPDU(file->destination, (getPDU_t)filePDU_extract, file);
      }
      //printf_debug(DEBUG_FILE, "Apres :\n");
      //   filePDU_dump(file);
   } else {
      printf_debug(DEBUG_FILE, "need some room, tail droping ...\n");

      /* Gestion des sondes */
      if (file->dropProbe) {
         probe_sample(file->dropProbe, PDU_size(PDU));
      }

      PDU_free(PDU); 
      file->nbOverflow++;
   }
}

/*
 * Une fonction permettant la conformité au modèle d'échange
 */
void filePDU_processPDU(struct filePDU_t * file,
                        getPDU_t getPDU,
                        void * source)
{
   struct PDU_t * pdu;
 
   assert(getPDU != NULL);
   assert(source != NULL);

   pdu = getPDU(source);

   printf_debug(DEBUG_FILE, "got PDU %d (size %d)\n",
                PDU_id(pdu), PDU_size(pdu));

   if (pdu)
      filePDU_insert(file, pdu);
}

int filePDU_length(struct filePDU_t * file)
{
   return file->nombre;
}

/*
 * Taille des n premiers paquets de la file (n>=1)
 */
int filePDU_size_n_PDU(struct filePDU_t * file, int n)
{
   int i = 1;
   int result;
   struct PDU_t * pq = file->premier;

   assert(pq);
   result = PDU_size((struct PDU_t *)pq->data);

   while (i < n) {
      pq = pq->next;
      assert(pq);
      result += PDU_size((struct PDU_t *)pq->data);
      i++;
   }

   printf_debug(DEBUG_FILE, "PDU - to %d : size %d\n", n, result);

   return result;
}

int filePDU_size(struct filePDU_t * file)
{
   return filePDU_size_n_PDU(file, filePDU_length(file));
}

/* WARNING : il vaudrait mieux une fonction qui donne un
 * pointeur vers la neme PDU, non ?
 */

/*
 * Taille du enieme paquet de la file (n>=1)
 */
int filePDU_size_PDU_n(struct filePDU_t * file, int n)
{
   int i = 1;
   struct PDU_t * pq = file->premier;

   assert(pq);

   while (i < n) {
      pq = pq->next;
      assert(pq);
      i++;
   }

   assert(pq->data);

   //   printf_debug(DEBUG_FILE, "PDU %d : size %d\n", n, PDU_size(pq->PDU));

   return PDU_size((struct PDU_t *)pq->data);
}

/*
 * Id du enieme paquet de la file (n>=1)
 */
int filePDU_id_PDU_n(struct filePDU_t * file, int n)
{
   int i = 1;
   struct PDU_t * pq = file->premier;

   assert(pq);

   while (i < n) {
      pq = pq->next;
      assert(pq);
      i++;
   }

   assert(pq->data);

   //   printf_debug(DEBUG_FILE, "PDU %d : id %d\n", n, PDU_id(pq->PDU));

   return PDU_id((struct PDU_t *)pq->data);
}

/*
 * Affectation d'une sonde sur les evenements d'insertion
 */
void filePDU_addInsertSizeProbe(struct filePDU_t * file, struct probe_t * insertProbe)
{
   probe_chain(insertProbe, file->insertProbe);
   file->insertProbe = insertProbe;
}

/*
 * Affectation d'une sonde sur les evenements d'extraction
 */
void filePDU_addExtractSizeProbe(struct filePDU_t * file, struct probe_t * extractProbe)
{
   probe_chain(extractProbe, file->extractProbe);
   file->extractProbe = extractProbe;
}


/*
 * Ajoût d'une sonde sur la taille des PDU jetées
 */
void filePDU_addDropSizeProbe(struct filePDU_t * file, struct probe_t * dropProbe)
{
   probe_chain(dropProbe, file->dropProbe);
   file->dropProbe = dropProbe;
}

/*
 * Affectation d'une sonde sur le temps de séjour
 */
void filePDU_addSejournProbe(struct filePDU_t * file, struct probe_t * sejournProbe)
{
   probe_chain(sejournProbe, file->sejournProbe);
   file->sejournProbe = sejournProbe;
}

/*
 * Mesure du débit d'entrée sur les n-1 dernières PDUs, où n est le
 * nombre de PDUs présentes. Le débit est alors obtenu en divisant la
 * somme des tailles des n-1 dernières PDUs par la durée entre les
 * dates d'arrivée de la première et la dernière.
 * S'il n'y a pas assez de PDUs, le résultat est nul
 */
double filePDU_getInputThroughput(struct filePDU_t * file)
{
   double result = 0.0;

   if (file->nombre > 1) {
      // Volume reçu depuis la première PDU
      result = filePDU_size_n_PDU(file, filePDU_length(file)) - PDU_size(file->premier->data);
      printf_debug(DEBUG_ALWAYS, "%f de %f a %f\n", result, file->premier->creationDate, file->dernier->creationDate);

      // On divise par le temps entre la première et la dernière
      result = result/(file->dernier->creationDate - file->premier->creationDate);
   }

   return result;
}
