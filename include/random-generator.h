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

/**
 * @brief Available types for the random values
 * @param rGTypeUInt Des nombres positifs entiers 
 * @param rGTypeULong Des nombres positifs entiers longs
 * @param rGTypeFloat Des nombres décimaux float
 * @param rGTypeDouble Des nombres décimaux double précision
 * @param rGTypeDoubleRange Des nombres double précision compris entre min et max
 * @param rGTypeUIntEnum Des nombres positifs entiers pour les rgs discrets
 * @param rGTypeDoubleEnum Des nombres décimaux double précision pour les rgs discrets
 * @param rGTypeUIntRange Des nombres entiers positifs compris entre min et max
 * @param rGTypeUIntConstant Des nombres entiers positifs constants
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


/**
 * @brief Available distributions
 * @param rGDistNoDist Pas de distribution définie
 * @param rGDistUniform Distribution uniforme
 * @param rGDistExponential Distribution exponentielle
 * @param rGDistDiscrete Distribution discrète
 * @param rGDistITS Distribution ITS
 * @param rGDistTruncLogNorm Distribution log-normale tronquée
 * @param rGDistTruncPareto Distribution Pareto tronquée
 */
#define rGDistNoDist       0
#define rGDistUniform      1
#define rGDistExponential  2
#define rGDistDiscrete     3
#define rGDistITS          4
#define rGDistTruncLogNorm 5
#define rGDistTruncPareto  6

/**
 * @brief Définition de la distribution par défaut qui est la distribution uniforme
*/
#define rGDistDefault rGDistUniform
/**
 * @brief Entropy sources
 * @param rGSourceErand48 Source d'entrotpie de type Errand48
 * @param rGSourceReplay Source d'entrotpie de type rGSourceReplay
 * @param rgSourceUrandom Source d'entrotpie de type rgSourceUrandom
 */
#define rGSourceErand48 1
#define rGSourceReplay  2
#define rgSourceUrandom 3

/**
 * @brief Source d'entropie par défaut Errand48
*/
#define rgSourceDefault rGSourceErand48

/*==========================================================================*/
/*  Creators                                                                */
/*==========================================================================*/

/**
 * @brief Creators without distribution
 * @return On obtient un générateur aléatoire sans distribution
 */
// Des entiers non signÃ©s quelconques
struct randomGenerator_t * randomGenerator_createUInt();

/**
 * @brief Créateur d'un RG (sans distribution prédifinie)
 * avec des entiers non signés entre min et max  (inclus)
 * @param min le min de la distribution
 * @param max le max de la distribution
*/
struct randomGenerator_t * randomGenerator_createUIntRange(unsigned int min,
						      unsigned int max);

/**
 * @brief Créateur d'un RG (sans distribution prédifinie)
 * avec des entiers non signés tous égaux!
 * @param v Distribution de dirac
*/
struct randomGenerator_t * randomGenerator_createUIntConstant(unsigned int v);

/**
 * @brief Créateur d'un RG (sans distribution prédifinie)
 * avec des entiers non signés listés
 * @param nbValues nombre de valeur de la disribution
 * @param values valeurs de la distribution!???
*/
struct randomGenerator_t * randomGenerator_createUIntDiscrete(int nbValues,
							      unsigned int * values);
/**
 * @brief Créateur d'un RG (sans distribution prédifinie)
 * avec des réels double précision
*/
struct randomGenerator_t * randomGenerator_createDouble();

/*
 * Creators with a pre defined distribution
 */

/**
 * @brief Création d'une distribution d'entier discrète d'après un fichier
 * @param fileName nom du fichier contenant la distribution à lire
 */
struct randomGenerator_t * randomGenerator_createUIntDiscreteFromFile(char * fileName);


/**
 * @brief Création d'une distribution d'entier discrète
 * @param nbValues  Le nombre de valeurs possibles
 * @param values liste de ces valeurs
 * @param xmin la liste de leurs probabilité
 */
struct randomGenerator_t * randomGenerator_createUIntDiscreteProba(int nbValues,
                                     unsigned int * values, double * proba);
 
/**
 * @brief create ULong (distribution avec des entiers longs)
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
 * @param min double minimum du segment
 * @param max double maximum du segment
 */
struct randomGenerator_t * randomGenerator_createDoubleRange(double min,
							     double max);

/**
 * @brief Generate a double discrete
 * @param nbValues int nombre de valeurs de la distribution
 * @param values *double les valeurs de la distribution!??
 */
struct randomGenerator_t * randomGenerator_createDoubleDiscrete(
                                     int nbValues,
                                     double * values);
/**
 * @brief Generate a double discrete proba
 * @param nbValues int nombres de valeurs de la distribtuion
 * @param values *double les valeurs de la distribution
 * @param proba *double les proba de chaque point de la distribution
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

/**
 * @brief Un nombre discret de probabilités
 * @param rg RG dont on doit configurer la distribution
 * @param nb nombre d'éléments de la distribution
 * @param proba la loi de la distribution
 */
void randomGenerator_setDistributionDiscrete(struct randomGenerator_t * rg,
					     int nb, double * proba);

/**
 * @brief Création d'une distribution uniforme construite depuis un fichier
 *
 * Le fichier doit être un fichier texte orienté ligne où chaque ligne
 * contient un entier (la valeur) suivi d'un réel (la probabilité associée).
 * @param rg RG dont on doit configurer la distribution
 * @param fileName nom du fichier contenant les infos sur la distribution
 */ 
void randomGenerator_setDistributionDiscreteFromFile(struct randomGenerator_t * rg,
						     char * fileName);


/**
 * @brief Choix d'une loi uniforme
 * @param rg RG dont on doit configurer la distribution
 */
void randomGenerator_setDistributionUniform(struct randomGenerator_t * rg);

/**
 * @brief Choix d'une loi exponentielle
 * @param rg RG dont on doit configurer la distribution
 * @param lambda paramètre de la distribution exponentielle
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
 * @param x paramètre de la CDF
 * @param lambda paramètre de la distribution exponentielle
 */
double randomGenerator_expDistQ(double x, double lambda);

/**
 * @brief Inverse of CDF for pareto distribution
 * @param x paramètre de la CDF
 * @param alpha paramètre de de la distribution pareto
 * @param xmin paramètre de la distribution pareto
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
/**
 * @brief Création d'un RG de distribution Pareto tronquée
 * @param alpha paramètre de la distribution pareto
 * @param xmin paramètre de la distribution pareto
 * @param plafond paramètre de la distribution de pareto
*/
struct randomGenerator_t * randomGenerator_createDoubleRangeTruncPareto(double alpha, double xmin, double plafond);

//Calqué sur randomGenerator_setDistributionExp
/**
 * @brief Définir une distribution Pareto tronquée pour un RG
 * @param rg RG dont on doit définir la distribution
 * @param alpha paramètre de la distribution pareto
 * @param xmin paramètre de la distribution pareto
 * @param plafond paramètre de la distribution de pareto
*/
void randomGenerator_setDistributionTruncPareto(struct randomGenerator_t * rg,
					     double alpha,
                                             double xmin, double plafond);

//Calqué sur randomGenerator_exponentialInit
/**
 * @brief Initialisation de la distribution Pareto tronquée pour une rg
 * @param rg RG dont on doit initialisé la distribution pareto tronquée
 * @param alpha paramètre de la distribution pareto
 * @param xmin paramètre de la distribution pareto
 * @param plafond paramètre de la distribution de pareto
*/
void randomGenerator_TruncParetoInit(struct randomGenerator_t * rg, double alpha, double xmin, double plafond);

// Calqué sur randomGenerator_exponentialGetNext
/**
 * @brief Value generation : double issue d'une distribution de pareto
 * @param rg random generator
 */
double randomGenerator_TruncParetoGetNext(struct randomGenerator_t * rg);

//Calqué sur randomGenerator_setLambda.
/**
 * @brief Change alpha , xmin and plafond for a pareto troncated distribution
 * @param rg random generator
 * @param alpha paramètre de la distribution pareto
 * @param xmin paramètre de la distribution pareto
 * @param plafond paramètre de la distribution de pareto
*/
void randomGenerator_setAlphaXminPlafond(struct randomGenerator_t * rg, double alpha, double xmin, double plafond);

//Les 5 fonctions suivantes sont calquées sur les 5 ci-dessus
/**
 * @brief Création d'un RG de distribution log normale tronquée
 * @param mu paramètre de la distribution log normale tronquée
 * @param sigma paramètre de la distribution log normale tronquée
 * @param plafond paramètre de la distribution de log normale tronquée
*/
struct randomGenerator_t * randomGenerator_createDoubleRangeTruncLogNorm(double mu, double sigma, double plafond);

/**
 * @brief Définir une distribution log normale tronquée pour un RG
 * @param rg RG dont on doit définir la distribution
 * @param mu paramètre de la distribution log normale tronquée
 * @param sigma paramètre de la distribution log normale tronquée
 * @param plafond paramètre de la distribution de log normale tronquée
*/
void randomGenerator_setDistributionTruncLogNorm(struct randomGenerator_t * rg,
					     double mu,
                                             double sigma, double plafond);

/**
 * @brief Initialisation de la distribution log normale tronquée pour une rg
 * @param rg RG dont on doit initialisé la distribution log normale tronquée
 * @param mu paramètre de la distribution log normale
 * @param sigma paramètre de la distribution log normale
 * @param plafond paramètre de la distribution de log normale
*/
void randomGenerator_TruncLogNormInit(struct randomGenerator_t * rg, double mu, double sigma, double plafond);

/**
 * @brief Value generation : double issue d'une distribution log normale tronquée
 * @param rg random generator
 */
double randomGenerator_TruncLogGetNext(struct randomGenerator_t * rg);

/**
 * @brief Change mu , sigma and plafond for a log normal distribution
 * @param rg random generator
 * @param mu paramètre de la distribution log normale
 * @param sigma paramètre de la distribution log normale
 * @param plafond paramètre de la distribution de log normale
*/
void randomGenerator_setMuSigmaPlafond(struct randomGenerator_t * rg, double mu, double sigma, double plafond);


//======================================
#endif
