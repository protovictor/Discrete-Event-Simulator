/*
 * Programme de test des divers générateurs
 */
#include <stdlib.h>    // Malloc, NULL, exit, ...
#include <assert.h>
#include <strings.h>   // bzero, bcopy, ...
#include <stdio.h>     // printf, ...

#include <probe.h>
#include <date-generator.h>
#include <motsim.h>

/*----------------------------------------------------------------------*/
/*                                                                      */
/*----------------------------------------------------------------------*/
int main() {
   struct dateGenerator_t * dateGenExp;
   unsigned long n;
   double d = 0.0;
   double l = 3.0;

   /* Les sondes */
   struct probe_t     * iap; // Interarrivee de la source

   motSim_create(); // Pour la date courante !

   iap = probe_createExhaustive();

   dateGenExp = dateGenerator_createExp(l);
   dateGenerator_setInterArrivalProbe(dateGenExp, iap);

   for (n = 0 ; n < 10000000;n++){
      d = dateGenerator_nextDate(dateGenExp, d);
   }
   printf("Lambda = %f : InterArrival  %f (%d samples)\n", l, probe_mean(iap), probe_nbSamples(iap));

    
   return 1;
}
