/**
 * Le RG-DRAW de Benjamin le beau gosse tmtc tkt !
 */

#include <stdio.h>
#include <math.h>

#include <motsim.h>
#include <random-generator.h>
#include <gnuplot.h>

/*
 * Affichage (via gnuplot) de la probre pr
 * elle sera affichée comme un graphbar de nbBar barres
 * avec le nom name
 */
void draw(struct probe_t * pr, char * fileName, char * name, int nbBar, double prop)
{
   struct probe_t   * gb;
   struct gnuplot_t * gp;

   printf("Min %f, max %f, nb %d\n", probe_min(pr), probe_max(pr), probe_nbSamples(pr));
   /* On crée une sonde de type GraphBar */
   gb = probe_createGraphBar(probe_min(pr), probe_max(pr), nbBar/prop);
   probe_setName(gb, name);

   /* On convertit la sonde passée en paramètre en GraphBar */
   probe_exhaustiveToGraphBar(pr, gb);
   probe_graphBarNormalize(gb);

   /* On initialise une section gnuplot */
   gp = gnuplot_create();

   //! Output png to filename
   gnuplot_setTerminalType(gp, gnuplotTerminalTypePng);
   gnuplot_setOutputFileName(gp, fileName);

   /* On recadre les choses */
   gnuplot_setXRange(gp, probe_min(gb), probe_max(gb)*prop);

   /* On affiche */
   gnuplot_displayProbe(gp, WITH_BOXES, gb);

   //! Destruction
   gnuplot_delete(gp);
   probe_delete(gb);
}


/**
 * Number of samples to generate
 */
#define NB_SAMPLES 100000

int main() {
   struct randomGenerator_t * rg;
   struct probe_t * pr;

   double lambda = 10.0;
   double alpha = 3.0; //!< Must be > 1
   double xmin = 2.0;
   double plafond = 5.0;
   double average;
   int n;
     srand(time(NULL));
   motSim_create();  //!< Always needed

   rg = randomGenerator_createDouble();
   pr = probe_createExhaustive();
   randomGenerator_addValueProbe(rg, pr);

   /**
    * Defining a random generator with exponential distribution
    */  
 printf("Partie avec distribution exponentielle (lambda = %f)\n",lambda);
   
  randomGenerator_setDistributionExp(rg, lambda);
   average = 0.0;
  for (n = 0; n < NB_SAMPLES; n++) {
      average += randomGenerator_getNextDouble(rg);
   }
   average /= NB_SAMPLES;

   draw(pr, "DistributionExponentiel.png", "Dist. exp.", 50, 0.4);

   motSim_reset();

   printf("Partie avec Trunc lognorm (mu = 0, sigma = 1, plafond = %f)\n",plafond);
   rg = randomGenerator_createDoubleRange(0.0, 1.0); // Default dist is uniform

   randomGenerator_setDistributionTruncLogNorm(rg,0.,1.,plafond);
   randomGenerator_addValueProbe(rg, pr);
   average = 0.0;
   for (n = 0; n < NB_SAMPLES; n++) {
      average += randomGenerator_getNextDouble(rg);
   }
   average /= NB_SAMPLES;

   draw(pr, "DistributionTruncLogNorm.png", "Dist. logNorm.", 50, 1.0);
   motSim_reset();

//====
//Partie avec Pareto 

   printf("Partie avec Pareto (alpha = %f, xmin = %f, plafond = %f)\n",alpha,xmin,plafond);
   rg = randomGenerator_createDoubleRange(0.,1.); // Default dist is uniform
   randomGenerator_setDistributionTruncPareto(rg,alpha,xmin,plafond);
   randomGenerator_addValueProbe(rg, pr);
   average = 0.0;
   for (n = 0; n < NB_SAMPLES; n++) {
      average += randomGenerator_getNextDouble(rg);
   }
   average /= NB_SAMPLES;

   draw(pr, "DistributionTruncPareto.png", "Dist. pareto.", 50, 1.0);
   motSim_reset();

   
   randomGenerator_delete(rg);
   rg = randomGenerator_createDoubleRange(-5.0, 5.0); // Distribution uniforme par défaut. J'ai détruit pour recréer...
    printf("Partie avec distribution uniforme (sur [-5,5] car [0;1] c'est trop mainstream)\n",alpha,xmin,plafond);

   randomGenerator_addValueProbe(rg, pr);
   average = 0.0;
   for (n = 0; n < NB_SAMPLES; n++) {
      average += randomGenerator_getNextDouble(rg);
   }
   average /= NB_SAMPLES;

   draw(pr, "DistributionUnif.png", "Dist. unif.", 50, 1.0);


  
   return 0;
}
