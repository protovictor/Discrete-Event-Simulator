/*
 * Quelques tests simples sur les sondes
 */
#include <stdlib.h>

#include <motsim.h>
#include <probe.h>


int main()
{
   struct probe_t * gbp, *ep;
   int n;

   double meanE, meanGB, data;

   motSim_create();

   // Un petit test tout bête de la moyenne  sur un graphBar
   gbp = probe_createGraphBar(-10.5, 10.5, 21);
   for (n = -10; n < 16; n++){
      data = (double)n + 0.5*((double)rand()/(double)RAND_MAX);
      //printf("[%f] ", data);
      probe_sample(gbp, data);
   };
   //printf("\n");
   //   probe_graphBarDump(gbp);
   meanGB = probe_mean(gbp);
   printf("[PROBE/1] Moyenne = %f\n", meanGB);

   probe_delete(gbp);

   gbp = probe_createGraphBar(-10.5, 10.5, 21);
   ep = probe_createExhaustive();
   gbp = probe_createGraphBar(-10.5, 10.5, 21);
   for (n = -10; n < 11; n++){
     probe_sample(ep, (double)n);
   };
   probe_exhaustiveToGraphBar(ep, gbp);

   meanE = probe_mean(ep);
   meanGB = probe_mean(gbp);
   printf("[PROBE/2] Moyennes = %f et %f\n", meanE, meanGB);
   //   probe_graphBarDump(gbp);

   return 0;
}
