/**
 * @file src-httpss.h
 * @brief A very basic HTTP source model (slow start only)
 *
 * This module implements a very basic HTTP source model as described
 * in [1].
*/
#ifndef __SRC_HTTP_H__
#define __SRC_HTTP_H__

#include <random-generator.h>

struct srcHTTPSS_t;

/**
 * @brief Creation/initialization of a source
 * @param Sm Type of Random, Generator Size of the main object in a page
 * @param Se Random Generator, Size of an embedded object in a page
 * @param Nd Random Generator, Number of embedded objects in a page
 * @param Dpc Random Generator, Reading time
 * @param Tp Random Generator, Parsing time for the main page.
 * @param nbPage number of visited pages
 * @param MTU is the maximum transmission unit of the link
 * @param RTTmd is the Round Trip Time minus transmission time on the access link
 * @param initialWindow is the initial value of cwnd
 * @param destination is a pointer to the destination entity
 * @param destProcessPDU is the PDU processing function of the destination
 * @param version 1 si 1.1, 0 si 1.0
 * @param nbTCP nombre de connections TCP utilisé pour charger les objets (==1 en 1.1)
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
					int initialWindow);

/**
 * @brief Creation of HTTP source, with default values for distribution given by http://www.3gpp2.org/Public_html/specs/C.R1002-0_v1.0_041221.pdf
 * @param MTU maximum transimtion unit of the link
 * @param nbTCP number of TCP connections (1 for HTTP 1.1)
 * @param version : 0 for hhtp 1.0 (burst-mode) and 1 for http 1.1 (persistent
 * mode
 */
struct srcHTTPSS_t * srcHTTPSS_init_default(int MTU, int nbTCP, int version, double RTTmd, int initialWindow, void * destination, processPDU_t destProcessPDU);

/*-------------------------------------------------------------------------------------------*/
					/*Setters*/
/*-------------------------------------------------------------------------------------------*/
/**
 * @brief Set the Random Generator Sm on the HTTP structure
 * @param Sm Type of Random, Generator Size of the main object in a page
 * @param src HTTP source
*/
void srcHTTPSS_setSm(struct srcHTTPSS_t * src, struct randomGenerator_t * Sm);

/**
 * @brief Set the Random Generator Se on the HTTP structure
 * @param Se Type of Random, Size of an embedded object in a page
 * @param src HTTP source
*/
void srcHTTPSS_setSe(struct srcHTTPSS_t * src, struct randomGenerator_t * Se);

/**
 * @brief Set the Random Generator Nd on the HTTP structure
 * @param Nd Random Generator, Number of embedded objects in a page
 * @param src HTTP source
*/
void srcHTTPSS_setNd(struct srcHTTPSS_t * src, struct randomGenerator_t * Nd);

/**
 * @brief Set the Random Generator Dpc on the HTTP structure
 * @param  Dpc Random Generator, Reading time
 * @param src HTTP source
*/
void srcHTTPSS_setDpc(struct srcHTTPSS_t * src, struct randomGenerator_t * Dpc);

/**
 * @brief Set the Random Generator Tp on the HTTP structure
 * @param  Tp Random Generator, Parsing time for the main page.
 * @param src HTTP source
*/
void srcHTTPSS_setTp(struct srcHTTPSS_t * src, struct randomGenerator_t * Tp);

/**
 * @brief Set the Random Generator MTU on the HTTP structure
 * @param  MTU is the maximum transmission unit of the link
 * @param src HTTP source
*/
void srcHTTPSS_setMTU(struct srcHTTPSS_t * src, int MTU);

/**
 * @brief Set the Random Generator MTU on the HTTP structure
 * @param  version 1.1 if true 1.0 if false
 * @param nbTCP number of TCP connections used to load objects (==1 in 1.1)
 * @param src HTTP source
*/
void srcHTTPSS_setversion(struct srcHTTPSS_t * src, int version, int nbTCP);

/*------------------------------------------------------------------------------------------*/
			/*Fonctions pour lancer la session HTTP*/

/*typedef struct fonctionsHttpssArguments fonctionsHttpssArguments;
struct fonctionsHttpssArguments;*/

/**
 * @brief This function creates a TCP source (hence the parameters for TCP
 * connection). 
 * It sends the main page and program downloading of embedded objects, treated
 * by the srcHTTPSS_EOTMainObject function
 * @param arg structure with the HTTP source, the informations for a TCP source
 * and the number of ended TCP connections when using several connections for
 * sending
 * arg->src source HTTP, we have src-> MTU as maximum transmission unit of the link
 * arg->RTTmd is the Round Trip Time minus transmission time on the access link
 * arg->initialWindow is the initial value of cwnd
 * arg->destination is a pointer to the destination entity
 * arg->destProcessPDU is the PDU processing function of the destination
*/
void srcHTTPSS_sessionStart(void * src) ;

/**
 * @brief This function sends embedded objects after a
 * parsing time when the main object is already sent.
 * @param arg structure with the HTTP source, the informations for a TCP source
 * and the number of ended TCP connections when using several connections for
 * sending
 * arg->src source HTTP, we have src-> MTU as maximum transmission unit of the link
 * arg->RTTmd is the Round Trip Time minus transmission time on the access link
 * arg->initialWindow is the initial value of cwnd
 * arg->destination is a pointer to the destination entity
 * arg->destProcessPDU is the PDU processing function of the destination
*/
void srcHTTPSS_EOTMainObject (struct srcHTTPSS_t * arg);

/**
 * @brief This functions creates TCP sources, then sends embedded objects and
 * computes the loading  of a new page (see srcHTTPSS_EOTEmbeddedObjects). 
 * @param arg structure with the HTTP source, the informations for a TCP source
 * and the number of ended TCP connections when using several connections for
 * sending
 * arg->src source HTTP, we have src-> MTU as maximum transmission unit of the link
 * arg->RTTmd is the Round Trip Time minus transmission time on the access link
 * arg->initialWindow is the initial value of cwnd
 * arg->destination is a pointer to the destination entity
 * arg->destProcessPDU is the PDU processing function of the destination
*/
void srcHTTPSS_sendEmbeddedObjects(struct srcHTTPSS_t * src);

/**
 * @brief Whenever a TCP connection has successfully sent an embedded object group, it is held by this function. When all TCP connections are done, they are freed, and a new page will be generated by srcHTTPSS_sessionSrtart 
 * @param arg structure with the HTTP source, the informations for a TCP source
 * and the number of ended TCP connections when using several connections for
 * sending

 * arg->src source HTTP, we have src-> MTU as maximum transmission unit of the link
 * arg->RTTmd is the Round Trip Time minus transmission time on the access link
 * arg->initialWindow is the initial value of cwnd
 * arg->destination is a pointer to the destination entity
 * arg->destProcessPDU is the PDU processing function of the destination
*/
void srcHTTPSS_EOTEmbeddedObjects(void * src);

#endif
