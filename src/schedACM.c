#include <math.h>      // exp, pow, ...

#include <schedACM.h>


// A virer

unsigned long nbRemplissageAlloc = 0 ;
unsigned long nbRemplissageFree = 0;

/*
 * A chaque file est associée une QoS
 */
struct fileQoS {
   struct filePDU_t * file;
   t_qosMgt           qos;
};

/*
 * Caractérisation générale d'un ordonnanceur sur ACM
 */
struct schedACM_t {
   int nbQoS;
   int nbModCod;

   // Les sources sont forcément des files, mais il serait 
   // bon de les utiliser de façon cohérente avec le modèle I/O
   struct filePDU_t *** files;  // Les files d'attentes
   t_qosMgt          ** qos;
   int                  declassement;

   // La destination est forcement un lien DVBS2, mais il serait 
   // bon de l'utiliser de façon cohérente avec le modèle I/O
   // De plus, on pourra généraliser sans trop de difficultés (?)
   // à un ACM
   struct DVBS2ll_t * dvbs2ll; // Le lien sur lequel on transmet

   // pqFromMQinMC[m][q][mc] est une probe qui compte la taille et le nombre de
   // paquets de la file [m][q] qui sont transmis par le MODCOD mc.
   struct probe_t **** pqFromMQinMC;

   int paquetsEnAttente; // Ai-je au moins un pq en attente ?
   t_remplissage solutionChoisie;

   int nbSol; // Le nombre de solutions envisagées
   struct probe_t * nbSolProbe;

   void * private;
   struct schedACM_func_t * func;
};
/*
 * Création d'un scheduler avec sa "destination". Cette dernière doit
 * être de type struct DVBS2ll_t  et avoir déjà été complêtement
 * construite (tous les MODCODS créés).
 * Le nombre de files de QoS différentes par MODCOD est également
 * passé en paramètre.
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

   // Allocation des tableaux de files, qos et paramètres
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
	 result->qos[i][j].debit   = 0.0; // WARNING bof
         result->qos[i][j].bwProbe = NULL;
      }
   }

   // Allocation des tableaux de probes pour la répartition des paquets
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

   result->nbSol = 0;
   result->nbSolProbe = NULL;

   result->declassement = declOK;
   result->private = NULL;

   result->func = func;
   printf_debug(DEBUG_ACM, "%p created (link : %p)\n", result, result->dvbs2ll);

   return result;

}

/*
 * Y a-t-il des paquets en attente ? Le résultat est booléen
 */
int schedACM_getPacketsWaiting(struct schedACM_t * sched)
{
   return sched->paquetsEnAttente;
}

/*
 * Si les fonctions getPDU et processPDU sont redéfinies, la présence
 * de paquets en attente n'est plus mise à jour. Il faut donc
 * l'assurer par des appels à la fonction suivante.
 */
void schedACM_setPacketsWaiting(struct schedACM_t * sched, int b)
{
   sched->paquetsEnAttente = b;
}

/*
 * Modification des données privées.
 */
void schedACM_setPrivate(struct schedACM_t * sched, void * private)
{
   sched->private = private;
}

/*
 * Obtention des données privées
 */
void * schedACM_getPrivate(struct schedACM_t * sched)
{
   return sched->private;
}

/*
 * Ajout d'une sonde pour compter les paquets d'une file (m, q) émis par un
 * MODCOD mc (mc peut être < m en cas de reclassement)
 */
void schedACM_setPqFromMQinMC(struct schedACM_t * sched, int m, int q, int mc, struct probe_t * pr)
{
   sched->pqFromMQinMC[m][q][mc] = pr;
}

/*
 * Attribution des files d'attente d'entrée pour un MODCOD donné dans
 * le paramètre mc. Le paramètre files est un tableau de pointeurs sur
 * des files de PDU. Il doit en contenir au moins nbQoS. Les nbQoS
 * premières seront utilisées ici.
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
 * Affectation d'une sonde permettant de suivre le débit estimé par
 * l'algorithme pour chaque file
 */
void schedACM_setThoughputProbe(struct schedACM_t * sched, int m, int q, struct probe_t * bwProbe)
{
   sched->qos[m][q].bwProbe = bwProbe;
}

/*
 * Attribution du type de QoS d'une file. La file est identififée par
 * (mc, qos), le type de QoS voulue est passée en paramètre, ainsi
 * qu'un éventuel paramètre de pondération.
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
inline int nbModCod(struct schedACM_t * sched)
{
   return sched->nbModCod;
}

/*
 * Consultation du nombre de QoS par MODCOD
 */
inline int nbQoS(struct schedACM_t * sched)
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
 * Peut-on faire du "déclassement" ?
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

/*
 * Obtention d'un pointeur sur la solution choisie
 */
t_remplissage * schedACM_getSolution(struct schedACM_t * sched)
{
   return &sched->solutionChoisie;
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
/*
 * Combien de solutions testées ?
 */
int schedACM_getNbSolutions(struct schedACM_t * sched)
{
  return sched->nbSol;
};

/********************************************************************************/
/*   La dérivée de la fonction d'utilité                                        */
/********************************************************************************/

/*
 * Calcul de la valeur en x de la derivee d'une fonction d'utilité
 * Le paramètre dvbs2ll est ici nécessaire pour certaines fonctions
 * Il faudra envisager de mettre ces info (le débit du lien en gros
 * pour le moment) dans la structure t_qosMgt, ou pas !)
 */
double utiliteDerivee(t_qosMgt * qos, double x, struct DVBS2ll_t * dvbs2ll)
{
   double result = 0.0;
   double debitBinaire;

   switch (qos->typeQoS) {
      case kseQoS_log :
         result = qos->beta/x;   // 2012-02-07 : remplacement de 1.0 au numérateur
      break;

      case kseQoS_lin :             // Utilité linéaire
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
/*   Les fonctions liées au modèle d'entrée/sortie                              */
/********************************************************************************/

/*
 * L'ordonnanceur lui-même
 */
void schedACM_schedule(struct schedACM_t * sched)
{
   remplissage_raz(&(sched->solutionChoisie), nbModCod(sched), nbQoS(sched));

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


/*
 * Construction d'une BBFRAME avec les paquets en attente dans les
 * files s'il y en a suffisemment. Sinon, un pointeur NULL est retourné.
 */
struct PDU_t * schedACM_buildBBFRAMEGeneric(struct schedACM_t * sched)
{
   int q, m, p, vol, s;
   double alphaMaaike;
   struct PDU_t * pdu = NULL;

   printf_debug(DEBUG_ACM, "Packets waiting ? %s \n", sched->paquetsEnAttente?"Oui":"non");

   if (sched->paquetsEnAttente) {
      // A priori, on va vider les paquets en attente
      sched->paquetsEnAttente = 0;

      // Recherche de l'ordonnancement par application de l'algo
      printf_debug(DEBUG_ACM, "On schedule ...\n");
      schedACM_schedule(sched);
      printf_debug(DEBUG_ACM, "Volume propose : %d\n", sched->solutionChoisie.volumeTotal);


      // Si on trouve au moins un paquet à envoyer
      if (sched->solutionChoisie.volumeTotal) {
         vol = 0;
         // On extrait les paquets concernés depuis les files d'entré
         for (m = 0; m < nbModCod(sched); m++) {
            for (q = 0; q < nbQoS(sched); q++) {
               s = 0;
               for (p = 0 ; p < sched->solutionChoisie.nbrePaquets[m][q]; p++){
                  pdu = filePDU_extract(schedACM_getInputQueue(sched, m, q));
                  s += PDU_size(pdu);
	          printf_debug(DEBUG_ACM, "Sortie du paquet %d de taille %d de la file (%d, %d)\n", PDU_id(pdu), PDU_size(pdu), m, q);
		  probe_sample(schedACM_getPqFromMQinMC(sched, m, q, sched->solutionChoisie.modcod), PDU_size(pdu));
                  PDU_free(pdu);
               }
	       vol += s;
               // Mise à jour des débits (utilisés pour le calcul du gain)
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
         assert(sched->solutionChoisie.modcod < nbModCod(sched));

         pdu = PDU_create(vol, (void *)sched->solutionChoisie.modcod);
      }
   }

   return pdu;
}


/*
 * Construction d'une BBFRAME avec les paquets en attente dans les
 * files s'il y en a suffisemment. Sinon, un pointeur NULL est retourné.
 */
struct PDU_t * schedACM_buildBBFRAME(struct schedACM_t * sched)
{
   if (sched->func && sched->func->buildBBFRAME) {
      printf_debug(DEBUG_ACM, "calling dedicated facility ...\n");
      return sched->func->buildBBFRAME(sched->private);
   } else {
      printf_debug(DEBUG_ACM, "calling generic facility ...\n");
      return schedACM_buildBBFRAMEGeneric(sched);
   }
}

/*
 * Fonction invoquée lors de la disponibilité d'un paquet dans une des
 * files.
 * En fait, ici on ne fait rien. L'activitée sera dictée par la
 * disponibilité du support. C'est uniquement sur des événements
 * de l'aval que l'on agit, ...
 */
void schedACM_processPDUGeneric(struct schedACM_t * sched,
                                getPDU_t getPDU, void * source)
{
   struct PDU_t * pdu;

   printf_debug(DEBUG_ACM, "Un paquet dispo pour %p\n", sched);
      sched->paquetsEnAttente = 1;    //WARNING, pourquoi uniquement dans ce cas !?

   // Si par hasard le support est dispo, il faut prendre l'initiative
   if (DVBS2ll_available(schedACM_getACMLink(sched))) {
      printf_debug(DEBUG_ACM, "Support libre\n");
      pdu = schedACM_buildBBFRAME(sched);
      // WARNING TBD remplacer par le modèle classique (destination, ...)
      DVBS2ll_sendPDU(schedACM_getACMLink(sched), pdu);
   } else { // Je me le note pour être prêt lorsque le support sera dispo
      sched->paquetsEnAttente = 1;    //WARNING, pourquoi uniquement dans ce cas !?
      printf_debug(DEBUG_ACM, "Support occupe\n");
   }
}

/*
 * Fonction à invoquer lorsque le support est libre afin de solliciter
 * la construction d'une nouvelle BBFRAME
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
 * Fonction à invoquer lorsque le support est libre afin de solliciter
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
 * Fonction invoquée lors de la disponibilité d'un paquet dans une des
 * files.
 * En fait, ici on ne fait rien. L'activitée sera dictée par la
 * disponibilité du support. C'est uniquement sur des événements
 * de l'aval que l'on agit, ...
 */
void schedACM_processPDU(struct schedACM_t * sched,
                         getPDU_t getPDU, void * source)
{
   if (sched->func && sched->func->processPDU) {
      printf_debug(DEBUG_ACM, "calling dedicated facility ...\n");
      sched->func->processPDU(sched->private, getPDU, source);
   } else {
      printf_debug(DEBUG_ACM, "calling generic facility ...\n");
      schedACM_processPDUGeneric(sched, getPDU, source);
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

   printf_debug(DEBUG_ALWAYS, "Etat des files considerees: \n");
   for (m = mc; m < (schedACM_getReclassification(sched)?nbModCod(sched):(mc+1)); m++) {
      printf_debug(DEBUG_ALWAYS, "  MODCOD %d\n", m);
      for (q = 0; q < nbQoS(sched); q++) {
	printf_debug(DEBUG_ALWAYS, "    QoS %d (%d PDUs)\n", q, filePDU_length(schedACM_getInputQueue(sched, m, q)));
	 for (n = 1; n <= filePDU_length(schedACM_getInputQueue(sched, m, q)); n++) {
            id = filePDU_id_PDU_n(schedACM_getInputQueue(sched, m, q), n);
            taille = filePDU_size_PDU_n(schedACM_getInputQueue(sched, m, q), n);
            printf_debug(DEBUG_ALWAYS, "      [%d] : PDU %d (taille %d, gain %7.2f)\n", n, id, 
			 taille, 
			 gainUtilite(schedACM_getQoS(sched, m, q), taille, mc, schedACM_getACMLink(sched)));
         }
      }
   }
}



/**********************************************************************************/
/*   Gestion des remplissages                                                     */
/**********************************************************************************/

/*
 * Remise à zéro. On doit pouvoir faire plus efficace (tout n'est pas utilisé)
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
 * Initialisation (création) d'une solution de remplissage
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

/*
 * Initialisation (création) d'un tableau de solutions
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

   dst->modcod = src->modcod;
   dst->volumeTotal = src->volumeTotal;
   dst->interet = src->interet;
   dst->nbChoix = src->nbChoix;   // Attention au piège
   dst->casTraite = src->casTraite;
   for (m = 0; m < nbModCod; m++) {
      assert(src->nbrePaquets[m]);
      assert(dst->nbrePaquets[m]);
      for (q = 0; q < nbQoS; q++) {
         dst->nbrePaquets[m][q] = src->nbrePaquets[m][q];
      }
   }
}

/**********************************************************************************/
/*   FIN de la Gestion des remplissages                                           */
/**********************************************************************************/
