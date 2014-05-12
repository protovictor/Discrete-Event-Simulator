/*     Une source de PDU permet de produire des PDUs */

#include <random-generator.h>
#include <date-generator.h>
#include <pdu.h>
#include <motsim.h>

struct PDUSource_t; //!< Le type d'une source

/**
 * @brief Définition de couples {date, taille}
 *
 * Pour définir explicitement une séquence de PDUs
 */
struct dateSize {
   double date;
   unsigned int size;
};

/**
 * @brief création d'une source de PDUs
 *
 * A chaque source est attribuée une destination et une 
 * fonction permettant de soumettre à cette destination
 * les PDU produites
 * 
 */
struct PDUSource_t * PDUSource_create(struct dateGenerator_t * dateGen,
				      void * destination,
				      processPDU_t destProcessPDU);

/** @brief Création d'un générateur déterministe
 * 
 *  @param sequence Un tableau de {date, size} définissant chaque PDU
 *  @param destination L'entité aval
 *  @param destProcessPDU La fonction de traitement de la destination
 *  @result Un pointeur sur la source créée/initialisée
 *
 *  Un tel générateur permet de définir explicitement la séquence des
 *  PDUs à générer. Cette séquence est définie par un tableau de
 *  couples {date, taille}. Le dernier élément du tableau doit être
 *  {0.0, 0}. Le tableau n'est pas copié, il ne doit donc pas être
 *  libéré tant que la source peut servir.
 */
struct PDUSource_t * PDUSource_createDeterministic(struct dateSize * sequence,
						   void * destination,
						   processPDU_t destProcessPDU);

/**
 * @brief Spécification du générateur de taille de PDU associé
 *
 *  En l'absence d'un tel générateur, les PDUs générées sont de taille
 *  nulle.
 */
void PDUSource_setPDUSizeGenerator(struct PDUSource_t * src,
				   struct randomGenerator_t * rg);

/*
 * Positionnement d'une sonde sur la taille des PDUs produites. Toutes
 * les PDUs créées sont concernées, même si elles ne sont pas
 * récupérées par la destination.
 */
void PDUSource_addPDUGenerationSizeProbe(struct PDUSource_t * src,
					 struct probe_t *  PDUGenerationSizeProbe);


/**
 * @brief Démarrage d'une source dans le cadre d'un simulateur
 *
 * A partir de cet instant, elle peut produire des PDUs.
 */
void PDUSource_start(struct PDUSource_t * source);

/*
 * The function used by the destination to actually get the next PDU
 */
struct PDU_t * PDUSource_getPDU(struct PDUSource_t * source);

struct PDU_t * PDUSource_getNextPDU(struct PDUSource_t * source);

struct probe_t *PDUSource_getPDUGenerationSizeProbe(struct PDUSource_t* source);
