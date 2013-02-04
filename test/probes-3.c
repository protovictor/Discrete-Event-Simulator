/*
 *    Quelques tests simples sur les sondes
 *
 *    probes-3 : Comparaison de résultats entre des sondes exhaustives
 *    et par moyenne
 */
#include <float.h>
#include <stdio.h>
#include <stdlib.h>

#include <motsim.h>
#include <probe.h>

#define TPMAX 2000
#define NBECH 2000

int main()
{
   struct probe_t * mp, *ep;
   int  e, n;
   unsigned long l;

   double  data;

   // Initialisation du simulateur
   motSim_create();

   // Création d'une sonde exhaustive
   ep = probe_createExhaustive();

   // Création d'une sonde en moyenne
   mp = probe_createMean();

   // On les peuple avec NBECH echantillons par unitée
   for (n = -TPMAX; n < TPMAX ; n++){
      for (e = 0 ; e < NBECH; e++) {
         data = (double)n + ((double)rand()/(double)RAND_MAX);
         probe_sample(ep, data);
         probe_sample(mp, data);
      }
   };

   // Comparaison des résultats
   printf("Nombre d'echantillons : %ld / %ld\n", probe_nbSamples(ep), probe_nbSamples(mp));
   printf("Moyenne : %f / %f\n", probe_mean(ep), probe_mean(mp));
   printf("Min : %f / %f\n", probe_min(ep), probe_min(mp));
   printf("Max : %f / %f\n", probe_max(ep), probe_max(mp));
}
