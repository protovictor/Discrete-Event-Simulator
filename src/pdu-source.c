/**
 * @file pdu-source.c
 *
 * @brief Source générique de PDU.
 *
 *   Lorsqu'une PDU est produite, elle est fourni immédiatement à la
 *   destination. Si celle-ci ne la consomme pas, elle RESTE
 *   DISPONIBLE jusqu'à la production de la suivante.
 *
 */
#include <stdlib.h>    // Malloc, NULL, exit...

#include <event.h>
#include <pdu-source.h>

struct PDUSource_t {
   struct dateGenerator_t * dateGen; //!< Le générateur de date de départ
   struct randomGenerator_t * sizeGen;//!< Le générateur de taille

   void * destination; //!< L'objet auquel sont destinées les PDUs
   processPDU_t destProcessPDU; //!< La fonction permettant de signaler la présence de la PDU

   struct probe_t *  PDUGenerationSizeProbe; //!< Une sonde sur la taille des PDU produites
   struct PDU_t * pdu;

   struct dateSize sequence[]; //!< Pour le cas déterministe
};

struct PDUSource_t * PDUSource_create(struct dateGenerator_t * dateGen,
				      void * destination,
				      processPDU_t destProcessPDU)
{
   struct PDUSource_t * result = (struct PDUSource_t *)
              sim_malloc(sizeof(struct PDUSource_t));

   result->pdu = NULL;
   result->dateGen = dateGen;
   result->destProcessPDU = destProcessPDU;
   result->destination = destination;
   result->sizeGen = NULL;
   result->PDUGenerationSizeProbe = NULL;
   result->sequence = NULL;

   // Ajout à la liste des choses à réinitialiser avant une prochaine simu
   motsim_addToResetList(result, PDUSource_start);
   
   return result;
}


/** @brief Création d'un générateur déterministe
 * 
 *  @param sequence Un tableau de {date, size} définissant chaque PDU
 *  @param destination L'entité aval
 *  @param destProcessPDU La fonction de traitement de la destination
 *  @result Un pointeur sur la source créée/initialisée
 *
 *  Un tel générateur permet de définir explicitement la séquence des
 *  PDUs à générer. Cette séquence est définie par un tableau de
 *  couples {date, taille}. Le dernier élément de la liste doit être
 *  {0.0, 0}. Le tableur n'est pas copié, il ne doit donc pas être
 *  libéré tant que la source peut servir.
 */
struct PDUSource_t * PDUSource_createDeterministic(struct dateSize * sequence,
						   void * destination,
						   processPDU_t destProcessPDU)
{
   struct PDUSource_t * result = PDUSource_create(dateGenDeterministic,
						  destination,
						  destProcessPDU);
   result->sizeGen = NULL; // Pour le moment, c'est implanté par
			   // quelques lignes spécifiques fondées sur
			   // l'absence de générateur de date, pas
			   // terrible mais vite fait !
   result->sequence = sequence;
   return result;

}

/*
 * Positionnement d'une sonde sur la taille des PDUs produites. Toutes
 * les PDUs créées sont concernées, même si elles ne sont pas
 * récupérées par la destination.
 */
void PDUSource_addPDUGenerationSizeProbe(struct PDUSource_t * src, struct probe_t *  newProbe)
{
   probe_chain(newProbe, src->PDUGenerationSizeProbe);
   src->PDUGenerationSizeProbe = newProbe;
}

/*
 * Spécification du générateur de taille de PDU associé
 */
void PDUSource_setPDUSizeGenerator(struct PDUSource_t * src, struct randomGenerator_t * rg)
{
   src->sizeGen = rg;
}

void PDUSource_buildNewPDU(struct PDUSource_t * source)
{
   double date;
   struct event_t * event;
   unsigned int size = 0; 
   int i;

   printf_debug(DEBUG_SRC, " building new PDU\n");

   // Suppression de la PDU précédente si pas consommée
   if (source->pdu) {
     //      printf("PDUSource_buildNewPDU : Destruction de %d\n", PDU_id(source->pdu));
      PDU_free(source->pdu);
   }

   // Gestion de la version "déterministe" par une valeur spéciale du
   // pointeur. Je n'aime pas ça, mais en attendant mieux, ...
   if (source->dateGen == NULL) {
      // On va chercher la prochaine date
A FAIRE
   } else {
      // On détermine la date de prochaine transmission
      date = dateGenerator_nextDate(source->dateGen, motSim_getCurrentTime());
      // On choisi la taille
      size = source->sizeGen?randomGenerator_getNextUInt(source->sizeGen):0;
   }

   // Création de la PDU
   source->pdu = PDU_create(size, NULL); 

   printf_debug(DEBUG_SRC, " PDU %d (size %u %u) created at %6.3f\n",
                PDU_id(source->pdu), size, PDU_size(source->pdu),motSim_getCurrentTime());

   // On enregistre dans la probe s'il y en a une
   if (source->PDUGenerationSizeProbe) {
      probe_sample(source->PDUGenerationSizeProbe, (double)PDU_size(source->pdu));
   }

   // On passe la PDU au suivant  
   if ((source->destProcessPDU) && (source->destination)) {
     (void)source->destProcessPDU(source->destination, PDUSource_getPDU, source);
   }

   printf_debug(DEBUG_SRC, " next PDU at %f\n", date);

   // On crée un événement pour cette date
   event = event_create((eventAction_t)PDUSource_buildNewPDU, source, date);

   // On ajoute cet événement au simulateur
   motSim_addEvent(event);
}

/*
 * The function used by the destination to actually get the next PDU
 */
struct PDU_t * PDUSource_getPDU(struct PDUSource_t * source)
{
   struct PDU_t * pdu = source->pdu;

   source->pdu = NULL;

   printf_debug(DEBUG_SRC, "releasing PDU %d (size %d)\n", PDU_id(pdu), PDU_size(pdu));

   return pdu;
}


void PDUSource_start(struct PDUSource_t * source)
{
   double date;
   struct event_t * event;

   // On ne sait jamais (cette fonction sert de reset)
   if (source->pdu) {
      PDU_free(source->pdu);
      source->pdu = NULL;
   }

   // On détermine la date de prochaine transmission
   date = dateGenerator_nextDate(source->dateGen, motSim_getCurrentTime());

A FAIRE ...

   printf_debug(DEBUG_SRC, " First PDU at %f\n", date);

   // On crée un événement pour cette date
   event = event_create((eventAction_t)PDUSource_buildNewPDU, source, date);

   // On ajoute cet événement au simulateur
   motSim_addEvent(event);
}

