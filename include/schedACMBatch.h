/**
 * @file schedACMBatch.h
 * @brief Ordonnancement par lot sur un lien ACM
 *
 */
#ifndef __SCHED_ACM_BATCH
#define __SCHED_ACM_BATCH

#include <schedACM.h>

#define schedBatchModeLength 1
#define schedBatchModeDuration 2
#define schedBatchModeUtil 3
#define schedBatchModeUtilThenLength  4
#define schedBatchModeUtilThenDuration 5
#define schedBatchModeDurationThenLength 6

/**
 * @brief Création d'un scheduler avec sa "destination".
 * Cette dernière doit  * être de type struct DVBS2ll_t  et avoir déjà
 * été complêtement construite (tous les MODCODS créés). Le nombre de
 * files de QoS différentes par MODCOD est également  passé en
 * paramètre. 
 */
struct schedACM_t * schedACMBatch_create(struct DVBS2ll_t * dvbs2ll, int nbQoS, int declOK, int seqLgMax, int mode);

#endif
