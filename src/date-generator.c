/** @file date-generator.c
 *  @brief Implantation des générateurs de dates
 *
 */
#include <stdio.h>     // printf
#include <stdlib.h>    // Malloc, NULL, exit...
#include <math.h>      // log

#include <motsim.h>
#include <probe.h>
#include <random-generator.h>  

/**
 * @brief Implantation des générateurs de dates.
 *
 * Largement fondé sur les générateurs de nombres aléatoires.
 * Toutes les dates sont toujours exprimées en secondes depuis le début de
 * la simulation.
 */
struct dateGenerator_t {
   /** Le generateur aleatoire sur lequel on se fonde. C'est
       concrètement lui qui va générer les dates successives.*/
   struct randomGenerator_t *randGen; 

   /** Une sonde sur les inter arrivees. Elle permettra par exemple
       de vérifier qu'on est conforme à ce que l'on souhaite. */
   struct probe_t * interArrivalProbe;

  //   double (* nextDate)(struct dateGenerator_t * dateGen, double currentTime);  //   void           * data;
};

/*-------------------------------------------------------------------------*/
/*   Les constructeurs                                                     */
/*-------------------------------------------------------------------------*/
void dateGenerator_setRandomGenerator(struct dateGenerator_t * dateGen,
				      struct randomGenerator_t * randGen);
/**
 * @brief Creation of a date generator
 * @result a struct dateGenerator_t * 
 *
 * The created dateGenerator is unusable for now. It needs to be
 * associated to a random generator
 */
struct dateGenerator_t * dateGenerator_create()
{
  struct dateGenerator_t * result = (struct dateGenerator_t * )
                  sim_malloc(sizeof(struct dateGenerator_t));
  result->interArrivalProbe = NULL;

  result->randGen = NULL;

  return result;
}

/*
 * Création d'une loi avec interarrivé exponentielle
 */
struct dateGenerator_t * dateGenerator_createExp(double lambda)
{
  struct dateGenerator_t * result = dateGenerator_create();

  dateGenerator_setRandomGenerator(result, randomGenerator_createDoubleExp(lambda));

  return result;
}

/*
 * Création d'une loi avec interarrivé constante
 */
struct dateGenerator_t * dateGenerator_createPeriodic(double period)
{
  struct dateGenerator_t * result = dateGenerator_create();
  double un = 1.0;

  dateGenerator_setRandomGenerator(result, randomGenerator_createDoubleDiscreteProba(1, &period, &un));

  return result;
}

/*-------------------------------------------------------------------------*/
/*   Les fonctions générales                                               */
/*-------------------------------------------------------------------------*/
/**
 * @brief Choix du générateur aléatoire des durées entre dates
 *
 * @param dateGen le générateur à modifier
 * @param randGen le générateur de date à affecter
 */
void dateGenerator_setRandomGenerator(struct dateGenerator_t * dateGen,
				      struct randomGenerator_t * randGen)
{
   dateGen->randGen = randGen;
}


/** @brief Obtention de la prochaine date
 *
 *  @param dateGen le générateur à utiliser
 *  @param currentTime la date actuelle
 */
double dateGenerator_nextDate(struct dateGenerator_t * dateGen, double currentTime)
{
   double result =  randomGenerator_getNextDouble(dateGen->randGen);

   if (dateGen->interArrivalProbe){
      probe_sample(dateGen->interArrivalProbe, result);
      printf_debug(DEBUG_GENE, " Mean = %6.3f\n", probe_mean(dateGen->interArrivalProbe));
   }

   return currentTime + result ;
  //   return dateGen->nextDate(dateGen, currentTime);
}

/**
 * @brief Insertion d'une sonde sur les inter-arrivees.
 * 
 * @param dateGen le générateur de date sur lequel greffer la sonde
 * @param probe la sonde à y appliquer
 */
void dateGenerator_addInterArrivalProbe(struct dateGenerator_t * dateGen, struct probe_t * probe)
{
   dateGen->interArrivalProbe = probe_chain(probe, dateGen->interArrivalProbe);
}

/*-------------------------------------------------------------------------*/
/*    Source de dates avec une interarrivee exponentielle.                 */
/*-------------------------------------------------------------------------*/
/*
double loi_expo(struct dateGenerator_t * dateGen, double currentTime)
{
   double alea, result, * data;

   alea = random() / (RAND_MAX + 1.0);

   data = (double*)dateGen->data;

   //   printf("alea = %f, data = %p, data[0] = %f\n", alea, data, data[0]);
   //printf("log(alea) = %f\n", log(alea));
    result = - log(alea) / data[0];

   printf_debug(DEBUG_GENE, " alea = %6.3f, lambda = %6.3f, result = %6.3f\n", alea, ((double*)dateGen->data)[0], result);
   if (dateGen->interArrivalProbe){
      probe_sample(dateGen->interArrivalProbe, result);
      printf_debug(DEBUG_GENE, " Mean = %6.3f\n", probe_mean(dateGen->interArrivalProbe));
   }

   return result + currentTime;
}
*/

/*
 * Modification du paramètre lambda
 */
void dateGenerator_setLambda(struct dateGenerator_t * dateGen, double lambda)
{
   randomGenerator_setLambda(dateGen->randGen, lambda);
}

/*
 * Prepare for record values in order to replay on each reset
 */
void dateGenerator_recordThenReplay(struct dateGenerator_t *  d){
  randomGenerator_recordThenReplay(d->randGen);
};
