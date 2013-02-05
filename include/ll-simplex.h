/*
 * Définition d'un lien unidirectionnel
 */
#ifndef __DEF_LL_SIMPLEX
#define __DEF_LL_SIMPLEX

#include <pdu.h>

/*
 * Création d'une entité. Les deux paramètres importants sont le
 * débits (en bits/s) et le temps de propagation (en secondes).
 */
struct llSimplex_t * llSimplex_create(void * destination,
				      processPDU_t destProcessPDU,
				      unsigned long throughput,
				      double propagation);

/*
 * Destruction
 */
void llSimplex_delete(struct llSimplex_t * lls);

/*
 * Gestion d'une PDU soumise par l'amont
 */
int DVBS2ll_processPDU(struct DVBS2ll_t * dvbs2ll,
                        getPDU_t getPDU,
                        void * source);

/*
 * Fourniture d'une PDU en aval
 */
struct PDU_t * DVBS2ll_getPDU(struct DVBS2ll_t * dvbs2l);


#endif
