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
int llSimplex_processPDU(void * lls,
                        getPDU_t getPDU,
                        void * source);

/*
 * Fourniture d'une PDU en aval
 */
struct PDU_t * llSimplex_getPDU(void * lls);


#endif
