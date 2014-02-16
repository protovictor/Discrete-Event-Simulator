/**
 * @file schedACMBatch.c
 * @brief Ordonnancement par lot sur un lien ACM
 *
 */
#include <stdlib.h>    // Malloc, NULL, exit...
#include <strings.h>   // bzero, bcopy, ...
#include <stdio.h>     // printf, ...
#include <math.h>      // exp

#include <assert.h>

#include <motsim.h>
#include <schedACMBatch.h>

/*
 * Les caractéristiques d'un tel ordonnanceur
 */
struct schedACMBatch_t {
   struct schedACM_t * schedACM;
   int mode;
};

/*
 * L'ordonnanceur suivant est un ordonnanceur "nativement" par
 * lot. Cela signifie qu'il ne se contente pas d'appliquer sur une
 * séquence de trames un ordonnancement fondé sur les trames. Il
 * ordonnance globalement n trames.
 */

/**
 * @brief Ordonnanceur natif par lot fondé sur les fonctions d'utilité
 * @param sched L'ordonnanceur
 * @param mode Le mode de calcul des poids
 * Les valeurs possibles de mode sont
 *    schedBatchModeUtil
 *    schedBatchModeLength
 */
void schedulerBatchOnePass(struct schedACMBatch_t * sched, int mode)
{
   int m; //!< Le MODCOD du remplissage en cours de réalisation
   int q; //!< La file en cours pour le remplissage en cours de
	   //!réalisation

   int mBase, mIdx;
   int qBase, qIdx;

   int qb, qbBase, qbIdx;

   double poids[NB_MODCOD_MAX][3];   // WARNING
   int  deficitBitSize[NB_MODCOD_MAX][3];   // WARNING
   int cumulSurMC; // Pour evalue le nombre de trames
   double  nbTrames;		     // nécessaires sur un MC (à supprimer)
   double sommePoids;
   int taille;

   t_sequence * sequence; //!< Séquence sur laquelle on travaille

   printf_debug(DEBUG_SCHED, "IN------------------------------------------------ \n");
   if (DEBUG_SCHED&debug_mask) {
      schedACM_printFilesSummary(sched->schedACM);
   }

   // Ici, on calcule directement une séquence unique, donc on peut
   // travailler directement sur celle de l'ordonnanceur
   sequence = schedACM_getSequenceChoisie(sched->schedACM);

   if ((schedACM_getNbModCod(sched->schedACM)> NB_MODCOD_MAX) 
     || (schedACM_getNbQoS(sched->schedACM) > 3)) {
       motSim_error(MS_FATAL, "ERREUR DE DIMENSIONNEMENT");
   }

   // 1 - On commence par calculer le poids de chaque file, qui
   // provient directement de l'utilité
   sommePoids = 0.0;
   //    Pour chaque modcod envisageable
   for (m = 0; m < schedACM_getNbModCod(sched->schedACM); m++) {
      //    Pour chaque file du modcod
      for (q = 0; q < schedACM_getNbQoS(sched->schedACM); q++) {
         if ( filePDU_length(schedACM_getInputQueue(sched->schedACM, m, q)) > 0) {
            printf_debug(DEBUG_SCHED, "derivee[%d, %d] = %f (débit = %lf)\n", m, q,
		      utiliteDerivee(schedACM_getQoS(sched->schedACM, m, q),
				     schedACM_getQoS(sched->schedACM, m, q)->debit,
				     schedACM_getACMLink(sched->schedACM)),
		      schedACM_getQoS(sched->schedACM, m, q)->debit);

           if (mode == schedBatchModeUtil){
               poids[m][q] = utiliteDerivee(schedACM_getQoS(sched->schedACM, m, q),
				      schedACM_getQoS(sched->schedACM, m, q)->debit,
				      schedACM_getACMLink(sched->schedACM));
	    } else if (mode == schedBatchModeLength) {
               poids[m][q] = filePDU_size(schedACM_getInputQueue(sched->schedACM, m, q));
	    } else {
	      motSim_error(MS_FATAL, "Mode de calcul de poids inconnu !");
	    }

            printf_debug(DEBUG_SCHED, "Poids[%d, %d] = %f (débit = %lf)\n", m, q,  poids[m][q], schedACM_getQoS(sched->schedACM, m, q)->debit);
	    assert(poids[m][q]>=0);
            sommePoids += poids[m][q];
         }
      }
   }
   //printf_debug(DEBUG_SCHED, "2 ...\n");

   // 2 - On normalise 
   //    Pour chaque modcod envisageable
   printf_debug(DEBUG_ALWAYS, "--------------------AVANT----------------------\n");
   for (m = 0; m < schedACM_getNbModCod(sched->schedACM); m++) {
      cumulSurMC = 0;
      //    Pour chaque file du modcod
      for (q = 0; q < schedACM_getNbQoS(sched->schedACM); q++) {
	 poids[m][q] = poids[m][q]/sommePoids ;
	 if (filePDU_length(schedACM_getInputQueue(sched->schedACM, m, q)) > 0) {
            // On a droit au débit de notre MODCOD multiplié par le temps
            // de l'époque. Tout ça multiplié par le poids.
	    deficitBitSize[m][q] = (int)(poids[m][q]
                              * DVBS2ll_bbframePayloadBitSize(schedACM_getACMLink(sched->schedACM), m)
	                      * schedACM_getEpochMinDuration(sched->schedACM)
				      / DVBS2ll_bbframeTransmissionTime(schedACM_getACMLink(sched->schedACM), m));
	    printf_debug(DEBUG_SCHED, "Poids [%d, %d] = %f\n", m, q,  poids[m][q]);
	    printf_debug(DEBUG_SCHED, "Volume[%d, %d] = %d\n", m, q,  deficitBitSize[m][q]);
	 } else {
            poids[m][q] = 0.0;
            deficitBitSize[m][q] = 0;
	 }
	 printf_debug(DEBUG_ALWAYS, "Deficit[%d, %d] = %d (%d pq)\n", m, q, deficitBitSize[m][q]/8, filePDU_length(schedACM_getInputQueue(sched->schedACM, m, q)));
	 cumulSurMC += deficitBitSize[m][q];
      }
      nbTrames = (double)cumulSurMC / (double)DVBS2ll_bbframePayloadBitSize(schedACM_getACMLink(sched->schedACM), m);
      printf_debug(DEBUG_ALWAYS, "Il faudrait %f trames %d (%d)\n", nbTrames, m,  DVBS2ll_bbframePayloadBitSize(schedACM_getACMLink(sched->schedACM), m)/8);
   }
   printf_debug(DEBUG_ALWAYS, "-----------------------------------------------\n");

   // 3 - Pour chaque file, on envisage de prendre un volume
   //     correspondant au poids multiplié par la taille que la
   //     séquence permettrait d'émettre (avec ce MODCOD, donc). On
   //     vient de noter ce volume, de sorte à décompter au fur et à
   //     mesure des remplissages

   sequence_raz(sequence,
		schedACM_getSeqLgMax(sched->schedACM),
		schedACM_getNbModCod(sched->schedACM),
		schedACM_getNbQoS(sched->schedACM));

   mBase = 2 ; //%schedACM_getNbModCod(sched->schedACM); // Commençons par le premier !
   qBase = random()%schedACM_getNbQoS(sched->schedACM);
   mIdx = 0;
   qIdx = 0;

   m = (mBase + mIdx)%schedACM_getNbModCod(sched->schedACM);
   q = (qBase + qIdx)%schedACM_getNbQoS(sched->schedACM);

   do {
      // J'avance (m, q) jusqu'à une file qui puisse fournir un paquet
      // Si je change de MODCOD, je dois cloturer le remplissage en
      // cours (s'il y en a un) et ouvrir le prochain en fonction de m
      // Si (m, q) a bouclé, c'est fini pour le traitement pondéré

      // J'avance tant que
      // (1) m < nombre de mc
      // (2) q < nombre de files par mc
      // (3) la file (m, q) ne peut pas fournir de paquet :
      //    (3.1) la file (m, q) est (virtuellement) vide
      //    (3.2) la file (m, q) a épuisé son quota
      // (4) Je n'ai pas atteint les limites de l'époque
      while (  (mIdx < schedACM_getNbModCod(sched->schedACM))             // (1) 
	   && (qIdx < schedACM_getNbQoS(sched->schedACM))                // (2)
	      && (sequence->positionActuelle<schedACM_getSeqLgMax(sched->schedACM))  //(4)
           && (                                                       // (3)
               (sequence->remplissages[sequence->positionActuelle].nbrePaquets[m][q]
                 + sequence_nbPackets(sequence, m, q)
		>= filePDU_length(schedACM_getInputQueue(sched->schedACM, m, q))) //  (3.1)
	       ||( deficitBitSize[m][q] <= 0     // (3.2)
                 )
               )
	      ) {
         // J'avance d'une file
         qIdx = qIdx + 1;
         q = (qBase + qIdx)%schedACM_getNbQoS(sched->schedACM);
	    printf_debug(DEBUG_SCHED, "ddd m%d:q%d\n", m, q);

         // Si on a fini sur ce MODCOD, on passe au suivant
         if (qIdx == schedACM_getNbQoS(sched->schedACM)) {
            // Si un remplissage était en cours, on le cloture.
            // ATTENTION, il n'est probablement pas plein, on va
            // devoir refaire une passe sans tenir compte des poids.
 	    if (sequence->remplissages[sequence->positionActuelle].modcod == m) {
	       printf_debug(DEBUG_SCHED, "Bourrage du remplissage (reste %d)\n",
			   DVBS2ll_bbframePayloadBitSize(schedACM_getACMLink(sched->schedACM), m)/8 
			   - sequence->remplissages[sequence->positionActuelle].volumeTotal);
               printf_debug(DEBUG_SCHED, "           -------<Etat des files avant bourrage>-------\n");
               if (DEBUG_SCHED&debug_mask) {
                  schedACM_printSequenceSummary(sched->schedACM, sequence);
               }
               // Pour chaque file on ajoute ce qu'on peut
               qbBase = random()%schedACM_getNbQoS(sched->schedACM);
               for (qbIdx = 0; qbIdx < schedACM_getNbQoS(sched->schedACM); qbIdx++){
                  qb = (qbBase + qbIdx)%schedACM_getNbQoS(sched->schedACM);
		  printf_debug(DEBUG_SCHED, "qb = %d, %d+%d/%d pq (nxtsz %d, reste %d)\n",
			      qb,
			      sequence->remplissages[sequence->positionActuelle].nbrePaquets[m][qb],
			      sequence_nbPackets(sequence, m, qb),
			      filePDU_length(schedACM_getInputQueue(sched->schedACM, m, qb)),
			      (sequence->remplissages[sequence->positionActuelle].nbrePaquets[m][qb]
			       + sequence_nbPackets(sequence, m, qb)+1
			       > filePDU_length(schedACM_getInputQueue(sched->schedACM, m, qb))?0
			       :(filePDU_size_PDU_n(schedACM_getInputQueue(sched->schedACM, m, qb),
						    sequence->remplissages[sequence->positionActuelle].nbrePaquets[m][qb]
						    + sequence_nbPackets(sequence, m, qb)+1))),
			       DVBS2ll_bbframePayloadBitSize(schedACM_getACMLink(sched->schedACM), m)/8 - sequence->remplissages[sequence->positionActuelle].volumeTotal);
		 while ((sequence->remplissages[sequence->positionActuelle].nbrePaquets[m][qb]
                 + sequence_nbPackets(sequence, m, qb)
			 < filePDU_length(schedACM_getInputQueue(sched->schedACM, m, qb)))
			&& (filePDU_size_PDU_n(schedACM_getInputQueue(sched->schedACM, m, qb),
                                     sequence->remplissages[sequence->positionActuelle].nbrePaquets[m][qb]
					       + sequence_nbPackets(sequence, m, qb)+1)
			    + sequence->remplissages[sequence->positionActuelle].volumeTotal
                            <= DVBS2ll_bbframePayloadBitSize(schedACM_getACMLink(sched->schedACM), m)/8))
		   {
                      taille = filePDU_size_PDU_n(schedACM_getInputQueue(sched->schedACM, m, qb),
                                     sequence->remplissages[sequence->positionActuelle].nbrePaquets[m][qb]
						  + sequence_nbPackets(sequence, m, qb)+1);
                      // Je place le paquet dans le remplissage
                      sequence->remplissages[sequence->positionActuelle].nbrePaquets[m][qb]++;
                      sequence->remplissages[sequence->positionActuelle].volumeTotal += taille;
                      sequence->remplissages[sequence->positionActuelle].interet
	                 += (double)(taille) * poids[m][qb]*sommePoids;
                      // C'est ça de moins pour la file (m, qb)
                      deficitBitSize[m][qb] -= 8*taille;
                      printf_debug(DEBUG_SCHED, "Plus %d pour m=%d, q=%d\n", taille, m, qb);
		   }
	       }
               sequence->positionActuelle++;
	    }
            // On passe donc au MODCOD suivant (Si on en était au
            // dernier, on a fini) 
            mIdx = mIdx + 1;
            qIdx = 0;
            m = (mBase + mIdx)%schedACM_getNbModCod(sched->schedACM);
            q = (qBase + qIdx)%schedACM_getNbQoS(sched->schedACM);
	    printf_debug(DEBUG_SCHED, "m%d:q%d, mi%d:qi%d pa%d*************************************************************\n", m, q, mIdx, qIdx, sequence->positionActuelle);
	 }
         printf_debug(DEBUG_SCHED, "cccc\n");
      }
      printf_debug(DEBUG_SCHED, "aaaa\n");
      assert(qIdx < schedACM_getNbQoS(sched->schedACM));

      // Si on en est là avec m < nombre de mc alors on a une file qui
      // a un paquet à prendre . On le fait si on n'a pas atteint les
      // limites de l'époque
      if ((mIdx < schedACM_getNbModCod(sched->schedACM)) 
         && (sequence->positionActuelle < schedACM_getSeqLgMax(sched->schedACM))
	  ){
         printf_debug(DEBUG_SCHED, "Nous allons prendre un paquet pour m=%d, q=%d (deficit %d)\n", m, q,  deficitBitSize[m][q]);
         // Je détermine la taille du prochain paquet à prendre
         taille = filePDU_size_PDU_n(schedACM_getInputQueue(sched->schedACM, m, q),
                                     sequence->remplissages[sequence->positionActuelle].nbrePaquets[m][q]
                                   + sequence_nbPackets(sequence, m, q)+1);
         printf_debug(DEBUG_SCHED, "Taille du paquet : %d\n", taille);

         // S'il ne tient pas dans le remplissage en cours, je dois le
         // cloturer (j'ouvrirai le prochain plus tard ci dessous)
         if ((sequence->remplissages[sequence->positionActuelle].modcod == m) &&
             (taille + sequence->remplissages[sequence->positionActuelle].volumeTotal
             > DVBS2ll_bbframePayloadBitSize(schedACM_getACMLink(sched->schedACM), m)/8) 
            ) {
            sequence->positionActuelle++;
            printf_debug(DEBUG_SCHED, "On passe au remplissage %d\n", sequence->positionActuelle);
         }

         // Si 
         // (1) aucun remplissage n'est ouvert, et que
         // (2) les limites de l'époque me le permettent
         // j'ouvre le prochain avec le MODCOD de la file en cours
	 if ( ((sequence->positionActuelle < schedACM_getSeqLgMax(sched->schedACM))   //(2)
	       && (sequence_getDuration(sched->schedACM, sequence)
		   < schedACM_getEpochMinDuration(sched->schedACM)))
	      && (sequence->remplissages[sequence->positionActuelle].modcod == -1) // (1)
	      ){
           printf_debug(DEBUG_SCHED, "Ouverture du remplissage avec m=%d\n", m);
           sequence->remplissages[sequence->positionActuelle].modcod = m;
         }

         // Ici a priori on a un remplissage ouvert avec le bon MODCOD
         // et qui n'est pas plein
         if (   (sequence->positionActuelle <  schedACM_getSeqLgMax(sched->schedACM))
             && (sequence->remplissages[sequence->positionActuelle].modcod == m)) {
            assert(taille + sequence->remplissages[sequence->positionActuelle].volumeTotal
		   <= DVBS2ll_bbframePayloadBitSize(schedACM_getACMLink(sched->schedACM), m)/8);

            // Je place le paquet dans le remplissage
            sequence->remplissages[sequence->positionActuelle].nbrePaquets[m][q]++;
            sequence->remplissages[sequence->positionActuelle].volumeTotal += taille;
            sequence->remplissages[sequence->positionActuelle].interet
	        += (double)(taille) * poids[m][q]*sommePoids;
            // C'est ça de moins pour la file (m, q)
            deficitBitSize[m][q] -= 8*taille;
	 } else {
            // Si à ce point on n'a pas de remplissage ouvert, c'est
            // qu'il n'y a pas moyen de prendre le dernier paquet prévu
	   assert(((sequence->positionActuelle >= schedACM_getSeqLgMax(sched->schedACM)))
                || (sequence->remplissages[sequence->positionActuelle].modcod == -1));
            printf_debug(DEBUG_SCHED, "C'est la fin de la sequence\n");
           //be  cest fini 
	 }
      }
      printf_debug(DEBUG_SCHED, "bbbb\n");

      // C'est fini dès qu'une des conditions suivantes est vérifiée
      // (1) On a épuisé tous les MODCODs
      // (2) On n'a pas pu ouvrir le prochain remplissage
      // (3) On a atteint les limites de l'époque
   } while (
	       (mIdx < schedACM_getNbModCod(sched->schedACM))   // (1)
	       && (sequence->positionActuelle < schedACM_getSeqLgMax(sched->schedACM)) // (3)
               && (sequence->remplissages[sequence->positionActuelle].modcod != -1)   // (2)
           );
   printf_debug(DEBUG_SCHED, "           -------<Resultat>-------\n");
   if (DEBUG_ALWAYS&debug_mask) {
       schedACM_printSequenceSummary(sched->schedACM, sequence);
   }
   printf_debug(DEBUG_SCHED, "OUT----------------------------------------------------\n");
   printf_debug(DEBUG_ALWAYS, "--------------------APRES----------------------\n");
   for (m = 0; m < schedACM_getNbModCod(sched->schedACM); m++) {
      //    Pour chaque file du modcod
      for (q = 0; q < schedACM_getNbQoS(sched->schedACM); q++) {
	printf_debug(DEBUG_ALWAYS, "Deficit[%d, %d] = %d (rest %d pq)\n", m, q,  deficitBitSize[m][q]/8, filePDU_length(schedACM_getInputQueue(sched->schedACM, m, q)));
      }
   }
   printf_debug(DEBUG_ALWAYS, "-----------------------------------------------\n");
}

/**
 * @brief Ordonnanceur natif par lot fondé sur les fonctions d'utilité
 */
void schedulerACMBatch(struct schedACMBatch_t * sched)
{
  if (sched->mode == schedBatchModeLength) {
     schedulerBatchOnePass(sched, schedBatchModeLength);
  } else if (sched->mode == schedBatchModeUtil){
     schedulerBatchOnePass(sched, schedBatchModeUtil);
  } else if (sched->mode == schedBatchModeUtilThenLength){
     schedulerBatchOnePass(sched, schedBatchModeUtil);
     schedulerBatchOnePass(sched, schedBatchModeLength);
  }
}

/**
 * @brief Ordonnanceur par lot natif
 * Ici, on va ordonancer globalement un certain nombre de trames à chaque fois
 */
static struct schedACM_func_t schedACMBatch_func = {
   .getPDU = NULL,
   .processPDU = NULL,
   .batch = 1,
   .buildBBFRAME = NULL,
   .schedule = (void (*)(void*))schedulerACMBatch
};

/**
 * @brief Création d'un scheduler avec sa "destination".
 * Cette dernière doit  * être de type struct DVBS2ll_t  et avoir déjà
 * été complêtement construite (tous les MODCODS créés). Le nombre de
 * files de QoS différentes par MODCOD est également  passé en
 * paramètre. 
 */
struct schedACM_t * schedACMBatch_create(struct DVBS2ll_t * dvbs2ll, int nbQoS, int declOK, int seqLgMax, int mode)
{
   struct schedACMBatch_t * result = (struct schedACMBatch_t * ) sim_malloc(sizeof(struct schedACMBatch_t));
   assert(result);
   assert(declOK == 0);

   result->schedACM = schedACM_create(dvbs2ll, nbQoS, declOK, &schedACMBatch_func);
   result->mode = mode;
   schedACM_setPrivate(result->schedACM, result);
   schedACM_setSeqLgMax(result->schedACM, seqLgMax);

   printf_debug(DEBUG_SCHED, "%p created (in schedACM %p)\n", result, result->schedACM);

   return result->schedACM;
}

