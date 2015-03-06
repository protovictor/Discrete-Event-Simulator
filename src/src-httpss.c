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
	int nbPageMax; // Nombre de page max à lire
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
					int initialWindow,
					int nbPageMax){	
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
	result -> nbPageMax = nbPageMax;
	

	return result;
}

/**
 * @brief Creation of HTTP source, with default values for distribution given by http://www.3gpp2.org/Public_html/specs/C.R1002-0_v1.0_041221.pdf
 * @param MTU maximum transimtion unit of the link
 * @param nbTCP nombre de connexion TCP (1 pour HTTP 1.1)
 * @param version : 0 pour hhtp 1.0 (burst-mode) et 1 pour http 1.1 (persistent
 * mode
 */
struct srcHTTPSS_t * srcHTTPSS_init_default(int MTU, int nbTCP, int version, double RTTmd, int initialWindow, void * destination, processPDU_t destProcessPDU, int nbPageMax) {
	
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
	result -> nbPageMax = nbPageMax;

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


void srcHTTPSS_sessionStart(void * arg)
{
	printf_debug(DEBUG_SRC,"Entree dans session start\n");
	//On caste arg en une sourcce HTTP
	struct srcHTTPSS_t * src = (struct srcHTTPSS_t *) arg;

	printf_debug(DEBUG_SRC,"Initialisation de la connexion TCP\n");

	//Initialiser les connections TCP
	/*On remet le nombre de connexions TCP terminés à 0
	si repassage dans session start*/
	src->nbTCPTermine = 0;

	// Si pas première lecture on doit free les connexions précédentes.
	//On détruit les sources TCP
	if (src->nbPage > 0) {
		//On free les src TCP
		//srcTCPss_free(src->srcTCP[0]);
	}

	// Création de la connexion pour la page principale, élément 0 du tableau
	src->srcTCP[0] = srcTCPss_create(src->MTU, src->RTTmd,
						src->initialWindow, src->destination,
						src->destProcessPDU);

	printf_debug(DEBUG_SRC,"Genere taille page principale\n");
	//On envoie la page principale
	//On recuperer la taille de la page principale
	double sm = randomGenerator_getNextDouble(src->Sm);

	printf("Envoi page principale, taille : %d\n",(int) sm);
	//Envoi de la page principale
int tempsReading = randomGenerator_getNextDouble(src->Dpc);
		printf("Tirage du temps de lecture : %d\n", tempsReading);

	srcTCPss_sendFile(src->srcTCP[0], sm);

	printf_debug(DEBUG_SRC,"Programmation de l'évènement fin de transmition via TCP_addEOTevent\n");
	//On déclenche l'événement fin de transmission de la page principale
	srcTCPss_addEOTEvent(src->srcTCP[0], event_create(srcHTTPSS_EOTMainObject, src, 0));
	printf_debug(DEBUG_SRC,"Evènement programmé\n");

	printf_debug(DEBUG_SRC,"%d : nbPagesLu\n\n", src->nbPage);
}


/*Lancement des objets embarqués*/
void srcHTTPSS_EOTMainObject (void * arg) {
	printf_debug(DEBUG_SRC,"Entrée dans la fct EOTMainObj\n");
	//On caste arg en une sourcce HTTP
	struct srcHTTPSS_t * src = (struct srcHTTPSS_t *) arg;

	printf_debug(DEBUG_SRC,"Génère temps de parsing \n");
	//On calcule le temps de parsing
	int tempsParsing = randomGenerator_getNextDouble(src->Tp);
	printf_debug(DEBUG_SRC,"Temps de parsing : %d\n",tempsParsing);

	//On peut programmer le chargement des objets embarqués
	printf_debug(DEBUG_SRC,"Programmation de l'event envoie des EbOb");
	event_add(srcHTTPSS_sendEmbeddedObjects, src, motSim_getCurrentTime() + tempsParsing);
	printf_debug(DEBUG_SRC,"Event programmé\n\n");
}


/*Envoyer les objets embarqués*/
void srcHTTPSS_sendEmbeddedObjects(void * arg)
{
	printf_debug(DEBUG_SRC,"Entrée dans l'envoie des EbOb\n");
	//On caste arg en une sourcce HTTP
	struct srcHTTPSS_t * src = (struct srcHTTPSS_t *) arg;

	printf_debug(DEBUG_SRC,"Création des connexions TCP (de la)\n");
	int i;
	//On crée de nouvelles connections TCP que si c'est en mode burst version ==0
	if (src->version == 0) {
		for ( i=0; i < src->nbTCP; i++) 
		{
			printf_debug(DEBUG_SRC,"Création de la %d-ème connexion\n",i);
			printf_debug(DEBUG_SRC,"Free de la source TCP \n");
			if (src->nbPage > 0) {
				srcTCPss_free(src->srcTCP[i]);
			}
			src->srcTCP[i] = srcTCPss_create(src->MTU, src->RTTmd, src->initialWindow, src->destination, src->destProcessPDU);
			printf_debug(DEBUG_SRC,"Destination : %p\n",src->destination);
		}
	printf_debug(DEBUG_SRC,"Connexions crée\n");
	}
	printf_debug(DEBUG_SRC,"Fin de création des connexions\n");

	//Calcul du nombre d'objets embarqués
	int Nd = randomGenerator_TruncParetoGetNext(src->Nd);
	//printf("%d\n",Nd);
	printf_debug(DEBUG_SRC,"Tirage du nombre d'objets : %d Nd embedded objects\n",Nd);

	//On envoie les Nd objets embarqués
	for (i = 0; i<Nd; i++) 
	{
		//On calcule la taille du ieme objet embarqué
		double s = randomGenerator_getNextDouble(src->Se);
		//printf("s : %lf\n",s);
		int sizeEbOb = (int) s;
		printf_debug(DEBUG_SRC,"Send file, taille de l'objet : %d\n", sizeEbOb);
		//On envoie le ieme objet embarqué sur le i%(nbTCP) eme connection TCP
		/* Par exemple si 3 connections TCP et 5 objets embarqués
		le 1er objet sur la 1ere connection TCP, la 2eme objet sur le 2eme connection TCP,
		le 3eme objet sur la 3me connection TCP, le 4eme objet sur la 1ere connection TCP,
		le 5eme objet sur la 2eme connection TCP*/
		srcTCPss_sendFile(src->srcTCP[i%(src->nbTCP)], s) ;
	}

	/*Calcul fin de boucle pour ne pas vérifier la terminaison de l'envoi sur une connexion où aucun envoi n'a été fait,
	ERREUR  corrigée : le nombre de connexion a terminé ne change pas, on risque de ne vérifier qu'une*/
	int fin;
	if (src->nbTCP > Nd) {
		fin = Nd;
		// Correction, on termine les sources non vérifiées
		src->nbTCPTermine = src->nbTCPTermine + src->nbTCP - Nd;
		printf("%d : nbTCPTermine, correction\n",src->nbTCPTermine);
	} else {
		fin = src->nbTCP;
	}
	printf_debug(DEBUG_SRC,"%d : FIN\n",fin);

	//On vérifie qu'on a envoyé les objets embarqués
	for (i = 0; i<fin; i++) {
		printf_debug(DEBUG_SRC,"Dans la boucle de vérification de fin d'envoi\n");
	//On déclenche l'événement fin de transmission des Objets Embarquées principale
		printf_debug(DEBUG_SRC,"%d-ème event\n",i);
		srcTCPss_addEOTEvent(src->srcTCP[i], event_create(srcHTTPSS_EOTEmbeddedObjects, src, 0));
		printf_debug(DEBUG_SRC,"%d : nbTCPTermine\n",src->nbTCPTermine);
	}
}


/*Les objets Embarqués ont été envoyés*/
void srcHTTPSS_EOTEmbeddedObjects(void * arg) {
printf_debug(DEBUG_SRC,"Entrée dans la fonction de vérif de fin d'envoi\n");
	//On caste arg en une sourcce HTTP
	struct srcHTTPSS_t * src = (struct srcHTTPSS_t *) arg;

	// Une source TCP a terminé, on incrémente le nombre de connection TCP ayant fini
	src->nbTCPTermine++;
	printf_debug(DEBUG_SRC,"Vérification par if de si tout les connections TCP ont terminé\n");
	if (src->nbTCPTermine == src->nbTCP) {
		//On a lu une page entière
		src->nbPage++;
		printf_debug(DEBUG_SRC,"Tout est terminé, pages lues : %d\n",src->nbPage);

		
		//On prépare le chargement de la nouvelle page
		//On calcule le temps de lecture
		int tempsReading = randomGenerator_getNextDouble(src->Dpc);
		printf("Tirage du temps de lecture : %d\n", tempsReading);

		// Si on a pas finit la session on programme la nouvelle page
		// Ici la session c'est la lecture de 2 pages
		if (src->nbPage < src->nbPageMax) {
			printf_debug(DEBUG_SRC,"Programmation de l'event lire une nouvelle page\n");
			event_add(srcHTTPSS_sessionStart, src, motSim_getCurrentTime()+ tempsReading);
			printf_debug(DEBUG_SRC,"Programmé\n\n");
		} else {
			printf("%d : nombre de page lu FINIII", src->nbPage);
		}

	} else {
		printf_debug(DEBUG_SRC,"Toutes les connections TCP ne sont pas terminées");
		printf_debug(DEBUG_SRC,"%d : nbTCPTermine = %d\n\n",src->nbTCPTermine);
	}
}
