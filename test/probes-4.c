/*
 * Quelques tests simples sur les sondes
 *
 * probes-4 : test des outils de consultation des sondes exhaustives
 *
 * On va remplir une sonde avec un grand nombre de valeurs puis essayer
 * de retrouver des vlaeurs au hasard
 */
#include <stdlib.h>
#include <stdio.h>

#include <motsim.h>
#include <probe.h>

#define NB_SAMPLE 10000000
#define NB_TEST 1000000

int main()
{
   struct probe_t  *ep;
   int n, v;

   printf("[PROBES-4] ... ");
   fflush(stdout);

   motSim_create();

   ep = probe_createExhaustive();

   for (n = 0 ; n < NB_SAMPLE; n++) {
      probe_sample(ep, (double)n);
   };

   for (n = 1 ; n < NB_TEST; n++) {
      v = rand()%NB_SAMPLE;
      if (v != (int)probe_exhaustiveGetSampleN(ep, v)) {
	 printf("ERROR : pr[%d] = %f\n", v, probe_exhaustiveGetSampleN(ep, v));
 	 exit(1);
      }
   }
   printf("[OK]\n");
   return 0;
}
