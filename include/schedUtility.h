/*
 *   Ordonnancement par un algorithme simple fondé sur des fnctions d'utilité 
 *
 */
#ifndef __SCHED_UTILITY
#define __SCHED_UTILITY

#include <schedACM.h>

/*
 * Création d'un scheduler avec sa "destination". Cette dernière doit
 * être de type struct DVBS2ll_t  et avoir déjà été complêtement
 * construite (tous les MODCODS créés).
 * Le nombre de files de QoS différentes par MODCOD est également
 * passé en paramètre.
 */
struct schedACM_t * schedUtility_create(struct DVBS2ll_t * dvbs2ll, int nbQoS, int declOK);

struct schedACM_t * schedUtilityProp_create(struct DVBS2ll_t * dvbs2ll, int nbQoS, int declOK);


#endif
