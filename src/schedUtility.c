/**
 * @file schedUtility.c
 * @brief Ordonnancement sur un lien DVB-S2 à base de fonctions d'utilité
 *
 *      Algorithme d'ordonnancement de flots de paquets sur un lien    
 *   DVB-S2. Cet algorithme est fondé sur des fonctions d'utilité. À
 *   chaque instant d'ordonnancement, il sert la file maximisant la
 *   dérivée de la fonction d'utilité. S'il ne peut pas compléter la
 *   BBFRAME avec, il passe à la file suivante selon cette même
 *   règle. 
 */
#include <stdlib.h>    // Malloc, NULL, exit...
#include <strings.h>   // bzero, bcopy, ...
#include <stdio.h>     // printf, ...
#include <math.h>      // exp

#include <assert.h>

#include <motsim.h>
#include <schedUtility.h>

#define propModDirect         1
#define propModProp           2
#define propModPropThenDirect 3
#define propMod               propModProp

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
   .batch = 0,
   .schedule = (void (*)(void*))schedulerUtility
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
 * Affichage de l'état du système des files d'attente avec l'interet de
 * chaque paquet étant donné le MODCOD envisagé 'mc'.
 */
void schedUtil_afficherFiles(struct schedUtility_t * sched, int mc)
{
   int m, q, n;
   int taille, id;
   double deriv ;

   printf_debug(DEBUG_ALWAYS, "Etat des files considerees: \n");
   for (m = mc; m < (schedACM_getReclassification(sched->schedACM)?nbModCod(sched->schedACM):(mc+1)); m++) {
      printf_debug(DEBUG_ALWAYS, "  MODCOD %d\n", m);
      for (q = 0; q < nbQoS(sched->schedACM); q++) {
	printf_debug(DEBUG_ALWAYS, "    QoS %d (%d PDUs)\n", q, filePDU_length(schedACM_getInputQueue(sched->schedACM, m, q)));
	 for (n = 1; n <= filePDU_length(schedACM_getInputQueue(sched->schedACM, m, q)); n++) {
            id = filePDU_id_PDU_n(schedACM_getInputQueue(sched->schedACM, m, q), n);
            taille = filePDU_size_PDU_n(schedACM_getInputQueue(sched->schedACM, m, q), n);
	    deriv = utiliteDerivee(schedACM_getQoS(sched->schedACM, m, q),
						  schedACM_getQoS(sched->schedACM, m, q)->debit,
				   schedACM_getACMLink(sched->schedACM));
            printf_debug(DEBUG_ALWAYS, "      [%d] : PDU %d (taille %d, deriv %e)\n", n, id, 
			 taille, 
			 deriv);
         }
      }
   }
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

/**
 * @brief La fonction d'ordonnancement fondée sur des fonctions d'utilité
 */
void schedulerUtility(struct schedUtility_t * sched)
{
   t_remplissage remplissage; // Le remplissage construit ici
   int m;

   remplissage_init(&remplissage, nbModCod(sched->schedACM), nbQoS(sched->schedACM));

   for (m = 0; m < nbModCod(sched->schedACM) ; m++){
      remplissage_raz(&remplissage, nbModCod(sched->schedACM), nbQoS(sched->schedACM));
      remplissage.modcod = m;

      printf_debug(DEBUG_SCHED, "Cherchons sur le modcod %d, avec les files suivantes\n", m);
#ifdef DEBUG_NDES
      if (debug_mask&DEBUG_SCHED)
        schedUtil_afficherFiles(sched,  m);
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

/**
 * @brief Version proportionnelle : On sert chaque file au prorata de sa fonction d'utilité
 */
void schedulerUtilityMCProp(struct schedUtility_t * sched, int mc, t_remplissage * remplissage)
{
   int bestQoS = 0;   
   int bestMC = mc;
   double poids[NB_MODCOD_MAX][3];   // WARNING
   double sommePoids;
   double meilleurPoids=0.0;
   int m, qa, qb, q, ma, mb; // Indices des boucles

   printf_debug(DEBUG_SCHED, "IN\n");

   if ((nbModCod(sched->schedACM)> NB_MODCOD_MAX) || (nbQoS(sched->schedACM) > 3)) {printf("***\nERREUR DE DIM\n"); exit(1);}
   printf_debug(DEBUG_SCHED, "1 ... \n");

   // 1 - On commence par calculer l'utilité sur chaque file
   sommePoids = 0.0;
   //    Pour chaque modcod envisageable
   for (m = mc; m < (schedACM_getReclassification(sched->schedACM)?nbModCod(sched->schedACM):(mc+1)); m++) {
      //    Pour chaque file du modcod
      for (q = 0; q < nbQoS(sched->schedACM); q++) {
         poids[m][q] = utiliteDerivee(schedACM_getQoS(sched->schedACM, m, q),
				      schedACM_getQoS(sched->schedACM, m, q)->debit,
				      schedACM_getACMLink(sched->schedACM));
	 assert(poids[m][q]>=0);
         sommePoids += poids[m][q];
      }
   }
   printf_debug(DEBUG_SCHED, "2 ...\n");

   // 2 - On normalise 
   //    Pour chaque modcod envisageable
   for (m = mc; m < (schedACM_getReclassification(sched->schedACM)?nbModCod(sched->schedACM):(mc+1)); m++) {
      //    Pour chaque file du modcod
      for (q = 0; q < nbQoS(sched->schedACM); q++) {
	 poids[m][q] = poids[m][q]/sommePoids ;
         printf_debug(DEBUG_SCHED, "Poids[%d, %d] = %f\n", m, q,  poids[m][q]);
	 if (poids[m][q] > meilleurPoids) {
  	    meilleurPoids = poids[m][q];
            bestMC = m;
   	    bestQoS = q;
	 }
      }
   }
   printf_debug(DEBUG_SCHED, "bestMC/bestQoS = %d/%d\n",bestMC, bestQoS);

   // 3 - Pour chaque file, on envisage de prendre un volume correspondant au poids
   //     multiplié par la taille de la BBFRAME
   for (m = mc; m < (schedACM_getReclassification(sched->schedACM)?nbModCod(sched->schedACM):(mc+1)); m++) {
      printf_debug(DEBUG_SCHED, "Lets try m = %d ...\n", m);
      //    Pour chaque file du modcod
      for (q = 0; q < nbQoS(sched->schedACM); q++) {
	/*
         printf_debug(DEBUG_SCHED, "Lets see q = %d ...\n", q);
         printf_debug(DEBUG_SCHED, "remplissage->nbrePaquets[m][q] = %d ...\n", remplissage->nbrePaquets[m][q]);
         printf_debug(DEBUG_SCHED, "filePDU_length(schedACM_getInputQueue(sched->schedACM, m, q)) = %d ...\n", filePDU_length(schedACM_getInputQueue(sched->schedACM, m, q)));
	 if (filePDU_length(schedACM_getInputQueue(sched->schedACM, m, q))) {
            printf_debug(DEBUG_SCHED, "filePDU_size_n_PDU(schedACM_getInputQueue(sched->schedACM, m, q), remplissage->nbrePaquets[m][q]+1) = %d ...\n", filePDU_size_n_PDU(schedACM_getInputQueue(sched->schedACM, m, q), remplissage->nbrePaquets[m][q]+1));
            printf_debug(DEBUG_SCHED, "poids[m][q]*(DVBS2ll_bbframePayloadBitSize(schedACM_getACMLink(sched->schedACM), mc)/8) = %f ...\n", poids[m][q]*(DVBS2ll_bbframePayloadBitSize(schedACM_getACMLink(sched->schedACM), mc)/8));
            printf_debug(DEBUG_SCHED, "remplissage->volumeTotal +filePDU_size_PDU_n(schedACM_getInputQueue(sched->schedACM, m, q), remplissage->nbrePaquets[m][q]+1) = %d ...\n", remplissage->volumeTotal +filePDU_size_PDU_n(schedACM_getInputQueue(sched->schedACM, m, q), remplissage->nbrePaquets[m][q]+1));
            printf_debug(DEBUG_SCHED, "DVBS2ll_bbframePayloadBitSize(schedACM_getACMLink(sched->schedACM), mc)/8 = %d ...\n", (DVBS2ll_bbframePayloadBitSize(schedACM_getACMLink(sched->schedACM), mc)/8));
	 }
	*/
         // Tant que (1) il reste un paquet  (2) qui tient dans la trame (3) que j'ai le droit d'émettre
	while ((remplissage->nbrePaquets[m][q] < filePDU_length(schedACM_getInputQueue(sched->schedACM, m, q))) // (1)
               &&  (remplissage->volumeTotal +filePDU_size_PDU_n(schedACM_getInputQueue(sched->schedACM, m, q), remplissage->nbrePaquets[m][q]+1) // (2)
		    <= (DVBS2ll_bbframePayloadBitSize(schedACM_getACMLink(sched->schedACM), mc)/8))
	       &&  // (3)
	       (
		 (   (propMod == propModDirect) 
		 )||((propMod == propModProp) && (
                     (filePDU_size_n_PDU(schedACM_getInputQueue(sched->schedACM, m, q), remplissage->nbrePaquets[m][q]+1)
                     <= poids[m][q]*(DVBS2ll_bbframePayloadBitSize(schedACM_getACMLink(sched->schedACM), mc)/8)) 
						  )
		 )
		)){
	  remplissage->nbrePaquets[m][q]++;
          remplissage->volumeTotal += filePDU_size_PDU_n(schedACM_getInputQueue(sched->schedACM, m, q), remplissage->nbrePaquets[m][q]);
	  /*
          printf_debug(DEBUG_SCHED, "   %d pq de %d/%d (volume %d -> cumul %d, reste %d sur %d)\n",
		       remplissage->nbrePaquets[m][q],
		       m, q,
		       filePDU_size_PDU_n(schedACM_getInputQueue(sched->schedACM, m, q),
					  remplissage->nbrePaquets[m][q]),
		       remplissage->volumeTotal,
		       (DVBS2ll_bbframePayloadBitSize(schedACM_getACMLink(sched->schedACM), mc)/8) - remplissage->volumeTotal,
		       (DVBS2ll_bbframePayloadBitSize(schedACM_getACMLink(sched->schedACM), mc)/8));
	  */
	  remplissage->interet += (double)(filePDU_size_PDU_n(schedACM_getInputQueue(sched->schedACM, m, q), remplissage->nbrePaquets[m][q])) * poids[m][q]*sommePoids;
	}
      }
   }
   printf_debug(DEBUG_SCHED, "4 ...\n");

   // 4 - On n'a pas forcément rempli : les restes laissés par chacun peuvent permettre
   //     d'émettre des paquets
   // on se dit que c'est marginal, donc on va reprendre les files dans l'ordre et bourrer !
   mb = bestMC;
   printf_debug(DEBUG_SCHED, "mb = %d\n", mb);
   for (ma = 0; ma < (schedACM_getReclassification(sched->schedACM)?nbModCod(sched->schedACM):(mc+1)) - mc; ma++) {
     printf_debug(DEBUG_SCHED, "   ma = %d, mb = %d, mc = %d, MOD = %d\n", ma, mb, mc, ((schedACM_getReclassification(sched->schedACM)?nbModCod(sched->schedACM):(mc+1))-mc));
      m = mc+((mb+ma-mc) % ((schedACM_getReclassification(sched->schedACM)?nbModCod(sched->schedACM):(mc+1))-mc));
      printf_debug(DEBUG_SCHED, "   m = %d\n", m);
      qb = bestQoS;
      printf_debug(DEBUG_SCHED, "   qb = %d\n", qb);
      for (qa = 0; qa < nbQoS(sched->schedACM); qa++) {
         printf_debug(DEBUG_SCHED, "   qa = %d\n", qa);
         q = (qa + qb)%nbQoS(sched->schedACM);
         printf_debug(DEBUG_SCHED, "   q = %d\n", q);

         while ((remplissage->nbrePaquets[m][q]
		 < filePDU_length(schedACM_getInputQueue(sched->schedACM, m, q))) // Il en reste un
	  && (remplissage->volumeTotal + filePDU_size_PDU_n(schedACM_getInputQueue(sched->schedACM, m, q),
							   remplissage->nbrePaquets[m][q]+1)
                    <= (DVBS2ll_bbframePayloadBitSize(schedACM_getACMLink(sched->schedACM), mc)/8))) {
	 remplissage->volumeTotal += 
            filePDU_size_PDU_n(schedACM_getInputQueue(sched->schedACM,
						      m, q), remplissage->nbrePaquets[m][q]+1);
         remplissage->nbrePaquets[m][q]++;
         }
      }
   }
   printf_debug(DEBUG_SCHED, "Done\n");

   printf_debug(DEBUG_SCHED, "--------- %d : v=%d/i=%f ---------\n", mc, remplissage->volumeTotal, remplissage->interet);
}

/**
 * @brief Ordonnancement d'une BBRRAME unique se fondant sur les
 * fonctions d'utilité de façon proportionnelle. 
 */
void schedulerUtilityProp(struct schedUtility_t * sched)
{
   t_remplissage remplissage; // Le remplissage construit ici
   int m,q;

   remplissage_init(&remplissage, nbModCod(sched->schedACM), nbQoS(sched->schedACM));

   for (m = 0; m < nbModCod(sched->schedACM) ; m++){
      remplissage_raz(&remplissage, nbModCod(sched->schedACM), nbQoS(sched->schedACM));
      remplissage.modcod = m;

      printf_debug(DEBUG_ALWAYS, "Cherchons sur le modcod %d, avec les files suivantes\n", m);
#ifdef DEBUG_NDES
      if (debug_mask&DEBUG_ALWAYS)
      schedUtil_afficherFiles(sched,  m);
#endif
      schedulerUtilityMCProp(sched, m, &remplissage);

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

   printf_debug(DEBUG_ALWAYS, "Solution choisie : mc %d, vol = %d, i = %f\n",
		schedACM_getSolution(sched->schedACM)->modcod,
		schedACM_getSolution(sched->schedACM)->volumeTotal,
		schedACM_getSolution(sched->schedACM)->interet);

   for (m = 0; m < nbModCod(sched->schedACM); m++) {
      for (q = 0; q < nbQoS(sched->schedACM); q++) {
	printf_debug(DEBUG_ALWAYS, "[%d][%d] - %d\n", m, q, schedACM_getSolution(sched->schedACM)->nbrePaquets[m][q]);
      }
   }

}

static struct schedACM_func_t schedUtilityProp_func = {
   .getPDU = NULL,
   .processPDU = NULL,
   .buildBBFRAME = NULL,
   .batch = 0,
   .schedule = (void (*)(void*))schedulerUtilityProp
};

/*
 * Création d'un scheduler avec sa "destination". Cette dernière doit
 * être de type struct DVBS2ll_t  et avoir déjà été complêtement
 * construite (tous les MODCODS créés).
 * Le nombre de files de QoS différentes par MODCOD est également
 * passé en paramètre.
 */
struct schedACM_t * schedUtilityProp_create(struct DVBS2ll_t * dvbs2ll, int nbQoS, int declOK)
{
   struct schedUtility_t * result = (struct schedUtility_t * ) sim_malloc(sizeof(struct schedUtility_t));
   assert(result);

   result->schedACM = schedACM_create(dvbs2ll, nbQoS, declOK, &schedUtilityProp_func);
   schedACM_setPrivate(result->schedACM, result);

   printf_debug(DEBUG_SCHED, "%p created (in schedACM %p)\n", result, result->schedACM);

   return result->schedACM;
}

/**********************************************************************************************/
/**********************************************************************************************/
/************************************    VERSION PAR LOT **************************************/
/**********************************************************************************************/
/**********************************************************************************************/
/**
 * @brief Ordonnancement d'un lot de BBFRAMEs se fondant sur les
 * fonctions d'utilité de façon proportionnelle.
 *
 * Cet algorithme va chercher une séquence de trames optimale et va
 * placer le résultat dans le champs sequenceChoisie du scheduler.
 */
void schedulerUtilityPropBatch(struct schedUtility_t * sched)
{
   t_sequence sequenceChoisie; //! < La séquence que l'on va construire
   t_sequence sequence; //! < Une séquence générique
   int m,q;

   sequence_init(&sequence, schedACM_getLgMax(sched->schedACM), nbModCod(sched->schedACM), nbQoS(sched->schedACM));

   // On parcourt l'ensemble des séquences de la façon suivante

   // 1 - On construit une séquence
   // 2 - Si c'est la meilleure, on la sauvegarde
}

/**
 * @brief Version par lot
 * Ici, on va ordonanncer un certain nombre de trames à chaque fois
 */
static struct schedACM_func_t schedUtilityPropBatch_func = {
   .getPDU = NULL,
   .processPDU = NULL,
   .batch = 1,
   .buildBBFRAME = NULL,
   .schedule = (void (*)(void*))schedulerUtilityPropBatch
};

/**
 * @brief Création d'un scheduler avec sa "destination".
 * Cette dernière doit
 * être de type struct DVBS2ll_t  et avoir déjà été complêtement
 * construite (tous les MODCODS créés).
 * Le nombre de files de QoS différentes par MODCOD est également
 * passé en paramètre.
 */
struct schedACM_t * schedUtilityPropBatch_create(struct DVBS2ll_t * dvbs2ll, int nbQoS, int declOK, int seqLgMax)
{
   struct schedUtility_t * result = (struct schedUtility_t * ) sim_malloc(sizeof(struct schedUtility_t));
   assert(result);

   result->schedACM = schedACM_create(dvbs2ll, nbQoS, declOK, &schedUtilityPropBatch_func);
   schedACM_setPrivate(result->schedACM, result);
   schedACM_setSeqLgMax(result->schedACM, seqLgMax);

   printf_debug(DEBUG_SCHED, "%p created (in schedACM %p)\n", result, result->schedACM);

   return result->schedACM;
}
