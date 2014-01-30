/**
 * @file random-generator.h
 * @brief Les générateurs de nombres aléatoires.
 *
 * Un générateur est caractérisé par plusieurs propriétés
 *
 * - Le type des valeurs générées.
 * - La distribution des probabilités
 *    . Les paramètres de cette distribution : min, max, moyenne, ...
 * - La source d'aléa. Certaines permettent un rejeu (rand unix)
 *   d'autres sont "vraiment aléatoires" (/dev/random)
 *    . Les paramètres de cette source (seed, ...)
 */

#ifndef __DEF_RANDOM_GENERATOR
#define __DEF_RANDOM_GENERATOR

#include <probe.h>

struct randomGenerator_t;

/*
 * Available types for the random values
 */
#define rGTypeUInt         1
#define rGTypeULong        2
#define rGTypeFloat        3
#define rGTypeDouble       4
#define rGTypeDoubleRange  5
#define rGTypeUIntEnum     6
#define rGTypeDoubleEnum   7
#define rGTypeUIntRange    8
#define rGTypeUIntConstant 9


/*
 * Available distributions
 */
#define rGDistNoDist       0
#define rGDistUniform      1
#define rGDistExponential  2
#define rGDistDiscrete     3

#define rGDistDefault rGDistUniform
/*
 * Entropy sources
 */
#define rGSourceErand48 1
#define rGSourceReplay  2
#define rgSourceUrandom 3

#define rgSourceDefault rGSourceErand48

/*==========================================================================*/
/*  Creators                                                                */
/*==========================================================================*/

// Des entiers non signés quelconques
struct randomGenerator_t * randomGenerator_createUInt();

// Des entiers non signés entre min et max  (inclus)
struct randomGenerator_t * randomGenerator_createUIntRange(unsigned int min,
						      unsigned int max);

// Des entiers non signés tous �gaux !
struct randomGenerator_t * randomGenerator_createUIntConstant(unsigned int v);

// Des entiers non signés listés
struct randomGenerator_t * randomGenerator_createUIntDiscrete(int nbValues,
							      unsigned int * values);
/*
 * Le nombre de valeurs possibles est passé en paramètre ainsi que la
 * liste de ces valeurs puis la liste de leurs probabilité.
 */
struct randomGenerator_t * randomGenerator_createUIntDiscreteProba(int nbValues,
                                     unsigned int * values, double * proba);
 
// Des entiers longs non signés
struct randomGenerator_t * randomGenerator_createULong(int distribution,
						       unsigned long min,
						       unsigned long max);
// Des réels double précision
struct randomGenerator_t * randomGenerator_createDouble();

// Des réels double précision, avec une distribution exp de paramètre lambda
struct randomGenerator_t * randomGenerator_createDoubleExp(double lambda);

/* 
 * A double range [min .. max}
 */
struct randomGenerator_t * randomGenerator_createDoubleRange(double min,
							     double max);

struct randomGenerator_t * randomGenerator_createDoubleDiscrete(
                                     int nbValues,
                                     double * values);
 
struct randomGenerator_t * randomGenerator_createDoubleDiscreteProba(
                                     int nbValues,
                                     double * values,
                                     double * proba);
 
/*==========================================================================*/

// Use a (previously built) probe to re-run a sequence
struct randomGenerator_t * randomGenerator_createFromProbe(struct probe_t * p);

void randomGenerator_reset(struct randomGenerator_t * rg);

/*
 * Destructor
 */
void randomGenerator_delete(struct randomGenerator_t * rg);

/*==========================================================================*/
/*   Select distribution                                                    */
/*==========================================================================*/
/*
 * Choix de la distribution
 */

/*
 * Un nombre discret de probabilités
 */
void randomGenerator_setDistributionDiscrete(struct randomGenerator_t * rg,
					     int nb,
                                             double * proba);

/**
 * @brief Cr�ation d'une distribution uniforme construite depuis un fichier
 *
 * Le fichier doit �tre un fichier texte orient� ligne o� chaque ligne
 * contient un entier (la valeur) suivi d'un r�el (la probabilit� associ�e).
 */ 
void randomGenerator_setDistributionDiscreteFromFile(struct randomGenerator_t * rg,
						     char * fileName);


// Choix d'une loi uniforme
void randomGenerator_setDistributionUniform(struct randomGenerator_t * rg);

// Choix d'une loi exponentielle
void randomGenerator_setDistributionExp(struct randomGenerator_t * rg, double lambda);


//void randomGenerator_setUniformDistribution(struct randomGenerator_t * rg);

//void randomGenerator_setUniformMinMaxDouble(struct randomGenerator_t * rg, double min, double max);

/*
 * Change lambda
 */
void randomGenerator_setLambda(struct randomGenerator_t * rg, double lambda);


/*
 * Prepare for record values in order to replay on each reset
 */
void randomGenerator_recordThenReplay(struct randomGenerator_t * rg);

/*
 * Value generation
 */
unsigned int randomGenerator_getNextUInt(struct randomGenerator_t * rg);
double randomGenerator_getNextDouble(struct randomGenerator_t * rg);

/*
 * Obtention de certains paramètres. Il s'agit ici de valeurs
 * théoriques, pour obtenir leurs équivalents sur une série
 * d'expériences, on utilisera des sondes.
 */
double randomGenerator_getExpectation(struct randomGenerator_t * rg);

/*==========================================================================*/
/*   Probes                                                                 */ 
/*==========================================================================*/
/**
 * @brief Ajout d'une sonde sur les valeurs g�n�r�es
 */

void randomGenerator_addValueProbe(struct randomGenerator_t * rg,
				   struct probe_t * p);

#endif
