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
};


/**
 * @brief Creation/initialization of a HTTP source
 * @param MTU is the maximum transmission unit of the link
 * @param RTTmd is the Round Trip Time minus transmission time on the access link
 * @param initialWindow is the initial value of cwnd
 * @param destination is a pointer to the destination entity
 * @param destProcessPDU is the PDU processing function of the destination
 */
struct srcHTTPSS_t * srcHTTPSS_init(struct randomGenerator_t * Sm,
								struct randomGenerator_t * Se,
								struct randomGenerator_t * Nd,
								struct randomGenerator_t * Dpc,
								struct randomGenerator_t * Tp,
								int MTU, int nbTCP,
								int version) 
{	
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
typedef struct fonctionsHttpssArguments fonctionsHttpssArguments;
struct fonctionsHttpssArguments
{
	struct srcHTTPSS_t * src;
	void * destination;
	processPDU_t destProcessPDU;
	double RTTmd;
	int initialWindow;
	struct srcTCPSS_t * srcTCP[100];
	int nbTCPTermine;
};


void srcHTTPSS_sessionStart(fonctionsHttpssArguments * arg) 
{
	/*Initialiser les connections TCP*/
	arg->nbTCPTermine = 0;
	arg->srcTCP[0] = srcTCPss_create(arg->src->MTU, arg->RTTmd,
						arg->initialWindow, arg->destination,
						arg->destProcessPDU);
	/*On envoie la page principale*/
	srcTCPss_sendFile(arg->srcTCP[0], randomGenerator_TruncParetoGetNext(arg->src->Sm));

	/*On déclenche l'événement fin de transmission de la page principale*/
	srcTCPss_addEOTEvent(arg->srcTCP[0], event_create(srcHTTPSS_EOTMainObject, arg, 0.0));
}

/*Lancement des objets embarqués*/
void srcHTTPSS_EOTMainObjet (fonctionsHttpssArguments * arg) {
	/*On détruit la source TCP*/
	srcTCPss_free(arg->srcTCP[0]);
	/*On peut programmer le chargement des objets embarqués*/
	int tempsParsing =randomGenerator_exponentialGetNext(arg -> src -> Tp);
	event_add(srcHTTPSS_sendEmbeddedObjects, arg, motSim_getCurrentTime() + tempsParsing);

}

/*Envoyer les objets embarqués*/
void srcHTTPSS_sendEmbeddedObjects(fonctionsHttpssArguments * arg) 
{
	int i;
	/*On crée de nouvelles connections TCP*/
	for ( i=0; i < arg->src->nbTCP; i++) 
	{
		arg->srcTCP[i] = srcTCPss_create(arg->src->MTU, arg->RTTmd, arg->initialWindow, arg->destination, arg->destProcessPDU);
	}

	/*Calculer le nombre d'objets embarqués*/
	int Nd = randomGenerator_TruncParetoGetNext(arg->src->Nd);
	for (i = 0; i<Nd; i++) 
	{
		srcTCPss_sendFile(arg->srcTCP[mod(i, arg->src->nbTCP)], randomGenerator_TruncLogGetNext(arg->src->Se)) ;
	}
	/*On vérifie qu'on a envoyé les objets embarqués*/
	for (i = 0; i<Nd; i++) {
	/*On déclenche l'événement fin de transmission des Objets Embarquées principale*/
		srcTCPss_addEOTEvent(arg->srcTCP[i], event_create(srcHTTPSS_EOTEmbeddedObjects, arg, 0.0));
	}
}

/*Les objets Embarqués ont été envoyés*/
void srcHTTPSS_EOTEmbeddedObjects(fonctionsHttpssArguments * arg) {
	if (arg->nbTCPTermine == arg->src->nbTCP - 1) {
		//On a lu une page entière
		arg->src->nbPage++;
		arg->nbTCPTermine = 0;
		//On détruit les sources TCP
		int i;
		for ( i=0; i < arg->src->nbTCP; i++)
		{
			arg->srcTCP[i] = srcTCPss_free(arg->srcTCP[i]);
		}

		/*On prépare le chargement de la nouvelle page*/
		int tempsReading =randomGenerator_exponentialGetNext(arg->src->Dpc);
		event_add(srcHTTPSS_sessionStart, arg, motSim_getCurrentTime() + tempsReading);

	} else {
		arg->nbTCPTermine++;
	}
}
