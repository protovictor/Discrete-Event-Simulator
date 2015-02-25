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
 * @brief Definition of a source
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
	int version; // 1 si 1.1, 0 si 1.0
};


/**      A FAIRE ********************
 * @brief Creation/initialization of a source
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

void srcHTTPSS_setSm(struct srcHTTPSS_t * src,
									struct randomGenerator_t * Sm)
{
	src->Sm = Sm;
}

void srcHTTPSS_setSe(struct srcHTTPSS_t * src,struct randomGenerator_t * Se)
{
	src->Se = Se;
}

void srcHTTPSS_setNd(struct srcHTTPSS_t * src,struct randomGenerator_t * Nd)
{
	src->Nd = Nd;
}

void srcHTTPSS_setDpc(struct srcHTTPSS_t * src,struct randomGenerator_t * Dpc)
{
	src->Dpc = Dpc;
}

void srcHTTPSS_setTp(struct srcHTTPSS_t * src,struct randomGenerator_t * Tp)
{
	src->Tp = Tp;
}

void srcHTTPSS_setMTU(struct srcHTTPSS_t * src,int MTU)
{
	src->MTU = MTU;
}

void srcHTTPSS_setversion(struct srcHTTPSS_t * src,int version, int nbTCP )
{
	src->version = version;
	src->nbTCP = nbTCP;
}


/*Création d'une structure afin de passer à event_add un seul argument.*/
typedef struct fonctionsHttpssArguments fonctionsHttpssArguments;
struct fonctionsHttpssArguments
{
	struct srcHTTPSS_t * src;
	void * destination;
	processPDU_t destProcessPDU;
	double RTTmd;
	int initialWindow;
	struct srcTCPSS_t * srcTCP[100];
};

/*Lanchement des objets embarqués*/
void srcHTTPSS_EOTMainObjet (fonctionsHttpssArguments * arg) {

	/*On peut programmer le chargement des objets embarqués*/
	int tempsParsing =randomGenerator_exponentialGetNext(arg -> src -> Tp);
	event_add(srcHTTPSS_sendEmbeddedObjects, arg, motSim_getCurrentTime() + tempsParsing);

}

void srcHTTPSS_sessionStart(fonctionsHttpssArguments * arg) 
{
	/*Initialiser les connections TCP*/
	struct srcTCPSS_t * srcTCP = srcTCPss_create(arg->src->MTU, arg->RTTmd,
														arg->initialWindow, arg->destination,
														arg->destProcessPDU);
	/*On envoie la page principale*/
	srcTCPss_sendFile(srcTCP,randomGenerator_TruncParetoGetNext(arg->src->Sm));

	/*On déclenche l'événement fin de transmission de la page principale*/
	srcTCPss_addEOTEvent(srcTCP, event_create(srcHTTPSS_EOTMainObject,arg, 0.0));
}

/*Les objets Embarqués ont été envoyés*/
void srcHTTPSS_EOTEmbbededObjects(fonctionsHttpssArguments * arg) {
	/*On a lu une page entière*/
	arg->src->nbPage++;
	/*On prépare le chargement de la nouvelle page*/
	int tempsReading =randomGenerator_exponentialGetNext(arg->src->Dpc);
	event_add(srcHTTPSS_sessionStart, arg, motSim_getCurrentTime() + tempsReading);
}
/*Envoyer les objets embarqués*/
void srcHTTPSS_sendEmbeddedObjects(fonctionsHttpssArguments * arg) 
{
	int i;
	/*On crée de nouvelles connections TCP*/
	struct srcTCPSS_t * srcTCP[arg->src->nbTCP];
	for ( i=0; i < arg->src->nbTCP; i++) 
	{
		srcTCP[i] = srcTCPss_create(arg->src->MTU,arg->RTTmd,arg->initialWindow,
														arg->destination, arg->destProcessPDU);
	}

	/*Calculer le nombre d'objets embarqués*/
	int Nd = randomGenerator_TruncParetoGetNext(arg->src->Nd);
	for (i = 0; i<Nd; i++) 
	{
		srcTCPss_sendFile(srcTCP[mod(i, arg->src->nbTCP)], randomGenerator_TruncLogGetNext(arg->src->Se)) ;
	}
	/*On vérifie qu'on a envoyé les objets embarqués*/
	for (i = 0; i<Nd; i++) {
	/*On déclenche l'événement fin de transmission des Objets Embarquées principale*/
		srcTCPss_addEOTEvent(srcTCP[i],
		event_create(srcHTTPSS_EOTEmbbededObjects, arg,0.0));
	}
}


/*
void srcHTTPSS_loadNewPage(fonctionsHttpssArguments * arg) 
{
	//Initialiser la connection TCP
	struct srcTCPSS_t * srcTCP = srcTCPss_create(arg->src->MTU, arg->RTTmd,
														arg->initialWindow, arg->destination,
														arg->destProcessPDU);
	//On envoie la nouvelle page principale
	srcTCPss_sendFile(srcTCP, randomGenerator_TruncParetoGetNext(arg->src -> Sm));
	//On peut programmer le chargement des objets embarquésde la nouvelle page
	int tempsParsing = randomGenerator_exponentialGetNext(arg->src->Tp);
	event_add(srcHTTPSS_sendEmbeddedObjects, arg,	motSim_getCurrentTime() + tempsParsing);
}
*/
