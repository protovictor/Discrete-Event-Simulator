/*     Une source de PDU permet de produire des PDUs */

#include <random-generator.h>
#include <date-generator.h>
#include <pdu.h>
#include <motsim.h>

struct PDUSource_t;

/*
 * A chaque source est attribuée une destination et une 
 * fonction permettant de soumettre à cette destination
 * les PDU produites
 */
struct PDUSource_t * PDUSource_create(struct dateGenerator_t * dateGen,
				      void * destination,
				      processPDU_t destProcessPDU);

/*
 * Spécification du générateur de taille de PDU associé. En l'absence
 * d'un tel générateur, les PDUs générées sont de taille nulle.
 */
void PDUSource_setPDUSizeGenerator(struct PDUSource_t * src, struct randomGenerator_t * rg);

/*
 * Positionnement d'une sonde sur la taille des PDUs produites. Toutes
 * les PDUs créées sont concernées, même si elles ne sont pas
 * récupérées par la destination.
 */
void PDUSource_addPDUGenerationSizeProbe(struct PDUSource_t * src, struct probe_t *  PDUGenerationSizeProbe);

/*
 * Démarrage d'une source dans le cadre d'un simulateur.
 * A partir de cet instant, elle peut produire des PDUs.
 */
void PDUSource_start(struct PDUSource_t * source);

/*
 * The function used by the destination to actually get the next PDU
 */
struct PDU_t * PDUSource_getPDU(struct PDUSource_t * source);
