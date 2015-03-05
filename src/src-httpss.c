/**
 * @file src-httpss.c
 * @brief HTTP source model (slow start only)
 *
 * This module implements a HTTP source model as described
 * in [1]. 
 *
 * [1] "cdma2000 Evaluation Methodology" 3GPP2
 * C. R1002-0. Version1.0. December 10, 2004
 */
#include <src-httpss.h>
#include <src-tcpss.h>
#include <event.h>
#include <motsim.h>
#include <file_pdu.h>
#include <random-generator.h>

/**
 * @brief Definition of a  HTTP source
 * @param Sm Size of the main object in a page randomGenerator
 * @param Se Size of an embedded object in a page randomGenerator
 * @param Nd Number of embedded objects in a page randomGenerator
 * @param Dpc Reading time randomGenerator
 * @param Tp Parsing time for the main page randomGenerator
 * @param MTU Le MTU de la connexion TCP
 * @param nbTCP nombre de connexion TCP selon le mode
 * @param nbPage nombre de page visité
 * @param version vaut 1 si 1.1, vaut 0 si 1.0
 * @param RTTmd is the Round Trip Time minus transmission time on the access link
 * @param initialWindow is the initial value of cwnd
 * @param destination is a pointer to the destination entity
 * @param destProcessPDU is the PDU processing function of the destination
 * @param srcTCP sources TCP utilisés par l'objet HTTP
 * @param nbTCPTermine le nombre de connecxions TCP ayant envoyer tout leurs paquets

 */
struct srcHTTPSS_t {
	struct randomGenerator_t * Sm; // Size of the main object in a page
	struct randomGenerator_t * Se; // Size of an embedded object in a page
	struct randomGenerator_t * Nd; // Number of embedded objects in a page
	struct randomGenerator_t * Dpc; // Reading time
	struct randomGenerator_t * Tp; // Parsing time for the main page
	int MTU;
	int nbTCP; // nombre de connexion TCP selon le mode
	int nbPage; // nombre de page visité
	int version; // vaut 1 si 1.1, vaut 0 si 1.0
	void * destination; // Objet auquel sont destinées les PDUs
	processPDU_t destProcessPDU; // Fonction notifiant la destination de la présence de PDUs
	struct srcTCPSS_t * srcTCP[4]; //sources TCP utilisés par l'objet HTTP
	int nbTCPTermine; //le nombre de connecxions TCP ayant envoyer tout leurs paquets
	double RTTmd; //is the Round Trip Time minus transmission time on the access link
	int initialWindow; //is the initial value of cwnd
};

struct srcTCPSS_t {
   int                windowSize ;
   int                MSS;
   double             RTT;

   int                backlog;   //!< the number of unsent bytes
   int                nbSentSegments;  //!< Number of segments actually sent
   struct filePDU_t * outputQueue; //!< An internal queue where
				   //!segments are queued waiting to
				   //!be actually sent
   void        * destination;    //!< L'objet auquel sont destinées les PDUs
   processPDU_t  destProcessPDU; //!< La fonction permettant de
				 //!signaler à la destination la
				 //!présence de la PDU
  struct eventFile_t * EOTEventList; //!< List of events to run at the
				  //!end of transmission
};


/**
 * @brief Creation/initialization of a HTTP source
 * @param Sm randomGenerator pour la taille de la page principale
 * @param Se randomGenerator pour la taille des objets embarqués
 * @param Nd randomGenerator pour le nombre d'objets embarqués
 * @param Dpc randomGenerator pour le temps de lecture
 * @param Tp randomGenerator pour le temps de parsing
 * @param MTU is the maximum transmission unit of the link
 * @param nbTCP nombre de connexions TCP (1 pour HTTP 1.1)
 * @param version : 0 pour hhtp 1.0 (burst-mode) et 1 pour http 1.1 (persistent
 * mode
 */
struct srcHTTPSS_t * srcHTTPSS_init(struct randomGenerator_t * Sm,
					struct randomGenerator_t * Se,
					struct randomGenerator_t * Nd,
					struct randomGenerator_t * Dpc,
					struct randomGenerator_t * Tp,
					int MTU, int nbTCP, int version,
					void * destination,
					processPDU_t destProcessPDU,
					double RTTmd,
					int initialWindow){	
	struct srcHTTPSS_t * result = 
		(struct srcHTTPSS_t *) sim_malloc(sizeof(struct srcHTTPSS_t ));

	result -> Sm = Sm;
	result -> Se = Se;
	result -> Nd = Nd;
	result -> Dpc = Dpc;
	result -> Tp = Tp;							
	result -> MTU = MTU; 
	result -> nbTCP = nbTCP; 
	result -> nbPage = 0;
	result -> version = version;
	result -> destination = destination;
	result -> destProcessPDU = destProcessPDU;
	result -> RTTmd = RTTmd;
	result -> initialWindow = initialWindow;

	return result;
}

/**
 * @brief Creation of HTTP source, with default values for distribution given by http://www.3gpp2.org/Public_html/specs/C.R1002-0_v1.0_041221.pdf
 * @param MTU maximum transimtion unit of the link
 * @param nbTCP nombre de connexion TCP (1 pour HTTP 1.1)
 * @param version : 0 pour hhtp 1.0 (burst-mode) et 1 pour http 1.1 (persistent
 * mode
 */
struct srcHTTPSS_t * srcHTTPSS_init_default(int MTU, int nbTCP, int version, double RTTmd, int initialWindow, void * destination, processPDU_t destProcessPDU) {
	
  	struct srcHTTPSS_t * result = 
		(struct srcHTTPSS_t *) sim_malloc(sizeof(struct srcHTTPSS_t ));
  	result -> Sm = randomGenerator_createDoubleRangeTruncLogNorm(8.35, 1.37, 2000000.0);
  	result -> Se = randomGenerator_createDoubleRangeTruncLogNorm(6.17, 2.36, 2000000.0);
	result -> Nd = randomGenerator_createDoubleRangeTruncPareto(1.1, 2.0, 55.0);
	result -> Dpc = randomGenerator_createDoubleExp(0.033);
	result -> Tp = randomGenerator_createDoubleExp(7.69);
	result -> MTU = MTU;
	result -> nbTCP = nbTCP;
	result -> nbPage = 0;
	result -> version = version;
	result -> destination = destination;
	result -> destProcessPDU = destProcessPDU;
	result -> RTTmd = RTTmd;
	result -> initialWindow = initialWindow;

	return result;
}
	 
/**
 * @brief Define another Sm of a HTTP source
 * @param src HTTP source
 * @param Sm randomGenerator Size of the main object in a page
*/
void srcHTTPSS_setSm(struct srcHTTPSS_t * src, struct randomGenerator_t * Sm) {src->Sm = Sm;}

/**
 * @brief Define another Se of a HTTP source
 * @param src HTTP source
 * @param Se randomGenerator Size of an embedded object in a page
*/
void srcHTTPSS_setSe(struct srcHTTPSS_t * src, struct randomGenerator_t * Se) {src->Se = Se;}

/**
 * @brief Define another Nd of a HTTP source
 * @param src HTTP source
 * @param Nd randomGenerator Number of embedded objects in a page
*/
void srcHTTPSS_setNd(struct srcHTTPSS_t * src, struct randomGenerator_t * Nd) {src->Nd = Nd;}

/**
 * @brief Define another Dpc of a HTTP source
 * @param src HTTP source
 * @param Dpc randomGenerator  Reading time
*/
void srcHTTPSS_setDpc(struct srcHTTPSS_t * src, struct randomGenerator_t * Dpc) {src->Dpc = Dpc;}

/**
 * @brief Define another Tp of a HTTP source
 * @param src HTTP source
 * @param Tp randomGenerator Parsing time for the main page randomGenerator
*/
void srcHTTPSS_setTp(struct srcHTTPSS_t * src, struct randomGenerator_t * Tp) {src->Tp = Tp;}

/**
 * @brief Define another MTU of a HTTP source
 * @param src HTTP source
 * @param MTU int Le MTU de la connexion TCP
*/
void srcHTTPSS_setMTU(struct srcHTTPSS_t * src, int MTU) {src->MTU = MTU;}

/**
 * @brief Define another version and number of TCP sources
 * matching with the version of a HTTP source
 * @param src HTTP source
 * @param version int version du protocole HTTP
 * @param nbTCP nombres de connexions TCP de la version HTTP considérée
*/
void srcHTTPSS_setVersion(struct srcHTTPSS_t * src, int version, int nbTCP ) {src->version = version;src->nbTCP = nbTCP;}


/**
 * @brief Création d'une structure afin de passer à event_add un seul argument.
 * arg structure contenant la source HTTP, les infos pour une sources TCP et le nombre
 * de connections TCP terminés lors d'un envoi sur plusieurs connections.
 * @param src source HTTP, we have src-> MTU as maximum transmission unit of the link
 * @param RTTmd is the Round Trip Time minus transmission time on the access link
 * @param initialWindow is the initial value of cwnd
 * @param destination is a pointer to the destination entity
 * @param destProcessPDU is the PDU processing function of the destination
 * @param srcTCP sources TCP utilisés par l'objet HTTP
 * @param nbTCPTermine le nombre de connecxions TCP ayant envoyer tout leurs paquets
*/
/*typedef struct fonctionsHttpssArguments fonctionsHttpssArguments;
struct fonctionsHttpssArguments
{
	struct srcHTTPSS_t * src;
};*/


void srcHTTPSS_sessionStart(void * arg)
{
printf("Entre dans session start\n");

	struct srcHTTPSS_t * src = (struct srcHTTPSS_t *) arg;

printf("Init connexion TCP1\n");
	/*Initialiser les connections TCP*/
	src->nbTCPTermine = 0;
printf("Init connexion TCP2\n");
	// Si pas première lecture on doit free les connexions précédentes.
	//On détruit les sources TCP
	if (src->nbPage > 0) {
printf("On free les src TCP (commenté)\n");
		srcTCPss_free(src->srcTCP[0]);
	}
	// Création de la connexion pour la page principale
	src->srcTCP[0] = srcTCPss_create(src->MTU, src->RTTmd,
						src->initialWindow, src->destination,
						src->destProcessPDU);
printf("Genere taille page principale\n");
	/*On envoie la page principale*/
	double sm = 146000;//randomGenerator_getNextDouble(src->Sm);
printf("Envoi page principale, taille : %lf\n",sm);
	printf("\n %f <- valeur de sm \n", sm);
	srcTCPss_sendFile(src->srcTCP[0], sm);
printf("Programmation de l'évènement fin de transmition via TCP_addEOTevent\n");

	/*On déclenche l'événement fin de transmission de la page principale*/
printf("%d : nbPagesLu\n\n", src->nbPage);
	srcTCPss_addEOTEvent(src->srcTCP[0], event_create(srcHTTPSS_EOTMainObject, src, 0));
printf("Evènement programmé\n");

}

/*Lancement des objets embarqués*/
void srcHTTPSS_EOTMainObject (void * arg) {
printf("Entrée dans la fct EOTMainObj\n");

	struct srcHTTPSS_t * src = (struct srcHTTPSS_t *) arg;
	/*On détruit la source TCP*/
	//	srcTCPss_free(src->srcTCP[0]);
printf("Génère temps de parsing \n");
	/*On peut programmer le chargement des objets embarqués*/
	int tempsParsing = 5;//randomGenerator_getNextDouble(src->Tp);
printf("Temps de parsing : %d\n",tempsParsing);
printf("Programmation de l'event envoie des EbOb");
	event_add(srcHTTPSS_sendEmbeddedObjects, src, motSim_getCurrentTime() + tempsParsing);
printf("Event programmé\n\n");


}


/*Envoyer les objets embarqués*/
void srcHTTPSS_sendEmbeddedObjects(void * arg)
{
printf("Entrée dans l'envoie des EbOb\n");

	struct srcHTTPSS_t * src = (struct srcHTTPSS_t *) arg;
printf("Création des connexions TCP (de la)\n");
	int i;
	/*On crée de nouvelles connections TCP*/
	if (src->version == 0) {
		for ( i=0; i < src->nbTCP; i++) 
		{
printf("Création de la %d-ème connexion\n",i);
printf("Free de la source TCP \n");
			if (src->nbPage > 0) {
				srcTCPss_free(src->srcTCP[i]);
			}
			src->srcTCP[i] = srcTCPss_create(src->MTU, src->RTTmd, src->initialWindow, src->destination, src->destProcessPDU);
			printf("Destination : %p\n",src->destination);
			printf("Taille de la fenêtre de la source : %d\n",src->srcTCP[i%(src->nbTCP)]->windowSize);
		}
printf("Connexion crée\n");
	}

printf("Fin de création des connexions\n");

	/*Calculer le nombre d'objets embarqués*/
	int Nd = 3;//randomGenerator_TruncParetoGetNext(src->Nd);
	printf(" Tirage du nombre d'objets : %d Nd embedded objects\n",Nd);
	for (i = 0; i<Nd; i++) 
	{
		double s = 146000.0;//randomGenerator_getNextDouble(src->Se);
		int sizeEbOb = (int) s;
printf("Send file, taille de l'objet : %d\n", sizeEbOb);
		srcTCPss_sendFile(src->srcTCP[i%(src->nbTCP)], s);//randomGenerator_getNextDouble(src->Se)) ;
	}
	int fin;

printf("Calcul fin de boucle pour ne pas vérifier la terminaison de l'envoi sur une connexion ou aucun envoi n'a été fait, ERREUR  corrigée : le nombre de connexion a terminé ne change pas, on risque de ne vérifier qu'une \n");
	if (src->nbTCP > Nd) {
		fin = Nd;
		// Correction, on termine les sources non vérifiées
		src->nbTCPTermine = src->nbTCPTermine + src->nbTCP - Nd;
		printf("%d : nbTCPTermine, correction\n",src->nbTCPTermine);
	} else {
		fin = src->nbTCP;
	}
printf("%d : FIN\n",fin);
// COMMENTE POUR MODIF
	/*On vérifie qu'on a envoyé les objets embarqués*/
	for (i = 0; i<fin; i++) {
printf("Dans la boucle de vérification de fin d'envoi\n");
	/*On déclenche l'événement fin de transmission des Objets Embarquées principale*/
		printf("%d-ème event\n",i);
		srcTCPss_addEOTEvent(src->srcTCP[i], event_create(srcHTTPSS_EOTEmbeddedObjects, src, 0));
		printf("%d : nbTCPTermine\n",src->nbTCPTermine);
	}
printf("\n");
}

/*Les objets Embarqués ont été envoyés*/
void srcHTTPSS_EOTEmbeddedObjects(void * arg) {
printf("Entrée dans la fonction de vérif de fin d'envoi\n");
	struct srcHTTPSS_t * src = (struct srcHTTPSS_t *) arg;
	// Une source a terminé, on incrémente
	src->nbTCPTermine++;
printf("Vérif de si tout est terminé\n");
	if (src->nbTCPTermine == src->nbTCP) {
		//On a lu une page entière
		src->nbPage++;
		src->nbTCPTermine = 0;
printf("Tout est terminé, pages lues : %d\n",src->nbPage);
		//On détruit les sources TCP
//		int i;
//		for (i=0; i < src->nbTCP; i++)
//		{
//printf("On free les src TCP (commenté)\n");
//			srcTCPss_free(src->srcTCP[i]);
//		}
		/*On prépare le chargement de la nouvelle page*/
		int tempsReading = 200;//randomGenerator_getNextDouble(src->Dpc);
printf("Tirage du temps de lecture : %d\n", tempsReading);
		// Si on a pas finit la session on programme la nouvelle page
		if (src->nbPage < 2) {
printf("Programmation de l'event lire une nouvelle page\n");
			event_add(srcHTTPSS_sessionStart, src, motSim_getCurrentTime()+ tempsReading);
		printf("Programmé\n\n");
		}

	} else {
printf("Tout n'est pas terminé"); //on ajoute un event pour que le simulateur ne s'arrête pas\n);
printf("%d : nbTCPTermine = %d\n\n",src->nbTCPTermine);
// On ajoute un évènement pour que le simulateur ne s'arrête pas
// BRICOLAGE
//	event_create(wait, src, motSim_getCurrentTime()+1);
	}
}


