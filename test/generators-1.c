/*
 * Programme de test des divers générateurs
 *
 * generators-1
 *
 */
#include <stdlib.h>    // Malloc, NULL, exit, ...
#include <assert.h>
#include <strings.h>   // bzero, bcopy, ...
#include <stdio.h>     // printf, ...
#include <math.h>      // sqrt

#include <random-generator.h>
#include <probe.h>
#include <gnuplot.h>
#include <motsim.h>

#define NBECH 10000000
#define MIN 0.0
#define MAX 1.0


/*----------------------------------------------------------------------*/
/*                                                                      */
/*----------------------------------------------------------------------*/
int main() {
   struct randomGenerator_t * rg;
   struct probe_t           * rp; 
   int                        n, res;
   double                     lambda = 3.0;

   motSim_create(); // Les sondes datent les échantillons

/*----------------------------------------------------------------------*/
/*     Vérification des moments d'une loi uniforme                      */
/*----------------------------------------------------------------------*/
   rp = probe_createExhaustive();

   rg = randomGenerator_createDoubleRange(MIN, MAX);
   randomGenerator_setDistributionUniform(rg);printf("Prout\n");

   for (n = 0 ; n < NBECH;n++){
      probe_sample(rp, randomGenerator_getNextDouble(rg));
   }

   printf("Moyenne    = %f (%f)\n", probe_mean(rp), (MAX-MIN)/2.0);
   printf("Variance   = %f (%f)\n", probe_variance(rp), (MAX-MIN)*(MAX-MIN)/12.0);
   printf("Ecart type = %f (%f)\n", probe_stdDev(rp), (MAX-MIN)/sqrt(12.0));

   res = (   (fabs(probe_mean(rp) - ((MAX-MIN)/2.0))/((MAX-MIN)/2.) < 0.05)
	  && (fabs(probe_variance(rp) - (MAX-MIN)*(MAX-MIN)/12.0) / ((MAX-MIN)*(MAX-MIN)/12.0)  < 0.05)
          && (fabs(probe_stdDev(rp) - (MAX-MIN)/sqrt(12.0))/((MAX-MIN)/sqrt(12.0))));
   probe_delete(rp);
   randomGenerator_delete(rg);

/*----------------------------------------------------------------------*/
/*     Vérification des moments d'une loi exponentielle                 */
/*----------------------------------------------------------------------*/
   rp = probe_createExhaustive();

   rg = randomGenerator_createDoubleExp(lambda);

   for (n = 0 ; n < NBECH;n++){
     probe_sample(rp, randomGenerator_getNextDouble(rg));
   }

   printf("Moyenne    = %f (%f)\n", probe_mean(rp), 1.0/lambda);
   printf("Variance   = %f (%f)\n", probe_variance(rp), 1.0/(lambda*lambda));
   printf("Ecart type = %f (%f)\n", probe_stdDev(rp), 1.0/lambda);

   probe_delete(rp);
   randomGenerator_delete(rg);

   res = res 
     && (fabs(probe_mean(rp) -  1.0/lambda)/( 1.0/lambda))
      && (fabs(probe_variance(rp) -  1.0/(lambda*lambda))/( 1.0/(lambda*lambda)))
      && (fabs(probe_stdDev(rp) -  1.0/lambda)/( 1.0/lambda));

   return !res;
}
