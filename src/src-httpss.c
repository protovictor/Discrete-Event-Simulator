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
#include <event-file.h>
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
	struct srcTCPSS_t * srcTCP[100]; //sources TCP utilisés par l'objet HTTP
	int nbTCPTermine; //le nombre de connecxions TCP ayant envoyer tout leurs paquets
	double RTTmd; //is the Round Trip Time minus transmission time on the access link
	int initialWindow; //is the initial value of cwnd
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
	struct srcHTTPSS_t * src = (struct srcHTTPSS_t *) arg;
	/*Initialiser les connections TCP*/
	src->nbTCPTermine = 0;
	src->srcTCP[0] = srcTCPss_create(src->MTU, src->RTTmd,
						src->initialWindow, src->destination,
						src->destProcessPDU);
	/*On envoie la page principale*/
	srcTCPss_sendFile(src->srcTCP[0],10000); //randomGenerator_TruncParetoGetNext(src->Sm));
	printf("TCPStarded,\n\n");
	/*On déclenche l'événement fin de transmission de la page principale*/
	srcTCPss_addEOTEvent(src->srcTCP[0], event_create((void (*)(struct srcHTTPSS_t *data))srcHTTPSS_EOTMainObject, (struct srcHTTPSS_t*)src, motSim_getCurrentTime()));
	//event_add((void (*)(void *data))srcHTTPSS_EOTMainObject, (void*)src, 20.0);
	printf("Still in SessionStartAtTheEND\n\n");
}

/*Lancement des objets embarqués*/
void srcHTTPSS_EOTMainObject (struct srcHTTPSS_t  * arg) {
  	printf("TCDDONE\n\n\n");
	struct srcHTTPSS_t * src = (struct srcHTTPSS_t *) arg;
	printf("blalbla");
	/*On détruit la source TCP*/
	srcTCPss_free(src->srcTCP[0]);
	/*On peut programmer le chargement des objets embarqués*/
	int tempsParsing = randomGenerator_exponentialGetNext(src->Tp);
	printf(" blabla Parsing, %d\n\n", tempsParsing); 
	//event_add(srcHTTPSS_sendEmbeddedObjects, src, motSim_getCurrentTime() + );
	//event_add((void (*)(void *data))srcHTTPSS_sendEmbeddedObjects, (void*)src, motSim_getCurrentTime());// + tempsParsing);
	//srcHTTPSS_sendEmbeddedObjects(src);

}


/*Envoyer les objets embarqués*/
void srcHTTPSS_sendEmbeddedObjects(struct srcHTTPSS_t * arg)
{
  	printf("HAAAAAAAAAHHHHHHHHHHHHHAAAAAAAAAA");
	struct srcHTTPSS_t * src = (struct srcHTTPSS_t *) arg;
	int i;
	/*On crée de nouvelles connections TCP*/
	for ( i=0; i < src->nbTCP; i++) 
	{
		src->srcTCP[i] = srcTCPss_create(src->MTU, src->RTTmd, src->initialWindow, src->destination, src->destProcessPDU);
	}

	/*Calculer le nombre d'objets embarqués*/
	int Nd = randomGenerator_TruncParetoGetNext(src->Nd);
	for (i = 0; i<Nd; i++) 
	{
		srcTCPss_sendFile(src->srcTCP[i%(src->nbTCP)], randomGenerator_TruncLogGetNext(src->Se)) ;
	}
	/*On vérifie qu'on a envoyé les objets embarqués*/
	for (i = 0; i<Nd; i++) {
	/*On déclenche l'événement fin de transmission des Objets Embarquées principale*/
		srcTCPss_addEOTEvent(src->srcTCP[i], event_create(srcHTTPSS_EOTEmbeddedObjects, src, 0.0));
	}
}

/*Les objets Embarqués ont été envoyés*/
void srcHTTPSS_EOTEmbeddedObjects(void * arg) {
	struct srcHTTPSS_t * src = (struct srcHTTPSS_t *) arg;
	if (src->nbTCPTermine == src->nbTCP - 1) {
		//On a lu une page entière
		src->nbPage++;
		//On détruit les sources TCP
		int i;
		for ( i=0; i < src->nbTCP; i++)
		{
			srcTCPss_free(src->srcTCP[i]);
		}

		/*On prépare le chargement de la nouvelle page*/
		int tempsReading =randomGenerator_exponentialGetNext(src->Dpc);
		event_add(srcHTTPSS_sessionStart, src, motSim_getCurrentTime() + tempsReading);

	} else {
		src->nbTCPTermine++;
	}
}
