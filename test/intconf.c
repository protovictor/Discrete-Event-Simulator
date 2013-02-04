/*
 * Test des mesures d'intervalle de confiance
 */
#include <random-generator.h>
#include <probe.h>

#include <stdio.h>     // printf, ...

/*----------------------------------------------------------------------*/
/*                                                                      */
/*----------------------------------------------------------------------*/
int main() {
   struct randomGenerator_t * rg;
   struct probe_t           * rp; 
   int n, nbEch;

   unsigned int facesDe[] = {1, 2, 3, 4, 5, 6};
   double probaDe[] = {1.0/6.0, 1.0/6.0, 1.0/6.0, 1.0/6.0, 1.0/6.0, 1.0/6.0};

   motSim_create(); // Les sondes datent les échantillons

   rp = probe_createExhaustive();

   rg = randomGenerator_createUIntDiscrete(6, facesDe, probaDe);

   // Sur une sonde exhaustive
   for (nbEch = 10485760; nbEch >= 10; nbEch /= 4) {
      printf("%d lancers de dé à six faces (non pipé) :\n", nbEch);
      for (n = 0 ; n < nbEch;n++){
         probe_sample(rp, (double)randomGenerator_getNextUInt(rg));
      }

      // Résultats pour un tirage de dé non pipé
      printf("Moyenne    = %f\n", probe_mean(rp));
      printf("Ecart type = %f\n", probe_stdDev(rp));
      printf("Intervalle = +/- %f\n", probe_demiIntervalleConfiance5pc(rp));
      probe_reset(rp);
   }

   probe_delete(rp);
   randomGenerator_delete(rg);
}
