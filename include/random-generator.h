/*
 * Les générateurs de nombres aléatoires.
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

/*
 * Available distributions
 */
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
/*
 * Creators
 */

struct randomGenerator_t * randomGenerator_createULong(int distribution,
						       unsigned long min,
						       unsigned long max);

struct randomGenerator_t * randomGenerator_createUInt(int distribution,
                                                      unsigned int min,
						      unsigned int max);

/*
 * Création d'un générateur aléatoire de nombres entiers parmis un
 * ensemble discret.
 */
struct randomGenerator_t * randomGenerator_createUIntDiscrete(int nbValues,
							      unsigned int * values);
 

/*
 * Le nombre de valeurs possibles est passé en paramètre ainsi que la
 * liste de ces valeurs puis la liste de leurs probabilité.
 */
struct randomGenerator_t * randomGenerator_createUIntDiscreteProba(int nbValues,
                                     unsigned int * values, double * proba);
 

/*
 * R+, default distribution : exponential
 */
struct randomGenerator_t * randomGenerator_createDouble(double lambda);

/*
 * Change lambda
 */
void randomGenerator_setLambda(struct randomGenerator_t * rg, double lambda);

/* 
 * A double range [min .. max}, default distribution : uniform
 */
struct randomGenerator_t * randomGenerator_createDoubleRange(double min,
							     double max);

struct randomGenerator_t * randomGenerator_createDoubleDiscrete(int nbValues,
                                     double * values);
 
struct randomGenerator_t * randomGenerator_createDoubleDiscreteProba(int nbValues,
                                     double * values, double * proba);
 
// Use a (previously built) probe to re-run a sequence
struct randomGenerator_t * randomGenerator_createFromProbe(struct probe_t * p);

void randomGenerator_reset(struct randomGenerator_t * rg);

/*
 * Destructor
 */
void randomGenerator_delete(struct randomGenerator_t * rg);

/*
 * Change distribution/parameters
 */
void randomGenerator_setUniformDistribution(struct randomGenerator_t * rg);
void randomGenerator_setUniformMinMaxDouble(struct randomGenerator_t * rg, double min, double max);

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

/*
 * Choix de la distribution
 */

// Un nombre discret de probabilités
void randomGenerator_setDistributionDiscrete(struct randomGenerator_t * rg,
					     int nb,
                                             double * proba);
// Choix d'une loi uniforme
void randomGenerator_setDistributionUniform(struct randomGenerator_t * rg);

#endif
