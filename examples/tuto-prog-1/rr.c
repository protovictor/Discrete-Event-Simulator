/*----------------------------------------------------------------------*/
/* Exemple de création d'une entité NDES.                               */
/*----------------------------------------------------------------------*/

#include <stdlib.h>    // Malloc, NULL, exit, ...
#include <assert.h>
#include <strings.h>   // bzero, bcopy, ...
#include <stdio.h>     // printf, ...

#include <motsim.h>
#include <pdu.h>
#include <file_pdu.h>
#include <pdu-source.h>
#include <pdu-sink.h>
#include <ll-simplex.h>

/*
 * Structure définissant notre ordonanceur
 */
#define SCHED_RR_NB_INPUT_MAX 8

struct rrSched_t {
   // La destination (typiquement un lien)
   void         * destination;
   processPDU_t   destProcessPDU;

   // Les sources (files d'entrée)
   int        nbSources;
   void     * sources[SCHED_RR_NB_INPUT_MAX];
   getPDU_t   srcGetPDU[SCHED_RR_NB_INPUT_MAX];

   // La dernière source servie par le tourniquet
   int lastServed;
};

/*
 * Création d'une instance de l'ordonnanceur avec la destination en
 * paramètre 
 */
struct rrSched_t * rrSched_create(void * destination,
				  processPDU_t destProcessPDU)
{
   struct rrSched_t * result = (struct rrSched_t * )sim_malloc(sizeof(struct rrSched_t));

   // Gestion de la destination
   result->destination = destination;
   result->destProcessPDU = destProcessPDU;

   // Pas de source définie
   result->nbSources = 0;

   // On commence quelquepart ...
   result->lastServed = 0;

   return result;
}

/*
 * Ajout d'une source (ce sera par exemple une file)
 */
void rrSched_addSource(struct rrSched_t * sched,
		       void * source,
		       getPDU_t getPDU)
{
   assert(sched->nbSources < SCHED_RR_NB_INPUT_MAX);

   sched->sources[sched->nbSources] = source;
   sched->srcGetPDU[sched->nbSources++] = getPDU;
}

/*
 * La fonction permettant de demander une PDU à notre scheduler
 * C'est ici qu'est implanté l'algorithme
 */
struct PDU_t * rrSched_getPDU(void * s)
{
   struct rrSched_t * sched = (struct rrSched_t * )s;
   struct PDU_t * result = NULL;

   assert(sched->nbSources > 0);

   int next = sched->lastServed;

   // Quelle est la prochaine source à servir ?
   do {
      // On cherche depuis la prochaine la première source qui a des
      // choses à nous donner
      next = (next + 1)%sched->nbSources;
      result = sched->srcGetPDU[next](sched->sources[next]);
   } while ((result == NULL) && (next != sched->lastServed));

   if (result)
     sched->lastServed = next;
   return result;
}

/*
 * La fonction de soumission d'un paquet à notre ordonnanceur
 */
int rrSched_processPDU(void *s,
		       getPDU_t getPDU,
		       void * source)
{
   int result;
   struct rrSched_t * sched = (struct rrSched_t *)s;

   printf_debug(DEBUG_SCHED, "in\n");

   result = sched->destProcessPDU(sched->destination, rrSched_getPDU, sched);

   printf_debug(DEBUG_SCHED, "out %d\n", result);

   return result;
}

#define NB_SOURCES 4
int main()
{
   struct dateGenerator_t   * dateGen[NB_SOURCES];
   struct randomGenerator_t * sizeGen[NB_SOURCES];
   struct PDUSource_t       * sources[NB_SOURCES];
   struct filePDU_t         * files[NB_SOURCES];
   struct rrSched_t         * scheduler;
   struct llSimplex_t       * lien;
   struct PDUSink_t         * puits;
   struct probe_t           * filesOutProbe[NB_SOURCES];

   int n; // Indice de boucle

   /* Creation du simulateur */
   motSim_create();

   // Le puits
   puits = PDUSink_create();

   // Le lien
   lien = llSimplex_create(puits, 
			   PDUSink_processPDU,
			   40000.0,
			   0.01);
   // Le scheduler
   scheduler = rrSched_create(lien, llSimplex_processPDU);

   // Les files
   for (n = 0; n < NB_SOURCES ; n++) {
      files[n] = filePDU_create(scheduler, rrSched_processPDU);
      rrSched_addSource(scheduler, files[n], filePDU_getPDU);

      // Une sonde pour mesurer le débit de sortie
      filesOutProbe[n] = probe_createTimeSliceThroughput(1.0);

      filePDU_addExtractSizeProbe(files[n], filesOutProbe[n]);
   }

   // Les sources
   for (n = 0; n < NB_SOURCES ; n++) {

      // Les dates de creation
      dateGen[n] =  dateGenerator_createExp(10.0);

      // La source
      sources[n] = PDUSource_create(dateGen[n], files[n], filePDU_processPDU);

      // Le générateur de tailles de paquets
      sizeGen[n] = randomGenerator_createUIntConstant(1000+1000*n);

      PDUSource_setPDUSizeGenerator(sources[n], sizeGen[n]);

      // On la démarre
      PDUSource_start(sources[n]);
   }

   /* C'est parti pour 100 000 millisecondes de temps simulÃ© */
   motSim_runUntil(1000.0);

   // On va juste se contenter de compter le nombre de paquets servis
   // dans chaque file et le débit obtenu par chacune.
   for (n=0; n< NB_SOURCES; n++){
     printf("[%d] %ld %f\n", n, probe_nbSamples(filesOutProbe[n]), probe_mean(filesOutProbe[n]));
   }
   return 0;
}
