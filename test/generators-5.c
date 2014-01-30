/*
 * Test des générateurs créés sur des fichiers
 */
#include <motsim.h>
#include <random-generator.h>

#include <stdio.h>     // printf, ...

#define NBECH 1000000

/*----------------------------------------------------------------------*/
/*                                                                      */
/*----------------------------------------------------------------------*/
int main() {
   struct randomGenerator_t * rg;
   struct probe_t           * pr1;
   int n;
   unsigned int v ;

   motSim_create(); 

   printf("AAA\n");
   rg = randomGenerator_createUIntDiscreteFromFile("uintdiscrete-1.txt");
   printf("BBB\n");
   pr1 = probe_createExhaustive();
   printf("CCC\n");
   for (n = 0 ; n < NBECH;n++){
     v = randomGenerator_getNextUInt(rg);
     //     printf("%d ", v);
      probe_sample(pr1, (double)v);
   }

   printf("Moyenne = %lf (esp %lf)\n", probe_mean(pr1), randomGenerator_getExpectation(rg));

   if (fabs( probe_mean(pr1)  - randomGenerator_getExpectation(rg)) < 0.1) {
     return 0;
   }
   return 1;
}

