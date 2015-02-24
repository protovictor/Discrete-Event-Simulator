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
#include <event.h>
#include <motsim.h>
#include <file_pdu.h>
#include <random-generator.h>

/**
 * @brief Definition of a source
 */
struct srcHTTPSS_t {
	struct randomGenerator_t Sm; // Size of the main object in a page
	struct randomGenerator_t Se; // Size of an embedded object in a page
	struct randomGenerator_t Nd; // Number of embedded objects in a page
	struct randomGenerator_t Dpc; // Reading time
	struct randomGenerator_t Tp; // Parsing time for the main page.
	int MTU;
	int nbTCP; // nombre de connexion TCP selon le mode
	int nbPage; // nombre de page visité
	bool version; // True si 1.1, false si 1.0
};


/**      A FAIRE ********************
 * @brief Creation/initialization of a source
 * @param MTU is the maximum transmission unit of the link
 * @param RTTmd is the Round Trip Time minus transmission time on the access link
 * @param initialWindow is the initial value of cwnd
 * @param destination is a pointer to the destination entity
 * @param destProcessPDU is the PDU processing function of the destination
 */
struct srcHTTPSS * srcHTTPSS_init(struct randomGenerator_t * Sm,
								struct randomGenerator_t * Se,
								struct randomGenerator_t * Nd,
								struct randomGenerator_t * Dpc,
								struct randomGenerator_t * Tp,
								int MTU, int nbTCP, int nbPage,
								bool version) 
{
	struct srcHTTPSS_t * result = 
		(struct srcHTTPSS_t *) sim_malloc(sizeof(struct srcHTTPSS_t ));

	result → Sm = Sm;
	result → Se = Se;
	result → Nd = Sd;
	result → Dpc = Dpc;
	result → Tp = Tp;							
	result → MTU = MTU; 
	result → nbTCP = nbTCP; 
	result → nbPage = 0;
	result → version = version;

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

void srcHTTPSS_setversion(struct srcHTTPSS_t * src,bool version, int nbTCP )
{
	src->version = version;
	src->nbTCP = nbTCP;
}


void srcHTTPSS_sessionStart(struct srcHTTPSS * src, void * destination,
							processPDU_t destProcessPDU,
							double RTTmd, int initialWindow) 
{
	/*Initialiser les connections TCP*/
	struct srcTCPSS_t * srcTCP = struct srcTCPSS_t * srcTCPss_create(src->MTU, RTTmd,
														initialWindow, destination,
														destProcessPDU);
	/*On envoie la page principale*/
	srcTCPss_sendFile(srcTCP,randomGenerator_TruncParetoGetNext(src->Sm));
	/*On peut programmer le chargement des objets embarqués*/
	int tempsParsing = randomGenerator_exponentialGetNext(src->Tp);

///**** A REFLECHIR ************/////////////
//A L4AIDE D4UNE STRUCTURE
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	event_add(srcHTTPss_sendEmbeddedObjects, {src, destination,	destProcessPDU,
											RTTmd,initialWindow},
											motSim_getCurrentTime() + tempsParsing);
}

/*Envoyer les objets embarqués*/
void srcHTTPSS_sendEmbeddedObjects(struct srcHTTPSS * src, void * destination,
									processPDU_t destProcessPDU, double RTTmd,
									int initialWindow) 
{
	/*On crée de nouvelles connections TCP*/
	srcTCPSS[] * srcTCP = scrTCP[src → nbTCP]
	for (int i = 0; i < src → nbTCP; i++) 
	{
		srcTCP[i] = struct srcTCPSS_t * srcTCPss_create(src->MTU,RTTmd,initialWindow,
														destination, destProcessPDU)
	}

	/*Calculer le nombre d'objets embarqués*/
	int Nd = randomGenerator_TruncParetoGetNext(src->Nd);
	for (int i =0, i<Nd, i++) 
	{
		srcTCPss_sendFile(srcTCP[mod(i, src->nbTCP)], randomGenerator_TruncLogGetNext(src->Se)) ;
	}
	/*On a lu une page entière*/
	src->nbPage++;
	/*On prépare le chargement de la nouvelle page*/
	int tempsReading = randomGenerator_exponentialGetNext(src->Dpc)
	event_add(srcHTTPss_loadNewPage, [src, destination,	destProcessPDU,	RTTmd,	initialWindow],    //// IDEM UTILISER UNE STRUCTURE
									motSim_getCurrentTime() + tempsReading);
}


void srcHTTPSS_loadNewPage(src, void * destination, processPDU_t destProcessPDU, 
									double RTTmd, int initialWindow) 
{
	/*On envoie la nouvelle page principale*/
	srcTCPss_sendFile(srcTCP[0], randomGenerator_TruncParetoGetNext(src -> Sm));
	/*On peut programmer le chargement des objets embarquésde la nouvelle page*/
	int tempsParsing = randomGenerator_exponentialGetNext(src->Tp);
	event_add(srcHTTPss_sendEmbeddedObjects, [src, destination, destProcessPDU,	RTTmd,     //// IDEM UTILISER UNE STRUCTURE
											initialWindow],
											motSim_getCurrentTime() + tempsParsing);
}

