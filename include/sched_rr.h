/**
 * @file sched_rr.h
 * @brief Définition d'un ordonnanceur Round Robin
 */
#ifndef __SCHED_ROUND_ROBIN
#define __SCHED_ROUND_ROBIN

#include <pdu.h>

/**
 * Nombre maximal d'entrées
 */
#define SCHED_RR_NB_INPUT_MAX 8

/**
 * Structure définissant notre ordonanceur
 */
struct rrSched_t ;

/**
 * Création d'une instance de l'ordonnanceur avec la destination en
 * paramètre 
 */
struct rrSched_t * rrSched_create(void * destination,
				  processPDU_t destProcessPDU);

/**
 * Ajout d'une source (ce sera par exemple une file)
 */
void rrSched_addSource(struct rrSched_t * sched,
		       void * source,
		       getPDU_t getPDU);
/**
 * La fonction permettant de demander une PDU à notre scheduler
 * C'est ici qu'est implanté l'algorithme
 */
struct PDU_t * rrSched_getPDU(void * s);

/**
 * La fonction de soumission d'un paquet à notre ordonnanceur
 */
int rrSched_processPDU(void *s,
		       getPDU_t getPDU,
		       void * source);
#endif
