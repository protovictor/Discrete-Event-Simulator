/*----------------------------------------------------------------------*/
/*      Algorithme d'ordonnancement de flots de paquets sur un lien     */
/*   DVB-S2. Cet algorithme est fondé sur des fonctions d'utilité.      */
/*----------------------------------------------------------------------*/
#include <stdlib.h>    // Malloc, NULL, exit...
#include <strings.h>   // bzero, bcopy, ...
#include <stdio.h>     // printf, ...
#include <math.h>      // exp

#include <assert.h>

#include <motsim.h>
#include <schedUtility.h>

/*
 * Les caractéristiques d'un tel ordonnanceur
 */
struct schedUtility_t {
   struct schedACM_t * schedACM;
};

void schedulerUtility(struct schedUtility_t * sched);

static struct schedACM_func_t schedUtility_func = {
   .getPDU = NULL,
   .processPDU = NULL,
   .buildBBFRAME = NULL,
   .schedule = schedulerUtility
};

/*
 * Création d'un scheduler avec sa "destination". Cette dernière doit
 * être de type struct DVBS2ll_t  et avoir déjà été complêtement
 * construite (tous les MODCODS créés).
 * Le nombre de files de QoS différentes par MODCOD est également
 * passé en paramètre.
 */
struct schedACM_t * schedUtility_create(struct DVBS2ll_t * dvbs2ll, int nbQoS, int declOK)
{
   struct schedUtility_t * result = (struct schedUtility_t * ) sim_malloc(sizeof(struct schedUtility_t));
   assert(result);

   result->schedACM = schedACM_create(dvbs2ll, nbQoS, declOK, &schedUtility_func);
   schedACM_setPrivate(result->schedACM, result);

   printf_debug(DEBUG_SCHED, "%p created (in schedACM %p)\n", result, result->schedACM);

   return result->schedACM;
}

/*
 * La fonction d'ordonnancement appliquée au ModCod donné
 */
void schedulerUtilityMC(struct schedUtility_t * sched, int mc, t_remplissage * remplissage)
{
   int bestQoS = 0;   
   int bestMC = mc;
   int paquetDispo; // reste-t-il au moins un paquet ?

   int m, qa, qb, q; // Indices des boucles

   printf_debug(DEBUG_SCHED, "--------- recherche sur le MC %d ---------\n", mc);
   // Tant qu'on n'a pas trouvé de quoi remplir la BBFRAME et qu'il
   // reste des choses
   do {
      paquetDispo = 0; // On va voir si on en trouve
      // 1. On cherche dans les files non (potentiellement) vides celle qui a
      // la meilleure dérivé de sa fonction d'utilité
      for (m = mc; m < (schedACM_getReclassification(sched->schedACM)?nbModCod(sched->schedACM):(mc+1)); m++) {

         // On démarre la boucle aléatoirement pour éviter un biais en
         // cas d'égalité
         qb = random()%nbQoS(sched->schedACM);
         for (qa = 0; qa < nbQoS(sched->schedACM); qa++) {
            q = (qa + qb)%nbQoS(sched->schedACM);

/*	    printf_debug(DEBUG_ALWAYS, "Etudions [%d][%d](type %d, beta %f) - (déjà %d paquets) reste %d pq, bw %f util %f\n",
			 m, q,
			 schedACM_getQoS(sched->schedACM, m, q)->typeQoS,
			 schedACM_getQoS(sched->schedACM, m, q)->beta,
			 remplissage->nbrePaquets[m][q],
			 filePDU_length(schedACM_getInputQueue(sched->schedACM, m, q)),
			 schedACM_getQoS(sched->schedACM, m, q)->debit,
			 utiliteDerivee(schedACM_getQoS(sched->schedACM, m, q),
					schedACM_getQoS(sched->schedACM, m, q)->debit,
					schedACM_getACMLink(sched->schedACM)));
*/
            // Cette file peut-elle fournir au moins un paquet
            // (première clause) qui tienne (deuxième) ?
            if ((remplissage->nbrePaquets[m][q] < filePDU_length(schedACM_getInputQueue(sched->schedACM, m, q))) // Il en reste un
		&& (remplissage->volumeTotal + filePDU_size_PDU_n(schedACM_getInputQueue(sched->schedACM, m, q), remplissage->nbrePaquets[m][q]+1)
                    <= (DVBS2ll_bbframePayloadBitSize(schedACM_getACMLink(sched->schedACM), mc)/8))) {
	      schedACM_tryingNewSolution(sched->schedACM); // On compte les solutions envisagées
	       // Si oui, est-elle la première ou, sinon, plus intéressante que la plus
	       // intéressante ?
	       if ((paquetDispo == 0)
                || (remplissage->interet < utiliteDerivee(schedACM_getQoS(sched->schedACM, m, q),
						  schedACM_getQoS(sched->schedACM, m, q)->debit,
						  schedACM_getACMLink(sched->schedACM)))){
                  paquetDispo = 1;
      		  bestMC = m;
		  bestQoS = q;
/*
  		  printf("Meilleur interet pour le moment : %f (was %f)\n",
			 utiliteDerivee(schedACM_getQoS(sched->schedACM, m, q),
					schedACM_getQoS(sched->schedACM, m, q)->debit,
					schedACM_getACMLink(sched->schedACM)),
			 remplissage->interet);
*/
		  remplissage->interet = utiliteDerivee(schedACM_getQoS(sched->schedACM, m, q),
						schedACM_getQoS(sched->schedACM, m, q)->debit,
						schedACM_getACMLink(sched->schedACM));
	       }
	    }
	 }
      }

      // 2. On prend tout ce qu'on peut dans cette file
      //      printf_debug(DEBUG_SCHED, "On rempli ...\n");
      while ((remplissage->nbrePaquets[bestMC][bestQoS]
               < filePDU_length(schedACM_getInputQueue(sched->schedACM, bestMC, bestQoS))) // Il en reste un
	  && (remplissage->volumeTotal + filePDU_size_PDU_n(schedACM_getInputQueue(sched->schedACM, bestMC, bestQoS),
							   remplissage->nbrePaquets[bestMC][bestQoS]+1)
                    <= (DVBS2ll_bbframePayloadBitSize(schedACM_getACMLink(sched->schedACM), mc)/8))) {
	 remplissage->volumeTotal += 
            filePDU_size_PDU_n(schedACM_getInputQueue(sched->schedACM,
						      bestMC, bestQoS), remplissage->nbrePaquets[bestMC][bestQoS]+1);
         remplissage->nbrePaquets[bestMC][bestQoS]++;
      } 
      //      printf("%d pq de [%d][%d] (vol %d)\n", remplissage->nbrePaquets[bestMC][bestQoS], bestMC, bestQoS, remplissage->volumeTotal);
   } while (paquetDispo);
   printf_debug(DEBUG_SCHED, "--------- %d : v=%d/i=%f ---------\n", mc, remplissage->volumeTotal, remplissage->interet);
}

void schedulerUtility(struct schedUtility_t * sched)
{
   t_remplissage remplissage; // Le remplissage construit ici
   int m, q;

   remplissage_init(&remplissage, nbModCod(sched->schedACM), nbQoS(sched->schedACM));

   for (m = 0; m < nbModCod(sched->schedACM) ; m++){
      remplissage_raz(&remplissage, nbModCod(sched->schedACM), nbQoS(sched->schedACM));
      remplissage.modcod = m;

      printf_debug(DEBUG_SCHED, "Cherchons sur le modcod %d, avec les files suivantes\n", m);
#ifdef DEBUG_NDES
      if (debug_mask&DEBUG_SCHED)
        schedACM_afficherFiles(sched->schedACM,  m);
#endif
      schedulerUtilityMC(sched, m, &remplissage);

      // N'a qqchose ?
      if (remplissage.volumeTotal) {

         // C'est mieux si au moins une des affirmations suivantes est vraie
         //    on n'avait rien pour le moment (volumeTotal nul)
         //    l'interet de la nouvelle proposition est meilleur
	 //    l'interet est le même, mais avec un plus gros volume
         if (   (schedACM_getSolution(sched->schedACM)->volumeTotal == 0)
	       || (remplissage.interet > schedACM_getSolution(sched->schedACM)->interet)
	       || (   (remplissage.interet == schedACM_getSolution(sched->schedACM)->interet)
		 && (remplissage.volumeTotal > schedACM_getSolution(sched->schedACM)->volumeTotal))) {
            remplissage_copy(&remplissage, schedACM_getSolution(sched->schedACM),
		       nbModCod(sched->schedACM), nbQoS(sched->schedACM));
            schedACM_getSolution(sched->schedACM)->modcod = m;
         }
      }
   }

   printf_debug(DEBUG_SCHED, "Solution choisie : mc %d, vol = %d, i = %f\n",
		schedACM_getSolution(sched->schedACM)->modcod,
		schedACM_getSolution(sched->schedACM)->volumeTotal,
		schedACM_getSolution(sched->schedACM)->interet);
/*
   for (m = 0; m < nbModCod(sched->schedACM); m++) {
      for (q = 0; q < nbQoS(sched->schedACM); q++) {
	printf_debug(DEBUG_SCHED, "[%d][%d] - %d\n", m, q, schedACM_getSolution(sched->schedACM)->nbrePaquets[m][q]);
      }
   }
*/
}
