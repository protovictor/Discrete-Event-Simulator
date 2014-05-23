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

#include "probe.h"

struct dateGenerator_t;

struct dateGenerator_t * dateGenerator_create();

/**
 * @brief Ajout d'une sonde sur les inter-arrivees
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
 * Creation d'une source qui genere des evenements a interrarivee
 * exponentielle.
 */
struct dateGenerator_t * dateGenerator_createExp(double lambda);
struct dateGenerator_t * dateGenerator_createLognormal(double alpha, double beta);
struct dateGenerator_t * dateGenerator_createWeibull(double alpha, double beta);
struct dateGenerator_t * dateGenerator_createGamma(double alpha, double beta);

/*
 * Modification du paramètre lambda
 */
void dateGenerator_setLambda(struct dateGenerator_t * dateGen, double lambda);
void dateGenerator_setAlpha(struct dateGenerator_t * dateGen, double alpha);
void dateGenerator_setBeta(struct dateGenerator_t * dateGen, double beta);
/*
 * Creation d'une source qui genere des evenements a interrarivee
 * constante. Bref, périodiques !
 */
struct dateGenerator_t * dateGenerator_createPeriodic(double period);

/*
 * Prepare for record values in order to replay on each reset
 */
void dateGenerator_recordThenReplay(struct dateGenerator_t *  d);


#endif
