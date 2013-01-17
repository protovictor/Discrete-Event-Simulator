/*
 * Test des générateurs discrets
 */
#include <random-generator.h>
#include <probe.h>

#include <stdio.h>     // printf, ...

#define NBECH 1000000

/*----------------------------------------------------------------------*/
/*                                                                      */
/*----------------------------------------------------------------------*/
int main() {
   struct randomGenerator_t * rg;
   struct probe_t           * rp; 
   int n, v;

   unsigned int facesDe[] = {1, 2, 3, 4, 5, 6};
   double probaDe[] = {1.0/6.0, 1.0/6.0, 1.0/6.0, 1.0/6.0, 1.0/6.0, 1.0/6.0};

   motSim_create(); // Les sondes datent les échantillons

   rp = probe_createExhaustive();

   rg = randomGenerator_createUIntDiscrete(6, facesDe, probaDe);

   for (n = 0 ; n < NBECH;n++){
      v = randomGenerator_getNextUInt(rg);
      //      printf("[%d] ", v);
      probe_sample(rp, (double)v);
   }
   printf("\n");

   // Résultats pour un tirage de dé non pipé
   printf("%d lancers de dé à six faces (non pipé) :\n", NBECH);
   printf("Moyenne    = %f\n", probe_mean(rp));
   printf("Variance   = %f\n", probe_variance(rp));
   printf("Ecart type = %f\n", probe_stdDev(rp));

   probe_delete(rp);
   randomGenerator_delete(rg);
}
