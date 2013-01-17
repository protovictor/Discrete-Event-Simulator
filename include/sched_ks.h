/*
 *   Ordonnancement par un algorithme de résolution du sac à dos.
 *
 *   Cet ordonnanceur est associé à une liaison de type DVB-S2 caractérisée
 * par un certain nombre de MODCOD. Il consomme des paquets placés dans des
 * files. Un ensemble de files est associé à un MODCOD. Chaque file est
 * ensuite caractérisée par une fonction d'utilité.
 */
#ifndef __SCHED_BACKSACK
#define __SCHED_BACKSACK

#include <schedACM.h>

#define NB_SOUS_CAS_MAX 65536

struct sched_kse_t;

/*
 * Création d'un scheduler avec sa "destination". Cette dernière doit
 * être de type struct DVBS2ll_t  et avoir déjà été complêtement
 * construite (tous les MODCODS créés).
 * Le nombre de files de QoS différentes par MODCOD est également
 * passé en paramètre.
 * Si exhaustif == 1 alors tous les cas sont envisages, ce qui peut
 * faire beaucoup. Sinon, pour une taille donnée, on ne poursuit que
 * la meilleure piste.
 */
struct schedACM_t * sched_kse_create(struct DVBS2ll_t * dvbs2ll, int nbQoS, int declOK, int exhaustif);


#endif
