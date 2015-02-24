/**
 * @file src-httpss.h
 * @brief A very basic HTTP source model (slow start only)
 *
 * This module implements a very basic HTTP source model as described
 * in [1].
*/
#ifndef __SRC_HTTP_H__
#define __SRC_HTTP_H__

struct srcHTTPSS_t;

/**
 * @brief Creation/initialization of a source
 * @param Sm Type of Random, Generator Size of the main object in a page
 * @param Se Random Generator, Size of an embedded object in a page
 * @param Nd Random Generator, Number of embedded objects in a page
 * @param Dpc Random Generator, Reading time
 * @param Tp Random Generator, Parsing time for the main page.
 * @param nbPage nombre de pages visités
 * @param MTU is the maximum transmission unit of the link
 * @param RTTmd is the Round Trip Time minus transmission time on the access link
 * @param initialWindow is the initial value of cwnd
 * @param destination is a pointer to the destination entity
 * @param destProcessPDU is the PDU processing function of the destination
 * @param version True si 1.1, false si 1.0
 * @param nbTCP nombre de connections TCP utilisé pour charger les objets (==1 en 1.1)
 */
struct srcHTTPSS * srcHTTPSS_init(struct randomGenerator_t * Sm,struct randomGenerator_t * Se,
				struct randomGenerator_t * Nd,
				struct randomGenerator_t * Dpc,
				struct randomGenerator_t * Tp,
				int MTU, int nbTCP, int nbPage,
				boolean version);

/*-------------------------------------------------------------------------------------------*/
					/*Setters*/
/*-------------------------------------------------------------------------------------------*/
/**
 * @brief Set the Random Generator Sm on the HTTP structure
 * @param Sm Type of Random, Generator Size of the main object in a page
 * @param src HTTP source
*/
struct srcHTTPSS * srcHTTPSS_setSm(struct srcHTTPSS_t * src, struct randomGenerator_t * Sm)

/**
 * @brief Set the Random Generator Se on the HTTP structure
 * @param Se Type of Random, Size of an embedded object in a page
 * @param src HTTP source
*/
struct srcHTTPSS * srcHTTPSS_setSe(struct srcHTTPSS_t * src, struct randomGenerator_t * Se)

/**
 * @brief Set the Random Generator Nd on the HTTP structure
 * @param Nd Random Generator, Number of embedded objects in a page
 * @param src HTTP source
*/
struct srcHTTPSS * srcHTTPSS_setNd(struct srcHTTPSS_t * src, struct randomGenerator_t * Nd)

/**
 * @brief Set the Random Generator Dpc on the HTTP structure
 * @param  Dpc Random Generator, Reading time
 * @param src HTTP source
*/
struct srcHTTPSS * srcHTTPSS_setDpc(struct srcHTTPSS_t * src, struct randomGenerator_t * Dpc)

/**
 * @brief Set the Random Generator Tp on the HTTP structure
 * @param  Tp Random Generator, Parsing time for the main page.
 * @param src HTTP source
*/
struct srcHTTPSS * srcHTTPSS_setTp(struct srcHTTPSS_t * src, struct randomGenerator_t * Tp)

/**
 * @brief Set the Random Generator MTU on the HTTP structure
 * @param  MTU is the maximum transmission unit of the link
 * @param src HTTP source
*/
struct srcHTTPSS * srcHTTPSS_setMTU(struct srcHTTPSS_t * src, int MTU)

/**
 * @brief Set the Random Generator MTU on the HTTP structure
 * @param  version True si 1.1, false si 1.0
 * @param nbTCP nombre de connections TCP utilisé pour charger les objets (==1 en 1.1)
 * @param src HTTP source
*/
struct srcHTTPSS * srcHTTPSS_setversion(struct srcHTTPSS_t * src, bool version, int nbTCP)

/*------------------------------------------------------------------------------------------*/
			/*Fonctions pour lancer la session HTTP*/

/**
 * @brief Dans cette fonction on va créer un source TCP (d'ou les paramètres pour la connection TCP).
 * On va envoyer la page principale à partir de cette fonction, et programmer 
*/
void srcHTTPSS_sessionStart(struct srcHTTPSS * src, void * destination, processPDU_t destProcessPDU,
							double RTTmd, int initialWindow);


void srcHTTPSS_sendEmbeddedObjects(struct srcHTTPSS * src, void * destination,
									processPDU_t destProcessPDU, double RTTmd,
									int initialWindow);
void srcHTTPSS_loadNewPage(src, void * destination, processPDU_t destProcessPDU, 
									double RTTmd, int initialWindow);
