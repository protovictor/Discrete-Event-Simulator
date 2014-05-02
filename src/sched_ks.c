/** @file sched_ks.c
 *  @brief Ordonnacement sur un lien DVB-S2
 * 
 *      Implantation d'un algorithme d'ordonnancement de flots de
 *      paquets sur un lien DVB-S2. Cet algorithme se fonde sur le
 *      problème du sac à dos. Il se veut donc efficace, mais son
 *      temps d'exécution peut devenir explosif !
 */
 
#include <stdlib.h>    // Malloc, NULL, exit...
#include <strings.h>   // bzero, bcopy, ...
#include <stdio.h>     // printf, ...
#include <math.h>      // exp

#include <assert.h>

#include <motsim.h>
#include <sched_ks.h>

void scheduler_knapsack_exhaustif(struct sched_kse_t * sched);

static struct schedACM_func_t schedKS_func = {
   .getPDU = NULL,
   .processPDU = NULL,
   .buildBBFRAME = NULL,
   .schedule = (void (*)(void *))scheduler_knapsack_exhaustif
};

/**
 * @brief : Les caractéristiques d'un tel ordonnanceur
 */
struct sched_kse_t {
   struct schedACM_t * schedACM;

   // La chose suivante est une variable de l'algo. Je sais que c'est crade de
   // la mettre là, mais   elle est lourde à initialiser dynamiquement !
   t_remplissage remplissage[NB_SOUS_CAS_MAX];

   // Cherche-t-on vraiment tous les cas ?
   int rechercheExhaustive;

};

/**
 * @brief Création et initialisation d'un ordonnanceur
 * @param dvbs2ll le lien sur lequel sont transmises les trames
 * @param nbQoS le nombre de files de qualité de service
 * @param declOK autorise-t-on le déclassement ?
 * @param exhaustif faut-il utiliser un algorithme exhaustif ?
 *
 * Création d'un scheduler avec sa "destination". Cette dernière doit
 * être de type struct DVBS2ll_t  et avoir déjà été complêtement
 * construite (tous les MODCODS créés).
 * Le nombre de files de QoS différentes par MODCOD est également
 * passé en paramètre.
 */
struct schedACM_t * sched_kse_create(struct DVBS2ll_t * dvbs2ll,
				     int nbQoS,
				     int declOK,
				     int exhaustif)
{
   struct sched_kse_t * result = (struct sched_kse_t * ) sim_malloc(sizeof(struct sched_kse_t));
   assert(result);

   result->schedACM = schedACM_create(dvbs2ll, nbQoS, declOK, &schedKS_func);
   schedACM_setPrivate(result->schedACM, result);

   // On initialise le tableau de l'algo
   tabRemplissage_init(result->remplissage, NB_SOUS_CAS_MAX, DVBS2ll_nbModcod(dvbs2ll), nbQoS);

   printf_debug(DEBUG_KS, "%p created (in schedACM %p)\n", result, result->schedACM);

   result->rechercheExhaustive = exhaustif;

   return result->schedACM;

}

/*
 * Calcul de l'impact sur la fonction d'utilité de l'émission d'un
 * paquet de taille 'taillePaquet' de la file de QoS 'qos' et de modcod
 * 'mcFile' dans une BBFRAME associée au MODCOD 'mc'.
 * 'taillePrecedente' est le volume des autres paquets (de la même
 * file, mais antérieurs) qui doivent être émis dans la même BBFRAME.
 * 2012-02-27 : Cette valeur n'est plus utilisée dans la nouvelle version
 */
double gainUtilite(t_qosMgt * qos, int taillePaquet, int mcBBFRAME, struct DVBS2ll_t * dvbs2ll)
{
   double result = 0.0;
   double tpsEmission = DVBS2ll_bbframeTransmissionTime(dvbs2ll, mcBBFRAME);

   result = utiliteDerivee(qos, qos->debit, dvbs2ll);
   result = result *taillePaquet/tpsEmission ;

   return result;
}

/*
 * Affichage de l'état du système des files d'attente avec l'interet de
 * chaque paquet étant donné le MODCOD envisagé 'mc'.
 */
void KS_afficherFiles(struct sched_kse_t * sched, int mc)
{
   int m, q, n;
   int taille, id;
   double gain ;
   printf_debug(DEBUG_KS, "Etat des files considerees: \n");
   for (m = mc; m < (schedACM_getReclassification(sched->schedACM)?schedACM_getNbModCod(sched->schedACM):(mc+1)); m++) {
      printf_debug(DEBUG_KS, "  MODCOD %d\n", m);
      for (q = 0; q < schedACM_getNbQoS(sched->schedACM); q++) {
	printf_debug(DEBUG_KS, "    QoS %d (%d PDUs)\n", q, filePDU_length(schedACM_getInputQueue(sched->schedACM, m, q)));
	 for (n = 1; n <= filePDU_length(schedACM_getInputQueue(sched->schedACM, m, q)); n++) {
            id = filePDU_id_PDU_n(schedACM_getInputQueue(sched->schedACM, m, q), n);
            taille = filePDU_size_PDU_n(schedACM_getInputQueue(sched->schedACM, m, q), n);
	    gain = gainUtilite(schedACM_getQoS(sched->schedACM, m, q), taille, mc, schedACM_getACMLink(sched->schedACM));
            printf_debug(DEBUG_KS, "      [%d] : PDU %d (taille %d, gain %f)\n", n, id, 
			 taille, 
			 gain);
         }
      }
   }
}

/**
 * @brief Resolution exhaustive du problème du sac à dos avec une
 * BBFRAME dont le modcod est passé en paramètre 
 * @param mc le numéro du MODCOD à tester
 * @param sched l'ordonnanceur à utiliser
 *
 * Cette fonction utilise le tableau remplissage de la structure
 * sched. Ce tableau doit être initialisé à 0, et il sera réinitialisé
 * à la fin de cette fonction.
 *
 * Si la meilleure solution trouvée pour ce MODCOD est meilleur que la
 * meilleure solution du scheduler, alors elle la remplace.
 */
void knapsackParModCod(int mc, struct sched_kse_t * sched)
{
   
   struct DVBS2ll_t * dvbs2ll = schedACM_getACMLink(sched->schedACM);
   int rCourant = 0; // Le rang de la taille de BBFRAME courante
   int rProchain, rS, rDispo, rMeilleur;
   int tp;
   int m, q, qa, qb;
   double interet;
   int volume;
   int fini;
   //   int choice; // Pour tirer au hasard en cas d'égalité
   int mt, qt, doublon; // Pour la recherche de doublon dans la démarche exhaustive

   do { // Pour chaque taille de BBFRAME
      printf_debug(DEBUG_KS_VERB, "       ---< Solution %d en cours d'analyse (volume %d / interet %5.2e) >---\n",
		   rCourant, sched->remplissage[rCourant].volumeTotal, sched->remplissage[rCourant].interet);
      printf_debug(DEBUG_KS_VERB, "--repartition envisagee--\n");
      for (m = 0; m < schedACM_getNbModCod(sched->schedACM); m++) {
         for (q = 0; q < schedACM_getNbQoS(sched->schedACM); q++) {
	   printf_debug(DEBUG_KS_VERB, "  [m=%d][q=%d] : %2d\n", m, q, sched->remplissage[rCourant].nbrePaquets[m][q]);
	 }
      }
      printf_debug(DEBUG_KS_VERB, "------------------------------\n");

      // Recherche de tous les sched->remplissages atteignables 
      // Pour chaque file d'attente du MODCOD mc ou d'un MODCOD permettant le déclassement ...
      for (m = mc; m < (schedACM_getReclassification(sched->schedACM)?schedACM_getNbModCod(sched->schedACM):(mc+1)); m++) {
         qb = random()%schedACM_getNbQoS(sched->schedACM);
         for (qa = 0; qa < schedACM_getNbQoS(sched->schedACM); qa++) {
            q = (qa + qb)%schedACM_getNbQoS(sched->schedACM);

            printf_debug(DEBUG_KS_VERB, "m/q = %d/%d, rCourant = %d (v %d, i %5.2e)\n", m, q, rCourant, sched->remplissage[rCourant].volumeTotal, sched->remplissage[rCourant].interet);
	    printf_debug(DEBUG_KS_VERB, "  Paquet de taille %d de q[%d][%d] (%d/%d paquets) dans BBF de %d oqp/%d ?\n",
                       (filePDU_length(schedACM_getInputQueue(sched->schedACM, m, q))>(sched->remplissage[rCourant].nbrePaquets[m][q]))?filePDU_size_PDU_n(schedACM_getInputQueue(sched->schedACM, m, q), sched->remplissage[rCourant].nbrePaquets[m][q]+1):0,
                       m, q,
                       sched->remplissage[rCourant].nbrePaquets[m][q],
                       filePDU_length(schedACM_getInputQueue(sched->schedACM, m, q)),
		       sched->remplissage[rCourant].volumeTotal,
		       DVBS2ll_bbframePayloadBitSize(dvbs2ll, mc)/8);
            // ... s'il reste un paquet qui tienne (pas de fragmentation pour le moment) ...
            if ((sched->remplissage[rCourant].nbrePaquets[m][q] < filePDU_length(schedACM_getInputQueue(sched->schedACM, m, q)))
		&& (sched->remplissage[rCourant].volumeTotal + filePDU_size_PDU_n(schedACM_getInputQueue(sched->schedACM, m, q), sched->remplissage[rCourant].nbrePaquets[m][q]+1) <= (DVBS2ll_bbframePayloadBitSize(dvbs2ll, mc)/8))) {
	       printf_debug(DEBUG_KS_VERB, "      Oui\n");
               tp = filePDU_size_PDU_n(schedACM_getInputQueue(sched->schedACM, m, q), sched->remplissage[rCourant].nbrePaquets[m][q]+1);
               printf_debug(DEBUG_KS_VERB, "         (taille %d)\n", tp);
               // ... quelle influence a ce remplissage sur l'utilité ?
               interet = sched->remplissage[rCourant].interet + gainUtilite(schedACM_getQoS(sched->schedACM, m, q), tp, mc, schedACM_getACMLink(sched->schedACM));
               printf_debug(DEBUG_KS_VERB, "         (interet %5.2e)\n", interet);
               // ... et à quel volume total cela nous conduit ?
               volume = sched->remplissage[rCourant].volumeTotal + tp;
               printf_debug(DEBUG_KS_VERB, "         vers volume %d\n", volume);

               if (sched->rechercheExhaustive) {
                  // Cherchons une place
                  rDispo = rCourant + 1; 
                  while ((rDispo < NB_SOUS_CAS_MAX) && (sched->remplissage[rDispo].volumeTotal != 0)) {
                     rDispo++;
	          }
                  // Cherchons si la meme configuration existait déjà
                  rS =  1;
                  doublon = 0;
                  while ((!doublon) &&(rS < NB_SOUS_CAS_MAX) && (sched->remplissage[rS].volumeTotal != 0)) {
		     if (sched->remplissage[rS].volumeTotal == volume) {
                       doublon = 1; // C'est la même jusqu'à preuve du contraire
                       // On compare file par file 
		       for (mt = 0; mt < schedACM_getNbModCod(sched->schedACM); mt++) {
                          for (qt = 0; qt < schedACM_getNbQoS(sched->schedACM); qt++) {
			    if ((mt==m) && (qt==q)) {
                               doublon = doublon 
				 && (sched->remplissage[rCourant].nbrePaquets[mt][qt] +1
				     == sched->remplissage[rS].nbrePaquets[mt][qt] );
			    } else {
                               doublon = doublon 
				 && (sched->remplissage[rCourant].nbrePaquets[mt][qt]
				     == sched->remplissage[rS].nbrePaquets[mt][qt] );
			    }
			  }
		       }
		       if (doublon) {
                          sched->remplissage[rS].nbChoix++;
		       }
		     }
                     rS++;
	          }
                  // Sinon, il faut sauver ce nouveau résultat
	          if (!doublon) {
                     if (rDispo < NB_SOUS_CAS_MAX)  {
                        schedACM_tryingNewSolution(sched->schedACM);
                        printf_debug(DEBUG_KS_VERB, "             Nouvel etat cree bis [id %d, taille %d]\n", rDispo, volume);
		        remplissage_copy(&(sched->remplissage[rCourant]), &(sched->remplissage[rDispo]), schedACM_getNbModCod(sched->schedACM), schedACM_getNbQoS(sched->schedACM));
                        sched->remplissage[rDispo].volumeTotal = volume;
                        sched->remplissage[rDispo].interet = interet;
                        sched->remplissage[rDispo].nbChoix = 1;
                        sched->remplissage[rDispo].nbrePaquets[m][q]++;
		     } else {
/* A des fins de debogage, on va dumper les cas, pour voir !
	   	        motSim_error(MS_WARN, "NB_SOUS_CAS_MAX=%d mal dimensionne bis (%d etats crees)!!!\n",
				     NB_SOUS_CAS_MAX,
				     schedACM_getNbSolutions(sched->schedACM));
		        for (rS=0; rS<NB_SOUS_CAS_MAX; rS++){
		 	  printf("[%d] %d ", rS, sched->remplissage[rS].volumeTotal);
		 	  for(mt = 0; mt < schedACM_getNbModCod(sched->schedACM); mt++) {
                             for (qt = 0; qt < schedACM_getNbQoS(sched->schedACM); qt++) {
			       printf("%d ", sched->remplissage[rS].nbrePaquets[mt][qt]);
			     }
                             printf("- ");
                          }
			  printf("(%d)\n", sched->remplissage[rS].nbChoix);
		        }
*/
	   	        motSim_error(MS_FATAL, "NB_SOUS_CAS_MAX=%d mal dimensionne bis (%d etats crees)!!!\n",
				     NB_SOUS_CAS_MAX,
				     schedACM_getNbSolutions(sched->schedACM));

		     }
	          } else {
	             printf_debug(DEBUG_KS_VERB, "             Deja vu\n");
		  }

	       } else { // Recherche non totalement exhaustive !!
                  // Nous avons donc une remplissage pour le knapsack de taille 'volume' !
                  // Cherchons s'il en existait déjà une ...
                  rS = rCourant + 1;
                  assert(rS < NB_SOUS_CAS_MAX);
                  rDispo = NB_SOUS_CAS_MAX; // On va en profiter pour chercher une place !
                  while ((sched->remplissage[rS].volumeTotal != volume) && (rS < NB_SOUS_CAS_MAX)) {
                     if ((sched->remplissage[rS].volumeTotal == 0) && (rDispo == NB_SOUS_CAS_MAX)) {
                        rDispo = rS; // On note la première place libre
	             }
                     rS++;
	          }
                  // S'il en existait une, ...
                  if ((rS < NB_SOUS_CAS_MAX) && (sched->remplissage[rS].volumeTotal == volume)) {
   		     printf_debug(DEBUG_KS_VERB, "         Etat %d (interet %f, %d choix)\n", rS, sched->remplissage[rS].interet, sched->remplissage[rS].nbChoix);
		     // En cas d'égalité (est-ce possible ?), on va tirer au sort
                     if (sched->remplissage[rS].interet == interet) {
                        sched->remplissage[rS].nbChoix++;  // Une de plus qui mène ici
		        //                     choice = (int)((random()/(RAND_MAX + 1.0))*sched->remplissage[rS].nbChoix);
		        //     		     printf("%d/%d\n", choice, sched->remplissage[rS].nbChoix);
		     }
                     // ... fait-on mieux ?
                     if  (    (sched->remplissage[rS].interet < interet)  // 2012-03-08 pas mieux si égalité
		  	   //		       || ((sched->remplissage[rS].interet == interet) && (choice == 1))
		          ){
                        printf_debug(DEBUG_KS_VERB, "            Mieux que %5.2f\n", sched->remplissage[rS].interet);
                        // Si oui, on remplace 
		        remplissage_copy(&(sched->remplissage[rCourant]), &(sched->remplissage[rS]), schedACM_getNbModCod(sched->schedACM), schedACM_getNbQoS(sched->schedACM));
                        sched->remplissage[rS].volumeTotal = volume;
                        sched->remplissage[rS].interet = interet;
                        sched->remplissage[rS].nbrePaquets[m][q]++;
    		     } else {
                        printf_debug(DEBUG_KS_VERB, "           Pas mieux que %5.2f\n", sched->remplissage[rS].interet);
		     }
                  // Sinon, il faut sauver ce nouveau résultat
	          } else if (rDispo < NB_SOUS_CAS_MAX)  {
                     schedACM_tryingNewSolution(sched->schedACM);
                     printf_debug(DEBUG_KS_VERB, "             Nouvel etat cree [id %d, taille %d]\n", rDispo, volume);
		     remplissage_copy(&(sched->remplissage[rCourant]), &(sched->remplissage[rDispo]), schedACM_getNbModCod(sched->schedACM), schedACM_getNbQoS(sched->schedACM));
                     sched->remplissage[rDispo].volumeTotal = volume;
                     sched->remplissage[rDispo].interet = interet;
                     sched->remplissage[rDispo].nbChoix = 1;
                     sched->remplissage[rDispo].nbrePaquets[m][q]++;
		  } else {
	   	     motSim_error(MS_FATAL, "NB_SOUS_CAS_MAX=%d mal dimensionne !!!\n", NB_SOUS_CAS_MAX);
	          }
	       }
            } else {
	       printf_debug(DEBUG_KS_VERB, "      Non\n");
	    }
         } // for q ....
      } // for m ...

      sched->remplissage[rCourant].casTraite = 1; // C'est boucle pour ce cas !
      // rCourant avance vers le plus petit prochain (ie plus gros que le courant)
      //  knapsack résolu s'il existe (il peut ne plus y avoir de paquet !)
      // On en profite pour chercher la meilleure solution trouvée pour le moment
      // parce que si c'est fini, il faut la donner
      volume = sched->remplissage[rCourant].volumeTotal;
      rProchain = 0;
      rMeilleur = 0;
      rS = rProchain + 1;
      while ((rS < NB_SOUS_CAS_MAX) && (sched->remplissage[rS].volumeTotal > 0)) {
         // Si c'est un plus gros snapsack que le courant ...
	//	if (sched->remplissage[rS].volumeTotal > volume) {
	if (!sched->remplissage[rS].casTraite) {
            // ... et le plus petit trouvé ...
            if ((sched->remplissage[rS].volumeTotal < sched->remplissage[rProchain].volumeTotal) || (rProchain == 0)) {
               // ... alors c'est le prochain !
               rProchain = rS;
	    }
         }
         // Si c'est la meilleure solution
         if (sched->remplissage[rS].interet > sched->remplissage[rMeilleur].interet) {
            rMeilleur = rS;
	 }
         rS++;
      }

      // Si on a trouvé un nouveau cas (non plein), on y va !
      if ((rProchain != 0) && (8*sched->remplissage[rProchain].volumeTotal < DVBS2ll_bbframePayloadBitSize(dvbs2ll, mc))) {
         printf_debug(DEBUG_KS_VERB, "Passons au cas %d (t %d, i %5.2e)\n", rProchain, sched->remplissage[rProchain].volumeTotal, sched->remplissage[rProchain].interet);
         rCourant = rProchain;
         fini = 0;
      } else {
         printf_debug(DEBUG_KS_VERB, "Fini meilleur cas %d (t %d, i %5.2e)\n", rMeilleur, sched->remplissage[rMeilleur].volumeTotal, sched->remplissage[rMeilleur].interet);

         fini = 1;
      }
   } while (!fini);

   // On sauvegarde la meilleure solution touvée ici
   // (on écrase le cas échéant la précédente)

   if (sched->remplissage[rMeilleur].interet > schedACM_getSolution(sched->schedACM)->interet) {
      remplissage_copy(&(sched->remplissage[rMeilleur]), schedACM_getSolution(sched->schedACM),
		       schedACM_getNbModCod(sched->schedACM), schedACM_getNbQoS(sched->schedACM));
      schedACM_getSolution(sched->schedACM)->modcod = mc;
   };

   // On nettoie le tableau des remplissages
   tabRemplissage_raz(sched->remplissage, NB_SOUS_CAS_MAX, schedACM_getNbModCod(sched->schedACM), schedACM_getNbQoS(sched->schedACM));
}

/**
 * @brief Application de l'ordonnancement
 * @param sched structure définissant l'ordonnanceur à utiliser
 *
 * C'est cette fonction qui réalise l'ordonnancement général. Elle
 * passe en revue tous les MODCODs disponibles et applique
 * l'algorithme sur chacun d'entre eux. Au final, l'ordonnanceur
 * contient dans son champ solutionChoisie (obtenue par la fonction
 * schedACM_getSolution) la meilleure de toutes les solutions
 * envisagées.
 */
void scheduler_knapsack_exhaustif(struct sched_kse_t * sched)
{
   struct DVBS2ll_t * dvbs2ll = schedACM_getACMLink(sched->schedACM);

   int mc;
   int q, m;

   /** On met à zéro la solution choisie
    */
   remplissage_raz(schedACM_getSolution(sched->schedACM),
		   schedACM_getNbModCod(sched->schedACM),
		   schedACM_getNbQoS(sched->schedACM));

   /*
    * Resolution pour tous les MODCODs
    */
   printf_debug(DEBUG_KS, "********************DEBUT KNAPSACK****************************\n");
   //   KS_afficherFiles(sched, 0);
   for (mc = 0; mc < schedACM_getNbModCod(sched->schedACM); mc++) {
      printf_debug(DEBUG_KS, "-------====< MODCOD %d >====-------\n", mc);
#ifdef DEBUG_NDES
      if (debug_mask&DEBUG_ALWAYS)
      KS_afficherFiles(sched, mc);
#endif
      knapsackParModCod(mc, sched);

      printf_debug(DEBUG_KS, "-------====< Ordonnancement choisi mc=%d >====-------\n",
		   schedACM_getSolution(sched->schedACM)->modcod);
      printf_debug(DEBUG_KS, "Nombre de solutions testées : %d\n",
		   schedACM_getNbSolutions(sched->schedACM));
      for (m = 0; m < schedACM_getNbModCod(sched->schedACM); m++) {
         printf_debug(DEBUG_KS, "  MODCOD %d\n", m);
         for (q = 0; q < schedACM_getNbQoS(sched->schedACM); q++) {
            printf_debug(DEBUG_KS, "   Ord[m=%d][q=%d] = %2d\n",
			 m, q, schedACM_getSolution(sched->schedACM)->nbrePaquets[m][q]);
         }
      }
      printf_debug(DEBUG_KS, "  Interet %7.2e  Volume %d/%d (mc %d)\n",
  		schedACM_getSolution(sched->schedACM)->interet,
		schedACM_getSolution(sched->schedACM)->volumeTotal,
		DVBS2ll_bbframePayloadBitSize(dvbs2ll, schedACM_getSolution(sched->schedACM)->modcod)/8,
		schedACM_getSolution(sched->schedACM)->modcod);
   }
   printf_debug(DEBUG_KS, "%d sol, %d in %d\n", schedACM_getNbSolutions(sched->schedACM), 8*schedACM_getSolution(sched->schedACM)->volumeTotal,DVBS2ll_bbframePayloadBitSize(dvbs2ll, schedACM_getSolution(sched->schedACM)->modcod));
   printf_debug(DEBUG_KS, "**********************FIN KNAPSACK****************************\n");

}

