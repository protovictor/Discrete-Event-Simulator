/**
 * @file schedACM.c
 * @brief Outils de base pour les ordonnanceurs ACM
 *
 */
#include <math.h>      // exp, pow, ...
#include <string.h>    // strcat

#include <schedACM.h>


// A virer

unsigned long nbRemplissageAlloc = 0 ;
unsigned long nbRemplissageFree = 0;

/*
 * A chaque file est associÃ©e une QoS
struct fileQoS {
   struct filePDU_t * file;
   t_qosMgt           qos;
};
 */

/**
 * @brief Caractérisation générale d'un ordonnanceur sur ACM
 */
struct schedACM_t {
   int nbQoS;      //!< Le nombre de files de QoS
   int nbModCod;    //!< Le nombre de MODCOD en aval

   // Les sources sont forcÃ©ment des files, mais il serait 
   // bon de les utiliser de faÃ§on cohÃ©rente avec le modÃ¨le I/O
   struct filePDU_t *** files;  //!< Les files d'attentes
   t_qosMgt          ** qos;
   int                  declassement;

   // La destination est forcément un lien DVBS2, mais il serait 
   // bon de l'utiliser de façon cohérente avec le modèle I/O
   // De plus, on pourra gÃ©nÃ©raliser sans trop de difficultÃ©s (?)
   // Ã  un ACM
   struct DVBS2ll_t * dvbs2ll; // Le lien sur lequel on transmet

   struct probe_t **** pqFromMQinMC;
   //!< pqFromMQinMC[m][q][mc] est une probe qui compte la taille et le nombre de
   //!< paquets de la file [m][q] qui sont transmis par le MODCOD mc.

   struct schedACM_func_t * func;

   int paquetsEnAttente; //!< Ai-je au moins un pq en attente ?

   // Dans la version la plus simple, l'algorithme d'ordonnancenemt
   // doit déterminer un remplissage pour la prochaine BBFRAME. Il
   // place alors sa solution dans le champ suivant
   t_remplissage solutionChoisie; //!< La solution choisie lors du
				  //!dernier ordonnancement

   int nbSol; //!< Le nombre de solutions envisagÃ©es
   struct probe_t * nbSolProbe;

   // Dans la version plus évoluée, l'ordonnanceur est amené à
   // calculer plusieurs BBFRAMEs d'un coup. Il sera donc invoqué
   // plus rarement et proposera le remplissage de plusieurs BBFRAMEs
   // qui seront utilisées par autant d'appels consécutifs à
   // getPDU. La liste des remplissages est stockée dans le champ
   // suivant.
   t_sequence sequenceChoisie;
   int        seqLgMax;  //!< La longueur maximale d'une séquence 
   double     epochMinDuration; //!< La durée minimale entre deux
				//!calculs 
   double    currentEpochStartTime ; //!< Ce n'est pas nécessairement
				     //!périodique
   struct probe_t * epochDurationProbe;
   int nbEpoch;
   int nbEpochStarvation;

   // Données privées
   void * private;
};

/**
 * @brief Création d'un scheduler avec sa "destination"
 * Cette derniÃ¨re doit
 * Ãªtre de type struct DVBS2ll_t  et avoir dÃ©jÃ  Ã©tÃ© complÃªtement
 * construite (tous les MODCODS crÃ©Ã©s).
 * Le nombre de files de QoS diffÃ©rentes par MODCOD est Ã©galement
 * passÃ© en paramÃ¨tre.
 */
struct schedACM_t * schedACM_create(struct DVBS2ll_t * dvbs2ll, int nbQoS, int declOK,
				    struct schedACM_func_t * func)
{
   int i, j, k;

   struct schedACM_t * result = (struct schedACM_t * ) sim_malloc(sizeof(struct schedACM_t));
   assert(result);

   printf_debug(DEBUG_ACM, "mc from dvb2 : %d, nbQoS : %d\n", DVBS2ll_nbModcod(dvbs2ll), nbQoS);

   result->dvbs2ll = dvbs2ll;

   // La destination doit nous connaitre
   DVBS2ll_setSource(dvbs2ll, result, (getPDU_t)schedACM_getPDU);

   result->nbQoS = nbQoS;
   result->nbModCod = DVBS2ll_nbModcod(dvbs2ll);

   // Allocation des tableaux de files, qos et paramÃ¨tres
   result->files = (struct filePDU_t ***)sim_malloc(sizeof(struct filePDU_t **)*result->nbModCod);
   assert(result->files);
   result->qos = (t_qosMgt **)sim_malloc(sizeof(t_qosMgt *)*result->nbModCod);
   assert(result->qos);

   for (i = 0; i < result->nbModCod; i++) {
      result->files[i] = (struct filePDU_t **)sim_malloc(sizeof(struct filePDU_t *)*result->nbQoS);
      assert(result->files[i]);
      result->qos[i] = (t_qosMgt *)sim_malloc(sizeof(t_qosMgt)*result->nbQoS);
      assert(result->qos[i]);
      for (j = 0; j < nbQoS; j++) {
         result->files[i][j] = NULL;
	 result->qos[i][j].typeQoS = -1;  // WARNING bof
	 result->qos[i][j].beta    = 0.0; // WARNING bof
	 result->qos[i][j].debit   = 0.00001; // WARNING bof !!! Le
					      // soucis c'est que la
					      // valeur 0.0 pose de
					      // gros problèmes
					      // (certaines fonctions
					      // ou dérivées n'y sont
					      // pas définies !!)
         result->qos[i][j].bwProbe = NULL;
      }
   }

   // Allocation des tableaux de probes pour la rÃ©partition des paquets
   result->pqFromMQinMC = (struct probe_t ****)sim_malloc(result->nbModCod*sizeof(struct probe_t ***));
   for (i = 0; i < result->nbModCod; i++) {
      result->pqFromMQinMC[i] = (struct probe_t ***)sim_malloc(result->nbQoS*sizeof(struct probe_t **));
      for (j = 0; j < nbQoS; j++) {
         result->pqFromMQinMC[i][j] = (struct probe_t **)sim_malloc(result->nbModCod*sizeof(struct probe_t *));
         for (k = 0; k < result->nbModCod; k++) {
            result->pqFromMQinMC[i][j][k] = (struct probe_t *)NULL;
	 }
      }
   }
   remplissage_init(&(result->solutionChoisie), DVBS2ll_nbModcod(dvbs2ll), nbQoS);

   // Pas de batch par défaut
   result->seqLgMax = 1;
   result->epochMinDuration = 0.0;
   result->currentEpochStartTime = 0.0;
   result->epochDurationProbe = NULL;
   result->nbEpoch = 0;
   result->nbEpochStarvation = 0;

   result->nbSol = 0;
   result->nbSolProbe = NULL;

   result->declassement = declOK;
   result->private = NULL;

   result->func = func;
   printf_debug(DEBUG_ACM, "%p created (link : %p)\n", result, result->dvbs2ll);

   return result;

}

/*
 * Y a-t-il des paquets en attente ? Le rÃ©sultat est boolÃ©en
 */
int schedACM_getPacketsWaiting(struct schedACM_t * sched)
{
   return sched->paquetsEnAttente;
}

/*
 * Si les fonctions getPDU et processPDU sont redÃ©finies, la prÃ©sence
 * de paquets en attente n'est plus mise Ã  jour. Il faut donc
 * l'assurer par des appels Ã  la fonction suivante.
 */
void schedACM_setPacketsWaiting(struct schedACM_t * sched, int b)
{
   sched->paquetsEnAttente = b;
}

/*
 * Modification des donnÃ©es privÃ©es.
 */
void schedACM_setPrivate(struct schedACM_t * sched, void * private)
{
   sched->private = private;
}

/*
 * Obtention des donnÃ©es privÃ©es
 */
void * schedACM_getPrivate(struct schedACM_t * sched)
{
   return sched->private;
}

/*
 * Ajout d'une sonde pour compter les paquets d'une file (m, q) Ã©mis par un
 * MODCOD mc (mc peut Ãªtre < m en cas de reclassement)
 */
void schedACM_setPqFromMQinMC(struct schedACM_t * sched, int m, int q, int mc, struct probe_t * pr)
{
   sched->pqFromMQinMC[m][q][mc] = pr;
}

/*
 * Attribution des files d'attente d'entrÃ©e pour un MODCOD donnÃ© dans
 * le paramÃ¨tre mc. Le paramÃ¨tre files est un tableau de pointeurs sur
 * des files de PDU. Il doit en contenir au moins nbQoS. Les nbQoS
 * premiÃ¨res seront utilisÃ©es ici.
 */
void schedACM_setInputQueues(struct schedACM_t * sched, int mc, struct filePDU_t * files[])
{
   int j;

   for (j = 0; j < sched->nbQoS; j++) {
      assert(files[j] != NULL);
      sched->files[mc][j] = files[j];
   }
}

/*
 * Affectation d'une sonde permettant de suivre le dÃ©bit estimÃ© par
 * l'algorithme pour chaque file
 */
void schedACM_addThoughputProbe(struct schedACM_t * sched, int m, int q, struct probe_t * bwProbe)
{
   sched->qos[m][q].bwProbe = probe_chain(bwProbe, sched->qos[m][q].bwProbe);
}

/*
 * Attribution du type de QoS d'une file. La file est identififÃ©e par
 * (mc, qos), le type de QoS voulue est passÃ©e en paramÃ¨tre, ainsi
 * qu'un Ã©ventuel paramÃ¨tre de pondÃ©ration.
 */
void schedACM_setFileQoSType(struct schedACM_t * sched, int mc, int qos, int qosType, double beta, double rmin)
{
   sched->qos[mc][qos].typeQoS = qosType; 
   sched->qos[mc][qos].beta = beta;
   sched->qos[mc][qos].rmin = rmin;
}

/*
 * Consultation du nombre de ModCod
 */
inline int schedACM_getNbModCod(struct schedACM_t * sched)
{
   return sched->nbModCod;
}

/*
 * Consultation du nombre de QoS par MODCOD
 */
inline int schedACM_getNbQoS(struct schedACM_t * sched)
{
   return sched->nbQoS;
}

/*
 * Obtention d'un pointeur sur une des files
 */
inline struct filePDU_t * schedACM_getInputQueue(struct schedACM_t * sched, int mc, int qos)
{
   return sched->files[mc][qos];
}

/*
 * Peut-on faire du "dÃ©classement" ?
 */
inline int schedACM_getReclassification(struct schedACM_t * sched)
{
   return sched->declassement;
}

/*
 * Obtention d'un pointeur sur une des QoS
 */
inline t_qosMgt * schedACM_getQoS(struct schedACM_t * sched, int mc, int qos)
{
   return &(sched->qos[mc][qos]);
}

/*
 * Obtention d'un pointeur sur le lien
 */
inline struct DVBS2ll_t * schedACM_getACMLink(struct schedACM_t * sched)
{
   return sched->dvbs2ll;
}

/*
 * Obtention d'un pointeur vers une sonde
 */
struct probe_t *  schedACM_getPqFromMQinMC(struct schedACM_t * sched, int m, int q, int mc)
{
  return sched->pqFromMQinMC[m][q][mc];
}

/**
 * @brief Obtention d'un pointeur sur la solution choisie
 * C'est vraiment pas propre, mais le type est opaque, ...
 */
t_remplissage * schedACM_getSolution(struct schedACM_t * sched)
{
   return &sched->solutionChoisie;
}

/**
 * @brief Obtention d'un pointeur sur la séquence choisie
 * C'est vraiment pas propre, mais le type est opaque, ...
 */
t_sequence * schedACM_getSequenceChoisie(struct schedACM_t * sched)
{
   return &sched->sequenceChoisie;
}

/*
 *   Fonction à invoquer par l'ordonnanceur pour décompter les solutions
 */
void schedACM_tryingNewSolution(struct schedACM_t * sched)
{
   sched->nbSol++;
}

/*
 * Ajout d'une sonde permettant de mesurer le nombre de solutions testées
 */
void schedACM_addNbSolProbe(struct schedACM_t * sched, struct probe_t * probe)
{
   probe_chain(probe, sched->nbSolProbe);
   sched->nbSolProbe = probe;
}

/**
 * @brief Combien de solutions testÃ©es ?
 */
int schedACM_getNbSolutions(struct schedACM_t * sched)
{
  return sched->nbSol;
};

/**
 * @brief Choix de la longueur maximale d'une séquence
 */
void schedACM_setSeqLgMax(struct schedACM_t * sched, int seqLgMax)
{
   assert(sched->func->batch != 0);
   sched->seqLgMax = seqLgMax;
   sequence_init(&(sched->sequenceChoisie), seqLgMax, schedACM_getNbModCod(sched), schedACM_getNbQoS(sched));
}

/**
 * @brief Lecture de la longureur maximale d'une séquence
 */
int schedACM_getSeqLgMax(struct schedACM_t * sched)
{
   return sched->seqLgMax;
}

/**
 * @brief Initialisation de la durée max d'une époque dans le cas d'un
 * ordonnancement par lot
 * @param sched L'ordonnanceur à paramétrer
 * @param minDur La durée minimale
 */
void schedACM_setEpochMinDuration(struct schedACM_t * sched, double minDur)
{
   sched->epochMinDuration = minDur;
   sched->currentEpochStartTime = -minDur -1.0; // Pour être certain de
					     // faire un premier ordonnancement
}
/**
 * @brief Consultation de la durée min de l'époque
 * @param sched Le scheduler concerné
 * @result La durée minimale d'une époque
 */
double schedACM_getEpochMinDuration(struct schedACM_t * sched)
{
   return sched->epochMinDuration;
}

void schedACM_addEpochTimeDurationProbe(struct schedACM_t * sched, struct probe_t * epochDurationProbe)
{
   sched->epochDurationProbe = probe_chain(epochDurationProbe, sched->epochDurationProbe);
}

/**
 * @brief Combien d'époques calculées ?
 * @param sched L'ordonnanceur visé
 * @result Le nombre d'époques qui ont été calculées
 */
int schedACM_getNbEpoch(struct schedACM_t * sched)
{
   return sched->nbEpoch;
}

/**
 * @brief Combien d'époques en famine calculées ?
 * @param sched L'ordonnanceur visé
 * @result Le nombre d'époques en famine qui ont été calculées
 */
int schedACM_getNbEpochStarvation(struct schedACM_t * sched)
{
   return sched->nbEpochStarvation;
}

/********************************************************************************/
/*   La dérivée de la fonction d'utilitÃ©                                        */
/********************************************************************************/

/*
 * Calcul de la valeur en x de la derivee d'une fonction d'utilitÃ©
 * Le paramÃ¨tre dvbs2ll est ici nÃ©cessaire pour certaines fonctions
 * Il faudra envisager de mettre ces info (le dÃ©bit du lien en gros
 * pour le moment) dans la structure t_qosMgt, ou pas !)
 */
double utiliteDerivee(t_qosMgt * qos, double x, struct DVBS2ll_t * dvbs2ll)
{
   double result = 0.0;
   double debitBinaire;

   switch (qos->typeQoS) {
      case kseQoS_log :
         result = qos->beta/x;   // 2012-02-07 : remplacement de 1.0 au numÃ©rateur
         printf_debug(DEBUG_NEVER, "x=%f, beta=%f, result(log)=%f\n", x, qos->beta, result);
      break;

      case kseQoS_lin :             // UtilitÃ© linÃ©aire
         result = qos->beta;
      break;

      case kseQoS_exp : 
	result = 1.0 + qos->beta * exp(qos->beta * (qos->rmin - x));
      break;

      case kseQoS_exn : 
        debitBinaire = (double)DVBS2ll_bbframePayloadBitSize(dvbs2ll, 0)
                       /DVBS2ll_bbframeTransmissionTime(dvbs2ll, 0); // WARNING : on normalise sur le premier, c'est arbitraire !!
	result = 1.0 + qos->beta * exp(qos->beta * (qos->rmin - x)/debitBinaire);
      break;

      default : 
         motSim_error(MS_FATAL, "QoS de type %d non implantée\n", qos->typeQoS);
   }

   return result;
}

/********************************************************************************/
/*   Les fonctions liÃ©es au modÃ¨le d'entrÃ©e/sortie                              */
/********************************************************************************/

/*
 * L'ordonnanceur lui-même
 */
void schedACM_schedule(struct schedACM_t * sched)
{
   remplissage_raz(&(sched->solutionChoisie), schedACM_getNbModCod(sched), schedACM_getNbQoS(sched));

   sched->nbSol = 0;

   if (sched->func && sched->func->schedule) {
      printf_debug(DEBUG_ACM, "calling dedicated facility ...\n");
      sched->func->schedule(sched->private);
      if (sched->nbSolProbe) {
 	 probe_sample(sched->nbSolProbe, sched->nbSol);
      }
   } else {
       motSim_error(MS_FATAL, "need a scheduler !\n");
   }
}

/**
 * @brief Création d'une BBFRAME en fonction d'un remplissage calculé
 * par un ordonnanceur
 * @param sched L'ordonnanceur qui doit émettre
 * @param solution Le remplissage calculé préalablement
 * Cette fonction positionne également le champs
 * sched->paquetsEnAttente en fonction de ce qu'elle laisse dans les
 * files. 
 */
struct PDU_t * schedACM_buildBBRFRAMEFromRemplissage(struct schedACM_t * sched, t_remplissage * solution)
{
   int q, m, p, vol, s;
   double alphaMaaike;
   struct PDU_t * pdu = NULL;

   // Si on trouve au moins un paquet à envoyer
   if (solution->volumeTotal) {
         vol = 0;
         // On extrait les paquets concernés depuis les files d'entrÃ©
         for (m = 0; m < schedACM_getNbModCod(sched); m++) {
            for (q = 0; q < schedACM_getNbQoS(sched); q++) {
               s = 0;
               for (p = 0 ; p < solution->nbrePaquets[m][q]; p++){
                  pdu = filePDU_extract(schedACM_getInputQueue(sched, m, q));
                  s += PDU_size(pdu);
	          printf_debug(DEBUG_ACM, "Sortie du paquet %d de taille %d de la file (%d, %d)\n", PDU_id(pdu), PDU_size(pdu), m, q);
		  probe_sample(schedACM_getPqFromMQinMC(sched, m, q, solution->modcod), PDU_size(pdu));
                  PDU_free(pdu);
               }
	       vol += s;
               // Mise à jour des débits (utilisés pour le calcul du
               // gain). ATTENTION : est-il normal que ce soit fait
               // ici !? On ne peut pas utiliser le débit observé par
               // la file elle-même (en environnement réel, ce n'est
               // pas disponible), c'est le scheduler qui doit le
               // calculer. 
               alphaMaaike = pow(alpha, 1000.0*DVBS2ll_bbframeTransmissionTime(schedACM_getACMLink(sched),
									       solution->modcod));
	       schedACM_getQoS(sched, m, q)->debit = calculeEMA(schedACM_getQoS(sched, m, q)->debit,
						   8.0*s/DVBS2ll_bbframeTransmissionTime(schedACM_getACMLink(sched),
											 solution->modcod),
						   alphaMaaike); 
	       if (schedACM_getQoS(sched, m, q)->bwProbe)
                  probe_sample(schedACM_getQoS(sched, m, q)->bwProbe, schedACM_getQoS(sched, m, q)->debit);

               // Reste-t-il au moins un paquet pour une prochaine
               // BBFRAME ? ATTENTION, on pourrait mettre ça dans
               // schedACM_buildBBFRAMEGeneric 
	       sched->paquetsEnAttente = sched->paquetsEnAttente || (filePDU_length(schedACM_getInputQueue(sched, m, q)) > 0);
            }
         }
         assert(vol == solution->volumeTotal);

         printf_debug(DEBUG_ACM, "Construction d'une BBFRAME du modcod %d\n", solution->modcod);


         // On construit une BBFRAME
         assert(solution->modcod >= 0);
         assert(solution->modcod < schedACM_getNbModCod(sched));

         // Attention, on passe l'indice du MODCOD en champs privé de la PDU
         // récupéré en particulier dans DVBS2ll_sendPDU. C'est pas
         // top, il  faudra plutôt faire une file par MODCOD
         pdu = PDU_create(vol, (void *)(long)solution->modcod);
      }
      return pdu;
}

/**
 * @brief Construction générique d'une BBFRAME
 *
 * Construction d'une BBFRAME avec les paquets en attente dans les
 * files s'il y en a suffisemment. Sinon, un pointeur NULL est retourné.
 */
struct PDU_t * schedACM_buildBBFRAMEGeneric(struct schedACM_t * sched)
{
  /*
   int q, m, p, vol, s;
   double alphaMaaike;
  */
   struct PDU_t * pdu = NULL;

   printf_debug(DEBUG_ACM, "Packets waiting ? %s \n", sched->paquetsEnAttente?"Oui":"non");

   if (sched->paquetsEnAttente) {
      // A priori, on va vider les paquets en attente
      sched->paquetsEnAttente = 0;

      // Recherche de l'ordonnancement par application de l'algo
      printf_debug(DEBUG_ACM, "On schedule ...\n");
      schedACM_schedule(sched);
      printf_debug(DEBUG_ACM, "Volume propose : %d\n", sched->solutionChoisie.volumeTotal);

      // Une solution a été calculée dans le champs
      // solutionChoisie. On construit une PDU fondée sur cette
      // solution
      pdu = schedACM_buildBBRFRAMEFromRemplissage(sched, &sched->solutionChoisie);
       
      /*
      // Si on trouve au moins un paquet à envoyer
      if (sched->solutionChoisie.volumeTotal) {
         vol = 0;
         // On extrait les paquets concernés depuis les files d'entrÃ©
         for (m = 0; m < schedACM_getNbModCod(sched); m++) {
            for (q = 0; q < schedACM_getNbQoS(sched); q++) {
               s = 0;
               for (p = 0 ; p < sched->solutionChoisie.nbrePaquets[m][q]; p++){
                  pdu = filePDU_extract(schedACM_getInputQueue(sched, m, q));
                  s += PDU_size(pdu);
	          printf_debug(DEBUG_ACM, "Sortie du paquet %d de taille %d de la file (%d, %d)\n", PDU_id(pdu), PDU_size(pdu), m, q);
		  probe_sample(schedACM_getPqFromMQinMC(sched, m, q, sched->solutionChoisie.modcod), PDU_size(pdu));
                  PDU_free(pdu);
               }
	       vol += s;
               // Mise à jour des débits (utilisés pour le calcul du
               // gain). ATTENTION : est-il normal que ce soit fait
               // ici !? On ne peut pas utiliser le débit observé par
               // la file elle-même (en environnement réel, ce n'est
               // pas disponible), c'est le scheduler qui doit le
               // calculer. 
               alphaMaaike = pow(alpha, 1000.0*DVBS2ll_bbframeTransmissionTime(schedACM_getACMLink(sched),
									       sched->solutionChoisie.modcod));
	       schedACM_getQoS(sched, m, q)->debit = calculeEMA(schedACM_getQoS(sched, m, q)->debit,
						   8.0*s/DVBS2ll_bbframeTransmissionTime(schedACM_getACMLink(sched),
											 sched->solutionChoisie.modcod),
						   alphaMaaike); 
	       if (schedACM_getQoS(sched, m, q)->bwProbe)
                  probe_sample(schedACM_getQoS(sched, m, q)->bwProbe, schedACM_getQoS(sched, m, q)->debit);

               // Reste-t-il au moins un paquet pour une prochaine BBFRAME ?
	       sched->paquetsEnAttente = sched->paquetsEnAttente || (filePDU_length(schedACM_getInputQueue(sched, m, q)) > 0);
            }
         }
         assert(vol == sched->solutionChoisie.volumeTotal);

         printf_debug(DEBUG_ACM, "Construction d'une BBFRAME du modcod %d\n", sched->solutionChoisie.modcod);


         // On construit une BBFRAME
         assert(sched->solutionChoisie.modcod >= 0);
         assert(sched->solutionChoisie.modcod < schedACM_getNbModCod(sched));

         // Attention, on passe l'indice du MODCOD en champs privé de la PDU
         // récupéré en particulier dans DVBS2ll_sendPDU. C'est pas
         // top, il  faudra plutôt faire une file par MODCOD
         pdu = PDU_create(vol, (void *)sched->solutionChoisie.modcod);
      }
      */
   }
      
   return pdu;
}

/**
 * @brief Construction générique d'une BBFRAME avec un ordonnanceur
 * "par lot" 
 *
 * Construction d'une BBFRAME avec les paquets en attente dans les
 * files s'il y en a suffisemment. Sinon, un pointeur NULL est retourné.
 */
struct PDU_t * schedACM_buildBBFRAMEGenericBatch(struct schedACM_t * sched)
{
   struct PDU_t * pdu = NULL;
   int r;

   printf_debug(DEBUG_ACM, "IN\n");

   assert(sched->func->batch == 1);

   // Si la séquence calculée est vide, c'est qu'il est temps de s'y
   // mettre ! 
   if (sched->sequenceChoisie.positionActuelle == 0) {
      // On ne peut ordonnancer que si l'époque précédente est finie
      if (motSim_getCurrentTime() >= sched->currentEpochStartTime + sched->epochMinDuration) {
         // Recherche de l'ordonnancement par application de l'algo
         printf_debug(DEBUG_ACM, "On schedule ...\n");
         schedACM_schedule(sched);
	 // On compte les époques et les époques non pleines
         sched->nbEpoch ++;
	 if (sequence_getDuration(sched, &sched->sequenceChoisie) < sched->epochMinDuration){
            sched->nbEpochStarvation++;
	 }
         if (DEBUG_SCHED&debug_mask) {
            printf_debug(DEBUG_SCHED, "Sequence proposee de %d trames : ", sched->sequenceChoisie.positionActuelle);
            for (r = 0 ; r < sched->sequenceChoisie.positionActuelle; r++){
               printf("%d ", sched->sequenceChoisie.remplissages[r].modcod);
            }
            printf("\n");
	 }
         if ( sched->currentEpochStartTime > 0.0) {
            probe_sample(sched->epochDurationProbe, motSim_getCurrentTime() - sched->currentEpochStartTime);
	 }
	 sched->currentEpochStartTime = motSim_getCurrentTime();
      }
   }
   printf_debug(DEBUG_ACM, "Looking for a frame\n");

   // Tant que la séquence calculée n'est pas vide, on prend la
   // prochaine trame de cette séquence.
   if (sched->sequenceChoisie.nextFrameToSend < sched->sequenceChoisie.positionActuelle) {
      printf_debug(DEBUG_ACM, "There is one ...\n");
      pdu = schedACM_buildBBRFRAMEFromRemplissage(sched, &(sched->sequenceChoisie.remplissages[sched->sequenceChoisie.nextFrameToSend]));
      printf_debug(DEBUG_ACM, "On incremente pour plus tard ...\n");
      sched->sequenceChoisie.nextFrameToSend++;
      // Si on vient de consommer la dernière, il faut invalider la
      // séquence afin d'en forcer le recalcul 
      if (sched->sequenceChoisie.nextFrameToSend == sched->sequenceChoisie.positionActuelle ){
         printf_debug(DEBUG_ACM, "End of epoch ...\n");
         sched->sequenceChoisie.nextFrameToSend = 0;
         sched->sequenceChoisie.positionActuelle = 0; //!< Ca suffit pour
						   //!déclancher
						   //!l'ordonnancement
						   //!(cf plus haut) 
      }
   }
   
   return pdu;
}

/**
 * @brief Construction d'une BBFRAME
 * 
 * Construction d'une BBFRAME avec les paquets en attente dans les
 * files s'il y en a suffisemment. Sinon, un pointeur NULL est retournÃ©.
 */
struct PDU_t * schedACM_buildBBFRAME(struct schedACM_t * sched)
{
   if (sched->func && sched->func->buildBBFRAME) {
      printf_debug(DEBUG_ACM, "calling dedicated facility ...\n");
      return sched->func->buildBBFRAME(sched->private);
   } else {
      if (sched->func->batch) {
         printf_debug(DEBUG_ACM, "calling generic batch facility ...\n");
         return schedACM_buildBBFRAMEGenericBatch(sched);
      } else {
         printf_debug(DEBUG_ACM, "calling generic facility ...\n");
         return schedACM_buildBBFRAMEGeneric(sched);
      }
   }
}

/*
 * Fonction invoquÃ©e lors de la disponibilitÃ© d'un paquet dans une des
 * files.
 * En fait, ici on ne fait rien. L'activitÃ©e sera dictÃ©e par la
 * disponibilitÃ© du support. C'est uniquement sur des Ã©vÃ©nements
 * de l'aval que l'on agit, ...
 * WARNING c'est pas génial, à revoir !
 */
int schedACM_processPDUGeneric(struct schedACM_t * sched,
                                getPDU_t getPDU, void * source)
{
   struct PDU_t * pdu;

   // Si c'est juste pour tester si je suis pret
   if ((getPDU == NULL) || (source == NULL)) {
      return 1; // On fait comme si on était pret puisque on gère nous même
   }

   printf_debug(DEBUG_ACM, "Un paquet dispo pour %p\n", sched);

   sched->paquetsEnAttente = 1;    //WARNING, pourquoi uniquement dans ce cas !?

   // Si par hasard le support est dispo, il faut prendre l'initiative
   if (DVBS2ll_available(schedACM_getACMLink(sched))) {
      printf_debug(DEBUG_ACM, "Support libre\n");
      pdu = schedACM_buildBBFRAME(sched);
      // WARNING TBD remplacer par le modÃ¨le classique (destination, ...)
      DVBS2ll_sendPDU(schedACM_getACMLink(sched), pdu);
   } else { // Je me le note pour êre prêt lorsque le support sera dispo
      sched->paquetsEnAttente = 1;    //WARNING, pourquoi uniquement dans ce cas !?
      printf_debug(DEBUG_ACM, "Support occupe\n");
   }
   return 0; // ?
}

/**
 * @brief Fonction générique de production d'une PDU
 *
 * @param sched L'ordonnanceur qui doit fournir la PDU
 * @result la PDU créée (NULL si rien n'est disponible)
 *
 * Fonction Ã  invoquer lorsque le support est libre afin de solliciter
 * la construction d'une nouvelle BBFRAME. Cette fonction générique
 * sera utilisée si l'ordonnanceur n'en implante pas une version
 * spécifique.
 * Elle se contente de faire appel à la fonction de création d'une
 * BBFRAME.
 */
struct PDU_t * schedACM_getPDUGeneric(struct schedACM_t * sched)
{
   struct PDU_t * result;

   printf_debug(DEBUG_ACM, "Le destinataire veut une BBFRAME\n");

   // L'ordonnanceur nous en produit-il une ?
   result = schedACM_buildBBFRAME(sched);

   return result;
}


/*
 * Fonction Ã  invoquer lorsque le support est libre afin de solliciter
 * la construction d'une nouvelle BBFRAME
 */
struct PDU_t * schedACM_getPDU(struct schedACM_t * sched)
{
   if (sched->func && sched->func->getPDU) {
      printf_debug(DEBUG_ACM, "calling dedicated facility ...\n");
      return sched->func->getPDU(sched->private);
   } else {
      printf_debug(DEBUG_ACM, "calling generic facility ...\n");
      return schedACM_getPDUGeneric(sched);
   }
}

/*
 * Fonction invoquÃ©e lors de la disponibilitÃ© d'un paquet dans une des
 * files.
 * En fait, ici on ne fait rien. L'activitÃ©e sera dictÃ©e par la
 * disponibilitÃ© du support. C'est uniquement sur des Ã©vÃ©nements
 * de l'aval que l'on agit, ...
 */
int schedACM_processPDU(struct schedACM_t * sched,
                         getPDU_t getPDU, void * source)
{
   if (sched->func && sched->func->processPDU) {
      printf_debug(DEBUG_ACM, "calling dedicated facility ...\n");
      return sched->func->processPDU(sched->private, getPDU, source);
   } else {
      printf_debug(DEBUG_ACM, "calling generic facility ...\n");
      return schedACM_processPDUGeneric(sched, getPDU, source);
   }
}

/********************************************************************************/


/*
 * Affichage de l'état du système des files d'attente avec l'interet de
 * chaque paquet étant donné le MODCOD envisagé 'mc'.
 */
void schedACM_afficherFiles(struct schedACM_t * sched, int mc)
{
   int m, q, n;
   int taille, id;

   printf_debug(DEBUG_SCHED, "Etat des files considerees: \n");
   for (m = mc; m < (schedACM_getReclassification(sched)?schedACM_getNbModCod(sched):(mc+1)); m++) {
      printf_debug(DEBUG_SCHED, "  MODCOD %d\n", m);
      for (q = 0; q < schedACM_getNbQoS(sched); q++) {
	printf_debug(DEBUG_SCHED, "    QoS %d (%d PDUs)\n", q, filePDU_length(schedACM_getInputQueue(sched, m, q)));
	 for (n = 1; n <= filePDU_length(schedACM_getInputQueue(sched, m, q)); n++) {
            id = filePDU_id_PDU_n(schedACM_getInputQueue(sched, m, q), n);
            taille = filePDU_size_PDU_n(schedACM_getInputQueue(sched, m, q), n);
            printf_debug(DEBUG_SCHED, "      [%d] : PDU %d (taille %d)\n", n, id, 
			 taille);
         }
      }
   }
}

/**
 * @brief Un affichage synthétique des files
 */
#define _MY_BUFF_SIZE_ 1024
void schedACM_printFilesSummary(struct schedACM_t * sched)
{
   int m, q;
   char buffer[_MY_BUFF_SIZE_];
   char bufferq[_MY_BUFF_SIZE_];

   printf_debug(DEBUG_SCHED, "-------------File status-------------\n");
   for (m = 0; m < schedACM_getNbModCod(sched); m++) {
      sprintf(buffer, " M%d ", m);
      for (q = 0; q < schedACM_getNbQoS(sched); q++) {
	sprintf(bufferq, " F%d[%d]", q, filePDU_length(schedACM_getInputQueue(sched, m, q)));
	 strcat(buffer, bufferq);
      }
      printf_debug(DEBUG_SCHED, "%s\n", buffer);
   }
   printf_debug(DEBUG_SCHED, "-------------------------------------\n");
}

/**********************************************************************************/
/*   Gestion des remplissages                                                     */
/**********************************************************************************/

/*
 * Remise Ã  zÃ©ro. On doit pouvoir faire plus efficace (tout n'est pas utilisÃ©)
 */
void remplissage_raz(t_remplissage * tr, int nbModCod, int nbQoS)
{
   int  m, q;

   tr->modcod = -1;
   tr->volumeTotal = 0;
   tr->interet = 0.0;
   tr->nbChoix = 1;
   tr->casTraite = 0;

   for (m = 0; m < nbModCod; m++) {
      for (q = 0; q < nbQoS; q++) {
         tr->nbrePaquets[m][q] = 0;
      }
   }
}

/*
 * Initialisation (crÃ©ation) d'une solution de remplissage
 */
void remplissage_init(t_remplissage * tr, int nbModCod, int nbQoS)
{
   int  m;

   //printf_debug(DEBUG_KS, "m/c = %d/%d\n", nbModCod, nbQoS);

   nbRemplissageAlloc++;

   assert(tr);

   tr->nbrePaquets = (int **)sim_malloc(nbModCod*sizeof(int *));
   assert(tr->nbrePaquets);

   for (m = 0; m < nbModCod; m++) {
      tr->nbrePaquets[m] = (int *)sim_malloc(nbQoS*sizeof(int));
      assert(tr->nbrePaquets[m]);
   }

   remplissage_raz(tr, nbModCod, nbQoS);

   //printf_debug(DEBUG_KS, "m/c = %d/%d done\n", nbModCod, nbQoS);
}


void remplissage_free(t_remplissage * tr, int nbModCod)
{
   int  m;
   nbRemplissageFree ++;
   for (m = 0; m < nbModCod; m++) {
      free( tr->nbrePaquets[m]);
   }
}


/**
 * @brief Nombre de paquets d'une file prévus dans un remplissage
 * @param tr pointeur sur le remplissage observé
 * @param m l'identifiant du MODCOD concerné
 * @param q L'identifiant de file de QoS concerné
 * @result Le nombre de paquets prévus dans cette file
 */
int remplissage_nbPackets(t_remplissage * tr, int m, int q)
{
   return tr->nbrePaquets[m][q];
}

/*
 * Initialisation (crÃ©ation) d'un tableau de solutions
 */
void tabRemplissage_init(t_remplissage * tr, int nbR, int nbModCod, int nbQoS)
{
   int r;

   for (r = 0; r < nbR; r++) {
      remplissage_init(tr + r, nbModCod, nbQoS);
   }
}

void tabRemplissage_raz(t_remplissage * tr, int nbR, int nbModCod, int nbQoS)
{
   int r;

   for (r = 0; r < nbR; r++) {
      remplissage_raz(&tr[r], nbModCod, nbQoS);
   }
}

/*
 * Copie d'une stratégie de remplissage dans une autre. La destination
 * doit être initialisée
 */
void remplissage_copy(t_remplissage * src, t_remplissage * dst, int nbModCod, int nbQoS)
{
   int m, q;

   //   printf_debug(DEBUG_SCHED, "IN\n");
   dst->modcod = src->modcod;
   dst->volumeTotal = src->volumeTotal;
   dst->interet = src->interet;
   dst->nbChoix = src->nbChoix;   // Attention au piège
   dst->casTraite = src->casTraite;
   for (m = 0; m < nbModCod; m++) {
     //      printf_debug(DEBUG_SCHED, "  m = %d\n", m);
      assert(src->nbrePaquets[m]);
      assert(dst->nbrePaquets[m]);
      for (q = 0; q < nbQoS; q++) {
	//         printf_debug(DEBUG_SCHED, "     q = %d\n", q);
         dst->nbrePaquets[m][q] = src->nbrePaquets[m][q];
      }
   }
   //   printf_debug(DEBUG_SCHED, "OUT\n");
}
/**
 * @brief Initialisation d'une séquence
 */
void sequence_init(t_sequence * seq, int lgMax, int nbModCod, int nbQoS)
{
   seq->longueur = lgMax;
   seq->remplissages = (t_remplissage *)malloc(lgMax * sizeof(t_remplissage));
   seq->positionActuelle = 0;
   seq->nextFrameToSend = 0;
   tabRemplissage_init(seq->remplissages, lgMax, nbModCod, nbQoS);
}

/**
 * @brief Remise à zéro d'une séquence
 */
void sequence_raz(t_sequence * seq, int lgMax, int nbModCod, int nbQoS)
{
   assert(seq->longueur == lgMax);
   seq->positionActuelle = 0;
   seq->nextFrameToSend = 0;
   tabRemplissage_raz(seq->remplissages, lgMax, nbModCod, nbQoS);

}


/**
 * @brief Copie d'une séquence
 * @param src Une séquence source (in)
 * @param dst Une séquence destination (out)
 * @param nbModCod Nombre de MODCODs des séquences
 * @param nbQoS Nombre de files de QoS par MODCODs des séquences
 * La séquence source est copiée dans la destination, jusqu'à la
 * position actuelle (exclue). Attention, la séquence dst doit avoir
 * été intégralement allouée !
 */
void sequence_copy(t_sequence * src, t_sequence * dst, int nbModCod, int nbQoS)
{
   int r;

   //   printf_debug(DEBUG_SCHED, "IN\n");
   dst->longueur = src->longueur;
   dst->positionActuelle = src->positionActuelle;
   /*
   printf_debug(DEBUG_SCHED, "l%d, p%d (m%d, q%d)\n",
		dst->longueur,
		dst->positionActuelle,
		nbModCod,
		nbQoS);
   */
   for (r = 0 ; r < dst->positionActuelle ; r++) {
     //      printf_debug(DEBUG_SCHED, "copie de %d\n", r);
      remplissage_copy(&(src->remplissages[r]), &(dst->remplissages[r]), nbModCod, nbQoS);
   }   
   //   printf_debug(DEBUG_SCHED, "OUT\n");
}

/**
 * @brief Nombre de paquets d'une file prévus dans une séquence
 * @param tr pointeur sur le remplissage observé
 * @param m L'identifiant du MODCOD concerné
 * @param q L'identifiant de file de QoS concerné
 * @result Le nombre de paquets prévus dans cette file
 * La positionActuelle n'est pas comptabilisée
 */
int sequence_nbPackets(t_sequence * seq, int m, int q)
{
   int result = 0;
   int r;
   
   for (r=0 ; r < seq->positionActuelle ; r++) {
      result += seq->remplissages[r].nbrePaquets[m][q];
   };

   return result ;
}

/**
 * @brief Les files sont-elles (virtuellement) vides ?
 * @param seq Une séquences de trames calculée par un ordonnanceur
 * @param sched Un ordonannceur
 * @result Suite à une telle séquence, les files sont-elles vides ?
 * Attention, la trame définie par la position actuelle n'est pas
 * prise en compte (elle est en cours de calcul, donc risque d'être
 * fausse). 
 */
int sequence_filesVides(t_sequence * seq, struct schedACM_t * sched)
{
   int m, q;
   int nb;
   int r;

   //   printf_debug(DEBUG_SCHED, "IN\n");
   // Pour chaque MODCOD, ...
   for (m = 0; m < sched->nbModCod; m++) {
      // ... et pour chaque file ...
      for (q = 0; q < sched->nbQoS; q++) {
         // ... il reste nb paquets réellement présents ...
         nb = filePDU_length(sched->files[m][q]);
	 //	 printf_debug(DEBUG_SCHED, "m = %d, q = %d -> %d\n", m, q, nb);
         r = 0;
         // ... dont on enlève ceux prévus dans les BBFRAMES
         while ((nb>0) && (r<seq->positionActuelle)) {
	   //            printf_debug(DEBUG_SCHED, "    - %d\n", seq->remplissages[r].nbrePaquets[m][q]);;
	    nb -= seq->remplissages[r].nbrePaquets[m][q];
	    r++;
	 }
         // S'il en reste un, c'est pas vide !
         if (nb>0){
	   //            printf_debug(DEBUG_SCHED, "OUT 0\n");
            return 0;
	 }
      }
   }

   // Si on en est là, c'est que toutes les files sont vides.
   //   printf_debug(DEBUG_SCHED, "OUT 1\n");
   return 1;
}

/**
 * @brief Détermination de l'intérêt cumulé sur une séquence
 * @param seq Une séquence de trames envisagée par un ordonnanceur
 * @result L'intérêt cumulé sur toutes les trames avant la courante
 * Attention, cette fonction cumule l'intérêt sur toutes les BBFRAMES
 * qui ont été calculées. Elle ne compte donc pas cemui de la
 * positionActuelle. L'intérêt doit avoir été calculé sur chacun des
 * remplissages envisagés.
 */
double sequence_getInteret(t_sequence * seq)
{
   double result = 0.0;

   int r;

   for (r= 0 ; r<seq->positionActuelle; r++) {
      result += seq->remplissages[r].interet;
   }

   return result;
}

/**
 * @brief Taille cumulée sur une séquence
 * @param seq La séquence calculée par un ordonnanceur
 * @param sched La structure d'ordonnanceur
 * @result La somme des tailles de tous les paquets prévus dans la
 * séquence, hors position actuelle
 */ 
int sequence_getTotalSize(t_sequence * seq, struct schedACM_t * sched)
{
   int result = 0;
   int m, q;
   //   printf_debug(DEBUG_SCHED, " 0\n");
   // Pour chaque MODCOD, ...
   for (m = 0; m < sched->nbModCod; m++) {
     // 	printf_debug(DEBUG_SCHED, "  (m%d)\n", m);
      // ... et pour chaque file ...
      for (q = 0; q < sched->nbQoS; q++) {
	// 	printf_debug(DEBUG_SCHED, "   [q%d]\n", q);
	// 	printf_debug(DEBUG_SCHED, "     + (%d, %d) (nb %d)\n", m, q, sequence_nbPackets(seq, m, q));
         result += filePDU_size_n_PDU(sched->files[m][q], sequence_nbPackets(seq, m, q));
	 // 	printf_debug(DEBUG_SCHED, "     -> %d\n", result);
      }
   }
   //   printf_debug(DEBUG_SCHED, " = %d\n", result);

   return result;
}

/**
 * @brief Taille cumulée sur une séquence par une file
 * @param seq La séquence calculée par un ordonnanceur
 * @param sched La structure d'ordonnanceur
 * @param m Le MODCOD concerné
 * @param q La file concernée
 * @result La somme des tailles de tous les paquets prévus dans la
 * séquence, hors position actuelle, sur la file donnée
 */ 
int sequence_getFileSize(t_sequence * seq, struct schedACM_t * sched, int m, int q)
{
   return filePDU_size_n_PDU(sched->files[m][q], sequence_nbPackets(seq, m, q));
}

/**
 * @brief Durée d'une séquence
 * @param sched L'ordonnanceur concerné
 * @param sequence La séquence à mesurer
 * Détermination de la durée cumulée de toutes les trames d'une
 * séquence hors positionActuelle.
 */
double sequence_getDuration(struct schedACM_t * sched, t_sequence * seq)
{
   double result = 0.0;
   int r;

   for (r= 0 ; r<seq->positionActuelle; r++) {
      result += DVBS2ll_bbframeTransmissionTime(schedACM_getACMLink(sched), seq->remplissages[r].modcod);
   }

   return result;
}

/**
 * @brief Un affichage synthétique d'une séquence
 */
#define _MY_BUFF_SIZE_ 1024
void schedACM_printSequenceSummary(struct schedACM_t * sched, t_sequence * sequence)
{
   int m, q;
   char buffer[_MY_BUFF_SIZE_];
   char bufferq[_MY_BUFF_SIZE_];
   int r;

   printf_debug(DEBUG_ALWAYS, "-------------Sequence (lg%d/%d)-------------\n",
		sequence->positionActuelle,
		sequence->longueur);

   for (r = 0; r <= ((sequence->positionActuelle<sequence->longueur-1)?sequence->positionActuelle:sequence->longueur-1); r++){
      if ((r<sequence->positionActuelle) || (sequence->remplissages[r].modcod >= 0)) {
         sprintf(buffer, "%cTr-%d (M%d) : ",
		 (r==sequence->positionActuelle)?'#':' ',
		 r, sequence->remplissages[r].modcod);
      for (m = 0; m < schedACM_getNbModCod(sched); m++) {
         for (q = 0; q < schedACM_getNbQoS(sched); q++) {   
	    if (sequence->remplissages[r].nbrePaquets[m][q]) { 
               sprintf(bufferq, " M%dQ%d[%d/%d]", m, q,
                    sequence->remplissages[r].nbrePaquets[m][q],
		    filePDU_length(schedACM_getInputQueue(sched, m, q)));
	    } else {
               sprintf(bufferq, " .");
	    }
	    strcat(buffer, bufferq);
	    bufferq[0]=0;
         }
      }
     }
      printf_debug(DEBUG_ALWAYS, "%s\n", buffer);
      buffer[0]=0;
   }
   printf_debug(DEBUG_ALWAYS, "-----------------------------------\n");
}

/**********************************************************************************/
/*   FIN de la Gestion des remplissages                                           */
/**********************************************************************************/
