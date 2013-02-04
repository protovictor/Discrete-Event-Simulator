/*
 *    Quelques tests simples sur les sondes
 *
 *    probes-1 : quelques tests élémentaires sur les sondes
 *    exhaustives
 */
#include <float.h>
#include <stdio.h>
#include <stdlib.h>

#include <motsim.h>
#include <probe.h>

#define NBTEST 25

int testerExhaustive(unsigned long nbEl)
{
   int result = 0;
   struct probe_t  *ep;
   unsigned long l;
   double meanE, meanGB, data;

   // Création d'une sonde exhaustive
   ep = probe_createExhaustive();

   //   printf("[PROBE-1] Insertion de %ld elements\n", nbEl);
   // On la peuple avec des valeurs séquentielles
   for (l = 0; l < nbEl; l++){
      probe_sample(ep, (double)l);
   }

   //   printf("[PROBE-1] Verification des elements\n");

   // On compare
   for (l = 0; l < nbEl; l++){
     //     printf("[%ld] ", l);
      if ((double)l != probe_exhaustiveGetSample(ep, l)){
         printf("[PROBE-1] ERREUR : t[%f] = %f\n", (double)l, probe_exhaustiveGetSample(ep, l));
         result = 1;
      }
   }

   // Vérification de quelques résultats (min, max, moyenne, ...)
   if (probe_min(ep) != 0.0) {
      printf("[PROBE-1] ERREUR : min incorrect\n");
      result = 1;
   }
   if (probe_max(ep) != (double)(nbEl - 1)) {
      printf("[PROBE-1] ERREUR : max incorrect\n");
      result = 1;
   }
   if ((probe_mean(ep) - ((double)nbEl - 1.0)/2.0) > 0.00001) {
      printf("[PROBE-1] ERREUR : mean incorrect\n");
      result = 1;
   }
   probe_delete(ep);

   return result;
}

int main()
{
   int result = 0;
   unsigned long n;

   // Initialisation du simulateur
   motSim_create();

   testerExhaustive(1);
   for (n = 1 ; n < NBTEST; n++) {
      result |= testerExhaustive(n* PROBE_NB_SAMPLES_MAX - 1);
      result |= testerExhaustive(n* PROBE_NB_SAMPLES_MAX );
      result |= testerExhaustive(n* PROBE_NB_SAMPLES_MAX + 1);
   };

   if (result) {
      printf("[FAILED]\n");
   }else { 
      printf("[SUCCESS]\n");
   }

   return result;
}
