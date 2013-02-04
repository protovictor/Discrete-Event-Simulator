/*
 *    Quelques tests simples sur les sondes
 *
 *    probes-2
 */
#include <float.h>
#include <stdio.h>
#include <stdlib.h>

#include <motsim.h>
#include <probe.h>

#define TPMAX 200
#define NBECH 200

void tracer(struct probe_t * gb, char * name)
{
   struct gnuplot_t * gp;

   probe_setName(gb, name);

   gp = gnuplot_create();
   gnuplot_setYRange(gp, 0.0, 1.1*(double)probe_graphBarGetMaxValue(gb));
   gnuplot_setXRange(gp, probe_min(gb), probe_max(gb));
   gnuplot_displayProbe(gp, gb);
}


int main()
{
   struct probe_t * gbp, *ep;
   int  e, n;
   unsigned long l;

   double meanE, meanGB, data;

   // Initialisation du simulateur
   motSim_create();

   // Création d'une sonde exhaustive
   ep = probe_createExhaustive();

   // On la peuple avec NBECH echantillons par unitée
   for (n = -TPMAX; n < TPMAX ; n++){
      for (e = 0 ; e < NBECH; e++) {
         data = (double)n + ((double)rand()/(double)RAND_MAX);
         probe_sample(ep, data);
      }
   };

   // On cré un graphbar avec une barre par unité
   gbp = probe_createGraphBar(-(double)TPMAX , (double)TPMAX, 2 * TPMAX );
   probe_exhaustiveToGraphBar(ep, gbp);

   // Calcul des deux moyennes (exhaustive et sur le graphBar)
   meanE = probe_mean(ep);
   meanGB = probe_mean(gbp);


   printf("[PROBE-1] Nombre d'echantillons = %d\n", probe_nbSamples(ep)); 
   printf("[PROBE-1] Moyenne exhaustive    = %f\n", meanE);
   printf("[PROBE-1] Nombre d'echantillons = %d\n", probe_nbSamples(gbp)); 
   printf("[PROBE-1] Moyenne graph         = %4.1e\n", meanGB);

   tracer(gbp, "Pour voir");

   printf("*** ^C pour finir ;-)\n");
   pause();


   if (    (meanGB >= 1.0e-10) 
        || (probe_nbSamples(ep) != probe_nbSamples(gbp))
      ){
      printf("[FAILED]\n");
      return 1;
   };

   printf("[SUCCESS]\n");
   return 0;
}
