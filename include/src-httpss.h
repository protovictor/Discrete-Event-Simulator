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
 * @param nbPage nombre de pages visités
 * @param MTU is the maximum transmission unit of the link
 * @param RTTmd is the Round Trip Time minus transmission time on the access link
 * @param initialWindow is the initial value of cwnd
 * @param destination is a pointer to the destination entity
 * @param destProcessPDU is the PDU processing function of the destination
 * @param version 1 si 1.1, 0 si 1.0
 * @param nbTCP nombre de connections TCP utilisé pour charger les objets (==1 en 1.1)
 */
struct srcHTTPSS_t * srcHTTPSS_init(struct randomGenerator_t * Sm, struct randomGenerator_t * Se,
				struct randomGenerator_t * Nd,
				struct randomGenerator_t * Dpc,
				struct randomGenerator_t * Tp,
				int MTU, int nbTCP, int version);

/**
 * @brief Creation of HTTP source, with default values for distribution given by http://www.3gpp2.org/Public_html/specs/C.R1002-0_v1.0_041221.pdf
 * @param MTU maximum transimtion unit of the link
 * @param nbTCP nombre de connexion TCP (1 pour HTTP 1.1)
 * @param version : 0 pour hhtp 1.0 (burst-mode) et 1 pour http 1.1 (persistent
 * mode
 */
struct srcHTTPSS_t * srcHTTPSS_init_default(int MTU, int nbTCP, int version);

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
 * @param  version True si 1.1, false si 1.0
 * @param nbTCP nombre de connections TCP utilisé pour charger les objets (==1 en 1.1)
 * @param src HTTP source
*/
void srcHTTPSS_setversion(struct srcHTTPSS_t * src, int version, int nbTCP);

/*------------------------------------------------------------------------------------------*/
			/*Fonctions pour lancer la session HTTP*/

typedef struct fonctionsHttpssArguments fonctionsHttpssArguments;
struct fonctionsHttpssArguments;

/**
 * @brief Dans cette fonction on va créer un source TCP (d'ou les paramètres pour la connection TCP).
 * On va envoyer la page principale à partir de cette fonction, et programmer le chargement
 * des objets embarqués qui est traité par la fonction srcHTTPSS_EOTMainObject
 * @param arg structure contenant la source HTTP, les infos pour une sources TCP et le nombre
 * de connections TCP terminés lors d'un envoi sur plusieurs connections.
 * arg->src source HTTP, we have src-> MTU as maximum transmission unit of the link
 * arg->RTTmd is the Round Trip Time minus transmission time on the access link
 * arg->initialWindow is the initial value of cwnd
 * arg->destination is a pointer to the destination entity
 * arg->destProcessPDU is the PDU processing function of the destination
*/
void srcHTTPSS_sessionStart(void * arguments);

/**
 * @brief On a fini d'envoyé l'objet principal
 * on va pouvoir alors programmer l'envoie des objets embarqués aprés le temps de parsing
 * @param arg structure contenant la source HTTP, les infos pour une sources TCP et le nombre
 * de connections TCP terminés lors d'un envoi sur plusieurs connections.
 * arg->src source HTTP, we have src-> MTU as maximum transmission unit of the link
 * arg->RTTmd is the Round Trip Time minus transmission time on the access link
 * arg->initialWindow is the initial value of cwnd
 * arg->destination is a pointer to the destination entity
 * arg->destProcessPDU is the PDU processing function of the destination
*/
void srcHTTPSS_EOTMainObject(void * arguments);

/**
 * @brief Dans cette fonction on va créer des sources TCP (d'ou les paramètres pour connections TCP).
 * On va envoyer les objets embarquées à partir de cette fonction, et programmer le chargement
 * d'une nouvelle page.
 * La préparation du chargement de la nouvelle page est traitée par srcHTTPSS_EOTEmbeddedObejcts
 * @param arg structure contenant la source HTTP, les infos pour une sources TCP et le nombre
 * de connections TCP terminés lors d'un envoi sur plusieurs connections.
 * arg->src source HTTP, we have src-> MTU as maximum transmission unit of the link
 * arg->RTTmd is the Round Trip Time minus transmission time on the access link
 * arg->initialWindow is the initial value of cwnd
 * arg->destination is a pointer to the destination entity
 * arg->destProcessPDU is the PDU processing function of the destination
*/
void srcHTTPSS_sendEmbeddedObjects(void * arguments);

/**
 * @brief Chaque connections TCP qui a fini d'envoyé groupe d'objets embarqués va
 * passer par cette fonction. Si tous les connections TCP on fini d'envoyer
 * On va preparer le chargement d'une nouvelle page à l'aide d'un événement
 * La fonction qui prendra la main aprés Dpc secondes sera srcHTTPSS_sessionStart
 * @param arg structure contenant la source HTTP, les infos pour une sources TCP et le nombre
 * de connections TCP terminés lors d'un envoi sur plusieurs connections.
 * arg->src source HTTP, we have src-> MTU as maximum transmission unit of the link
 * arg->RTTmd is the Round Trip Time minus transmission time on the access link
 * arg->initialWindow is the initial value of cwnd
 * arg->destination is a pointer to the destination entity
 * arg->destProcessPDU is the PDU processing function of the destination
*/
void srcHTTPSS_EOTEmbeddedObjects(void * arguments);

#endif
