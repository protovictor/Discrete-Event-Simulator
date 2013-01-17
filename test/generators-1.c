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

void tracer(struct probe_t * pr, char * name, int nbBar)
{
   struct probe_t   * gb;
   struct gnuplot_t * gp;

   gb = probe_createGraphBar(probe_min(pr), probe_max(pr), nbBar);
   probe_exhaustiveToGraphBar(pr, gb);
   probe_setName(gb, name);

   gp = gnuplot_create();
   gnuplot_setYRange(gp, 0.0, 1.1*(double)probe_graphBarGetMaxValue(gb));
   gnuplot_setXRange(gp, probe_min(gb), probe_max(gb)/2.0);
   gnuplot_displayProbe(gp, WITH_BOXES, gb);
}

/*----------------------------------------------------------------------*/
/*                                                                      */
/*----------------------------------------------------------------------*/
int main() {
   struct randomGenerator_t * rg;
   struct probe_t           * rp; 
   int n;
   double                     lambda = 3.0;

   motSim_create(); // Les sondes datent les échantillons

/*----------------------------------------------------------------------*/
/*     Vérification des moments d'une loi uniforme                      */
/*----------------------------------------------------------------------*/
   rp = probe_createExhaustive();

   rg = randomGenerator_createDoubleRange(MIN, MAX);

   for (n = 0 ; n < NBECH;n++){
      probe_sample(rp, randomGenerator_getNextDouble(rg));
   }

   printf("Moyenne    = %f (%f)\n", probe_mean(rp), (MAX-MIN)/2.0);
   printf("Variance   = %f (%f)\n", probe_variance(rp), (MAX-MIN)*(MAX-MIN)/12.0);
   printf("Ecart type = %f (%f)\n", probe_stdDev(rp), (MAX-MIN)/sqrt(12.0));

   tracer(rp, "Loi uniforme", 100);

   probe_delete(rp);
   randomGenerator_delete(rg);

/*----------------------------------------------------------------------*/
/*     Vérification des moments d'une loi exponentielle                 */
/*----------------------------------------------------------------------*/
   rp = probe_createExhaustive();

   rg = randomGenerator_createDouble(lambda);

   for (n = 0 ; n < NBECH;n++){
     probe_sample(rp, randomGenerator_getNextDouble(rg));
   }

   printf("Moyenne    = %f (%f)\n", probe_mean(rp), 1.0/lambda);
   printf("Variance   = %f (%f)\n", probe_variance(rp), 1.0/(lambda*lambda));
   printf("Ecart type = %f (%f)\n", probe_stdDev(rp), 1.0/lambda);

   tracer(rp, "Loi exponentielle", 100);

   probe_delete(rp);
   randomGenerator_delete(rg);

   printf("*** ^C pour finir ;-)\n");
   pause();


   return 1;
}
