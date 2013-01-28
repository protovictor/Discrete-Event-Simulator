/*
 * Test du rejeu des générateurs
 */
#include <motsim.h>
#include <random-generator.h>

#include <stdio.h>     // printf, ...

#define NBECH 100000

/*----------------------------------------------------------------------*/
/*                                                                      */
/*----------------------------------------------------------------------*/
int main() {
   struct randomGenerator_t * rg;
   struct probe_t           * pr1, *pr2, *pr3; 
   int n;
   double v ;

   motSim_create(); // Les sondes datent les échantillons

   pr1 = probe_createExhaustive();
   probe_setPersistent(pr1);  // On va utiliser un motSim_reset
   pr2 = probe_createExhaustive();
   probe_setPersistent(pr2);
   pr3 = probe_createExhaustive();
   probe_setPersistent(pr3);

   // On crée un générateur de réels (loi exp par défaut)
   rg = randomGenerator_createDoubleExp(1.0);

   // On lui fait faire un premier tour
   for (n = 0 ; n < NBECH;n++){
      probe_sample(pr1, randomGenerator_getNextDouble(rg));
   }

   // On demande maintenant d'enregistrer
   randomGenerator_recordThenReplay(rg);

   // Un deuxième tour
   for (n = 0 ; n < NBECH;n++){
      v = randomGenerator_getNextDouble(rg);
      //      printf("pr2[%d] <- %f\n", n, v);
      probe_sample(pr2, v);
   }

   // Le troisième tour sera un rejeu 
   motSim_reset();

   // Un troisieme tour
   for (n = 0 ; n < NBECH;n++){
      v = randomGenerator_getNextDouble(rg);
      //      printf("pr3[%d] <- %f\n", n, v);
      probe_sample(pr3, v);
   }

   // Comparaison des résultats
   for (n = 0 ; n < NBECH;n++){
     //          printf("%f != %f = %f\n", probe_exhaustiveGetSampleN(pr1, n),
     //	    probe_exhaustiveGetSampleN(pr2, n),probe_exhaustiveGetSampleN(pr3, n));

     if (probe_exhaustiveGetSampleN(pr1, n) == probe_exhaustiveGetSampleN(pr2, n)) {
        printf("Coup de chance sur %d !!!\n", n);
     }
     if (probe_exhaustiveGetSampleN(pr2, n) != probe_exhaustiveGetSampleN(pr3, n)) {
       printf("ERREUR sur %d : %f != %f\n", n, probe_exhaustiveGetSampleN(pr2, n),probe_exhaustiveGetSampleN(pr3, n));
       exit(1);
     }
   }

   probe_delete(pr1);
   probe_delete(pr2);
   probe_delete(pr3);
   randomGenerator_delete(rg);

   printf("[OK]\n");
   return 0;
}
