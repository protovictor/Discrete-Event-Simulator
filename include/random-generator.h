/**
 * @file random-generator.h
 * @brief Les générateurs de nombres aléatoires.
 *
 * Un gÃ©nÃ©rateur est caractÃ©risÃ© par plusieurs propriÃ©tÃ©s
 *
 * - Le type des valeurs gÃ©nÃ©rÃ©es.
 * - La distribution des probabilitÃ©s
 *    . Les paramÃ¨tres de cette distribution : min, max, moyenne, ...
 * - La source d'alÃ©a. Certaines permettent un rejeu (rand unix)
 *   d'autres sont "vraiment alÃ©atoires" (/dev/random)
 *    . Les paramÃ¨tres de cette source (seed, ...)
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
#define rGDistITS          4
#define rGDistTruncLogNorm 5
#define rGDistTruncPareto  6

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

/*
 *  Creators without distribution
 */
// Des entiers non signÃ©s quelconques
struct randomGenerator_t * randomGenerator_createUInt();

// Des entiers non signÃ©s entre min et max  (inclus)
struct randomGenerator_t * randomGenerator_createUIntRange(unsigned int min,
						      unsigned int max);

// Des entiers non signÃ©s tous égaux !
struct randomGenerator_t * randomGenerator_createUIntConstant(unsigned int v);

// Des entiers non signÃ©s listÃ©s
struct randomGenerator_t * randomGenerator_createUIntDiscrete(int nbValues,
							      unsigned int * values);
// Des rÃ©els double prÃ©cision
struct randomGenerator_t * randomGenerator_createDouble();

/*
 * Creators with a pre defined distribution
 */

/**
 * @brief Création d'une distribution d'après un fichier
 */
struct randomGenerator_t * randomGenerator_createUIntDiscreteFromFile(char * fileName);


/**
 * @brief 
 * @param nbValues  Le nombre de valeurs possibles
 * @param values liste de ces valeurs
 * @param xmin la liste de leurs probabilité
 */
struct randomGenerator_t * randomGenerator_createUIntDiscreteProba(int nbValues,
                                     unsigned int * values, double * proba);
 
/**
 * @brief create ULong
 * @param distribution numéro de la distribution
 * @param min minimum entier long non signé
 * @param max maximum entier long non signé
 */
struct randomGenerator_t * randomGenerator_createULong(int distribution,
						       unsigned long min,
						       unsigned long max);

/**
 * @brief Génère une distribution exponentielle (des réels double précision)
 * @param lambda paramètre lambda (double) de la loi exponentielle
 */
struct randomGenerator_t * randomGenerator_createDoubleExp(double lambda);

///  ****************   A FAIRE **************** 
/**
 * @brief Génère une distribution Truncated Lognormal (des réels double précision)
 * @param sigma paramètre sigma (double) de la loi Truncated Lognormal
 * @param mu paramètre mu (double) de la loi Truncated Lognormal
 */

struct randomGenerator_t * randomGenerator_createDoubleTruncLogNorm(double sigma, double mu);


///  ****************   A FAIRE **************** 
/**
 * @brief Génère une distribution Truncated Pareto (des réels double précision)
 * @param alpha paramètre sigma (double) de la loi Truncated Pareto
 * @param k paramètre k (int) de la loi Truncated Pareto
 * @param m paramètre m (int) de la loi Truncated Pareto
 */
//struct randomGenerator_t * randomGenerator_createDoubleTruncLogNorm(double alpha, int k, int m);

/**
 * @brief Generate a double range [min .. max]
 * @param min double
 * @param max double
 */
struct randomGenerator_t * randomGenerator_createDoubleRange(double min,
							     double max);

/**
 * @brief Generate a double discrete
 * @param nbValues int
 * @param values *double
 */
struct randomGenerator_t * randomGenerator_createDoubleDiscrete(
                                     int nbValues,
                                     double * values);
/**
 * @brief Generate a double discrete proba
 * @param nbValues int
 * @param values *double
 * @param proba *double
 */
struct randomGenerator_t * randomGenerator_createDoubleDiscreteProba(
                                     int nbValues,
                                     double * values,
                                     double * proba);
 
/*==========================================================================*/
/**
 * @brief  Use a (previously built) probe to re-run a sequence
 * @param p *probe_t previously built probe
 */
struct randomGenerator_t * randomGenerator_createFromProbe(struct probe_t * p);

/**
 * @brief Reset a random generator
 * @param rg random generator which has to be reseted
 */
void randomGenerator_reset(struct randomGenerator_t * rg);

/**
 * @brief  Destructor, destruct a random generator
 * @param rg random generator which has to be destructed
 */
void randomGenerator_delete(struct randomGenerator_t * rg);

/*==========================================================================*/
/*   Select distribution                                                    */
/*==========================================================================*/
/*
 * Choix de la distribution
 */

/*
 * Un nombre discret de probabilitÃ©s
 */
void randomGenerator_setDistributionDiscrete(struct randomGenerator_t * rg,
					     int nb, double * proba);

/**
 * @brief Création d'une distribution uniforme construite depuis un fichier
 *
 * Le fichier doit être un fichier texte orienté ligne où chaque ligne
 * contient un entier (la valeur) suivi d'un réel (la probabilité associée).
 */ 
void randomGenerator_setDistributionDiscreteFromFile(struct randomGenerator_t * rg,
						     char * fileName);


/**
 * @brief Choix d'une loi uniforme
 */
void randomGenerator_setDistributionUniform(struct randomGenerator_t * rg);

/**
 * @brief Choix d'une loi exponentielle
 */
void randomGenerator_setDistributionExp(struct randomGenerator_t * rg, double lambda);

///  ****************   PAS A FAIRE POUR L INSTANT ( pas essentiel )   **************** 
//
/**
 * @brief Choix d'une loi TruncLogNorm 
 */ 
//void randomGenerator_setDistributionTruncLogNorm(struct randomGenerator_t * rg, double sigma, double mu);
//NOTE DE BENJAMIN : il avait dit "pas essentiel" mais je l'ai fait... (cf ci-dessous)

///  ****************   PAS A FAIRE POUR L INSTANT ( pas essentiel )   **************** 
//
/**
 * @brief Choix d'une loi Trunc Pareto
 */ 
//void randomGenerator_setDistributionTruncPareto(struct randomGenerator_t * rg, double alpha, int k, int m);
//NOTE DE BENJAMIN : idem.

/**
 * @brief Set a pareto distribution
 * @param rg The random generator to be modified
 * @param alpha The shape of the pareto distribution
 * @param xmin The scale of the pareto distribution
 */
#define randomGenerator_setDistributionPareto(rg, alpha, xmin) \
   randomGenerator_setQuantile2Param(rg, randomGenerator_paretoDistQ, alpha, xmin)

/**
 * @brief Define a distribution by its quantile function for inverse
 * transform sampling
 * @param rg The random generator
 * @param q The inverse cumulative density (quantile) function
 * @param p1 The single parameter of the quantile function
 */
void randomGenerator_setQuantile1Param(struct randomGenerator_t * rg,
				       double (*q)(double x, double p),
				       double p);

/**
 * @brief Define a distribution by its quantile function for inverse
 * transform sampling
 * @param rg The random generator
 * @param q The inverse cumulative density (quantile) function
 * @param p1 The first parameter of the quantile function
 * @param p2 The second parameter of the quantile function
 */
void randomGenerator_setQuantile2Param(struct randomGenerator_t * rg,
				       double (*q)(double x, double p1, double p2),
				       double p1, double p2);
/**
 * @brief Inverse of CDF for exponential distribution
 */
double randomGenerator_expDistQ(double x, double lambda);

/**
 * @brief Inverse of CDF for pareto distribution
 */
double randomGenerator_paretoDistQ(double x, double alpha, double xmin);

/**
 * @brief Change lambda
 * @param rg random generator
 * @param lambda lambda which has to be change
 */
void randomGenerator_setLambda(struct randomGenerator_t * rg, double lambda);

/**
 * @brief Prepare for record values in order to replay on each reset
 * @param rg random generator
 */
void randomGenerator_recordThenReplay(struct randomGenerator_t * rg);

/**
 * @brief Value generation : unsigned int
 * @param rg random generator
 */
unsigned int randomGenerator_getNextUInt(struct randomGenerator_t * rg);

/**
 * @brief Value generation : double
 * @param rg random generator
 */
double randomGenerator_getNextDouble(struct randomGenerator_t * rg);

/**
 * @brief Obtention de certains paramètres. Il s'agit ici de valeurs
 * théoriques, pour obtenir leurs équivalents sur une série
 * d'expèriences, on utilisera des sondes.
 * @param rg random generator
 */
double randomGenerator_getExpectation(struct randomGenerator_t * rg);

/*==========================================================================*/
/*   Probes                                                                 */ 
/*==========================================================================*/
/**
 * @brief Ajout d'une sonde sur les valeurs générées
 * @param rg random generator
 * @param p probe
 */
void randomGenerator_addValueProbe(struct randomGenerator_t * rg,
				   struct probe_t * p);


//======================================
//Nouveautés made in Benj ;)

/*Inspirez-vous de createDoubleExp, setDistributionExp, etc... et faites la doc !  
 */

//Calqué sur randomGenerator_createDoubleExp
struct randomGenerator_t * randomGenerator_createDoubleRangeTruncPareto(double alpha, double xmin, double plafond);
//Calqué sur randomGenerator_setDistributionExp
void randomGenerator_setDistributionTruncPareto(struct randomGenerator_t * rg,
					     double alpha,
                                             double xmin, double plafond);
//Calqué sur randomGenerator_exponentialInit
void randomGenerator_TruncParetoInit(struct randomGenerator_t * rg, double alpha, double xmin, double plafond);
// Calqué sur randomGenerator_exponentialGetNext
double randomGenerator_TruncParetoGetNext(struct randomGenerator_t * rg);
//Calqué sur randomGenerator_setLambda.
void randomGenerator_setAlphaXminPlafond(struct randomGenerator_t * rg, double alpha, double xmin, double plafond);

//Les 5 fonctions suivantes sont calquées sur les 5 ci-dessus
struct randomGenerator_t * randomGenerator_createDoubleRangeTruncLogNorm(double mu, double sigma, double plafond);
void randomGenerator_setDistributionTruncLogNorm(struct randomGenerator_t * rg,
					     double mu,
                                             double sigma, double plafond);
void randomGenerator_TruncLogNormInit(struct randomGenerator_t * rg, double mu, double sigma, double plafond);
double randomGenerator_TruncLogGetNext(struct randomGenerator_t * rg);
void randomGenerator_setMuSigmaPlafond(struct randomGenerator_t * rg, double mu, double sigma, double plafond);








#endif
