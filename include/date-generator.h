/**
 * @file date-generator.h
 * @brief Les générateurs de dates
 * Ils sont largement fondé sur les générateurs de
 * nombres aléatoires.
 * Toutes les dates sont toujours exprimées en secondes depuis le début de
 * la simulation.
 */

#ifndef __DEF_DATE_GENERATOR
#define __DEF_DATE_GENERATOR

#include <probe.h>
#include <random-generator.h>

struct dateGenerator_t;

/**
 * @brief Creation of a date generator
 * @result a struct dateGenerator_t * 
 *
 * The created dateGenerator is unusable for now. It needs to be
 * associated to a random generator
 */
struct dateGenerator_t * dateGenerator_create();

/**
 * Creation d'une source qui genere des evenements a interrarivee
 * exponentielle.
 */
struct dateGenerator_t * dateGenerator_createExp(double lambda);

/**
 * Creation d'une source qui genere des evenements a interrarivee
 * constante. Bref, périodiques !
 */
struct dateGenerator_t * dateGenerator_createPeriodic(double period);

/**
 * @brief Choix du générateur aléatoire des durées entre dates
 *
 * @param dateGen le générateur à modifier
 * @param randGen le générateur de date à affecter
 */
void dateGenerator_setRandomGenerator(struct dateGenerator_t * dateGen,
				      struct randomGenerator_t * randGen);

/**
 * @brief Ajout d'une sonde sur les inter-arrivees
 *
 * @param dateGen le générateur à modifier
 * @param probe la sonde à affecter
 */
void dateGenerator_addInterArrivalProbe(struct dateGenerator_t * dateGen,
					struct probe_t * probe);

/** @brief Obtention de la prochaine date
 *
 *  @param dateGen le générateur à utiliser
 *  @param currentTime la date actuelle
 */
double dateGenerator_nextDate(struct dateGenerator_t * dateGen,
			      double currentTime);

/*
 * Modification du paramètre lambda
 */
void dateGenerator_setLambda(struct dateGenerator_t * dateGen, double lambda);

/*
 * Prepare for record values in order to replay on each reset
 */
void dateGenerator_recordThenReplay(struct dateGenerator_t *  d);

#endif
