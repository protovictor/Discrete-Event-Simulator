/*
 * Test des générateurs discrets.
 * 
 * On simule un grand nombre de lancers d'un dé à six faces.
 *
 * WARNING les résultats théoriques sont en durs, donc indépendants
 * des paramètres.
 */
#include <stdio.h>     // printf, ...
#include <math.h>      // fabs

#include <motsim.h>
#include <random-generator.h>
#include <probe.h>


#define NBECH 1000000

/*----------------------------------------------------------------------*/
/*                                                                      */
/*----------------------------------------------------------------------*/
int main() {
   struct randomGenerator_t * rg;
   struct probe_t           * rp; 
   int n, v;
   double  m, e, var, t;

   unsigned int facesDe[] = {1, 2, 3, 4, 5, 6};
   double probaDe[] = {1.0/6.0, 1.0/6.0, 1.0/6.0, 1.0/6.0, 1.0/6.0, 1.0/6.0};

   motSim_create();

   rp = probe_createExhaustive();  // Les sondes datent les échantillons

   rg = randomGenerator_createUIntDiscreteProba(6, facesDe, probaDe);

   for (n = 0 ; n < NBECH;n++){
      v = randomGenerator_getNextUInt(rg);
      //      printf("[%d] ", v);
      probe_sample(rp, (double)v);
   }
   printf("\n");

   // Résultats pour un tirage de dé non pipé
   m = probe_mean(rp);
   e = 3.5;  // WARNING
   var = probe_variance(rp);
   t =  35.0/12.0; // WARNING

   printf("%d lancers de dé à six faces (non pipé) :\n", NBECH);
   printf("Moyenne    = %f\n", m);
   printf("Espérance  = %f\n", e);
   printf("Variance   = %f\n", var);
   printf("(théorique)= %f\n", t);
   printf("Ecart type = %f\n", probe_stdDev(rp));


   probe_delete(rp);
   randomGenerator_delete(rg);

   if ((fabs(m-e)/e < 0.05) && (fabs(t-var)/t < 0.05)){
      return  0;
   } else {
      return 1;
   }
}
