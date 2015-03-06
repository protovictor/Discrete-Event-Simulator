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

   double lambda;
   double alpha; //!< Must be > 1
   double xmin;
   double plafond;
   double mu;
   double sigma;
   double average;
   int n;
 //    srand(time(NULL));
   motSim_create();  //!< Always needed
   pr = probe_createExhaustive();

//======================
//Partie avec exponentielle !
   printf("\n---Tests expo---\n");

   lambda = 5.5;
   printf("=======\nPartie avec distribution exponentielle (lambda = %f)\n",lambda);
   
   rg = randomGenerator_createDoubleExp(lambda);
   randomGenerator_addValueProbe(rg, pr);//Si vous oubliez cette ligne, malheur à vous !
   average = 0.0;
  for (n = 0; n < NB_SAMPLES; n++) {
      average += randomGenerator_getNextDouble(rg);
   }
   average /= NB_SAMPLES;
   printf("Moyenne = %f (théorique = %f)\n",average,1/lambda);
   draw(pr, "DistributionExponentiel (l = 4).png", "Dist. exp.", 50, 0.4);

   motSim_reset();

   lambda = 1.1;
   printf("=======\nPartie avec distribution exponentielle (lambda = %f)\n",lambda);
   
    randomGenerator_setDistributionExp(rg, lambda);
   average = 0.0;
  for (n = 0; n < NB_SAMPLES; n++) {
      average += randomGenerator_getNextDouble(rg);
   }
   average /= NB_SAMPLES;
   printf("Moyenne = %f (théorique = %f)\n",average,1/lambda);
   draw(pr, "DistributionExponentiel (l = 1).png", "Dist. exp.", 50, 0.4);

   motSim_reset();


   lambda = 0.7;
   printf("=======\nPartie avec distribution exponentielle (lambda = %f)\n",lambda);
   
   rg = randomGenerator_createDoubleExp(lambda);
   randomGenerator_addValueProbe(rg, pr);//Si vous oubliez cette ligne, malheur à vous !
   average = 0.0;
  for (n = 0; n < NB_SAMPLES; n++) {
      average += randomGenerator_getNextDouble(rg);
   }
   average /= NB_SAMPLES;
   printf("Moyenne = %f (théorique = %f)\n",average,1/lambda);
   draw(pr, "DistributionExponentiel (l = 1).png", "Dist. exp.", 50, 0.4);

   motSim_reset();

   lambda = 0.5;
   printf("=======\nPartie avec distribution exponentielle (lambda = %f)\n",lambda);
   
  randomGenerator_setDistributionExp(rg, lambda);
   average = 0.0;
  for (n = 0; n < NB_SAMPLES; n++) {
      average += randomGenerator_getNextDouble(rg);
   }
   average /= NB_SAMPLES;
   printf("Moyenne = %f (théorique = %f)\n",average,1/lambda);
   draw(pr, "DistributionExponentiel (l = 1S2).png", "Dist. exp.", 50, 0.4);

   motSim_reset();






   //=========================================
   //Tests sur trunc lognorm !
   printf("\n---Tests lognorm---\n");

   mu=0.;
   sigma = 1;
   plafond = 100.;
   printf("=======\nPartie avec Trunc lognorm (mu = %f, sigma = %f, plafond = %f)\n",mu,sigma,plafond);

   rg = randomGenerator_createDoubleRangeTruncLogNorm(mu,sigma,plafond);
      pr = probe_createExhaustive();
   randomGenerator_addValueProbe(rg, pr);

   average = 0.0;
   for (n = 0; n < NB_SAMPLES; n++) {
      average += randomGenerator_getNextDouble(rg);
   }
   average /= NB_SAMPLES;
   printf("Moyenne = %f (théorique = %f)\n",average,exp(mu+sigma*sigma/2));
   draw(pr, "DistributionTruncLogNorm sigma 1.png", "Dist. logNorm.", 50, 1.0);
   motSim_reset();


   sigma = 1.37;
   mu = 8.23;
   plafond = 50000000.;
   printf("=======\nPartie avec Trunc lognorm (mu = %f, sigma = %f, plafond = %f)\n",mu,sigma,plafond);

   randomGenerator_setDistributionTruncLogNorm(rg,mu,sigma,plafond);
   average = 0.0;
   for (n = 0; n < NB_SAMPLES; n++) {
      average += randomGenerator_getNextDouble(rg);
   }
   average /= NB_SAMPLES;
   printf("Moyenne = %f (théorique = %f)\n",average,exp(mu+sigma*sigma/2));
   draw(pr, "DistributionTruncLogNorm sigma 5.png", "Dist. logNorm.", 50, 1.0);
   motSim_reset();


   sigma = 0.5; mu = 0.;
   plafond = 50.;
   printf("=======\nPartie avec Trunc lognorm (mu = %f, sigma = %f, plafond = %f)\n",mu,sigma,plafond);

   randomGenerator_setDistributionTruncLogNorm(rg,mu,sigma,plafond);
   average = 0.0;
   for (n = 0; n < NB_SAMPLES; n++) {
      average += randomGenerator_getNextDouble(rg);
   }
   average /= NB_SAMPLES;
   printf("Moyenne = %f (théorique = %f)\n",average,exp(mu+sigma*sigma/2));
   draw(pr, "DistributionTruncLogNorm sigma 1S2.png", "Dist. logNorm.", 50, 1.0);
   motSim_reset();

   mu=0.5;
   sigma = 0.5;
   plafond = 50.;
   printf("=======\nPartie avec Trunc lognorm (mu = %f, sigma = %f, plafond = %f)\n",mu,sigma,plafond);

   randomGenerator_setDistributionTruncLogNorm(rg,mu,sigma,plafond);
   average = 0.0;
   for (n = 0; n < NB_SAMPLES; n++) {
      average += randomGenerator_getNextDouble(rg);
   }
   average /= NB_SAMPLES;
   printf("Moyenne = %f (théorique = %f)\n",average,exp(mu+sigma*sigma/2));
   draw(pr, "DistributionTruncLogNorm sigma 1S2 (mu 1S2).png", "Dist. logNorm.", 50, 1.0);
   motSim_reset();



   mu = 0.;
   sigma = 0.125;
   printf("=======\nPartie avec Trunc lognorm (mu = %f, sigma = %f, plafond = %f)\n",mu,sigma,plafond);

   average = 0.0;
   for (n = 0; n < NB_SAMPLES; n++) {
      average += randomGenerator_getNextDouble(rg);
   }
   average /= NB_SAMPLES;
   printf("Moyenne = %f (théorique = %f)\n",average,exp(mu+sigma*sigma/2));
   draw(pr, "DistributionTruncLogNorm sigma 1S8.png", "Dist. logNorm.", 50, 1.0);
   motSim_reset();

   randomGenerator_delete(rg);

//====
//Partie avec Pareto 
   printf("\n---Tests Pareto---\n");

   alpha = 4.;xmin = 2.;
   printf("=======\nPartie avec Pareto (alpha = %f, xmin = %f, plafond = %f)\n",alpha,xmin,plafond);
   
   //rg = randomGenerator_createDoubleRange(0.,1.); // Default dist is uniform
   //randomGenerator_setDistributionTruncPareto(rg,alpha,xmin,plafond);
   rg = randomGenerator_createDoubleRangeTruncPareto(alpha,xmin,plafond);
   randomGenerator_addValueProbe(rg, pr);
   average = 0.0;
   for (n = 0; n < NB_SAMPLES; n++) {
      average += randomGenerator_getNextDouble(rg);
   }
   average /= NB_SAMPLES;
   printf("Moyenne = %f (théorique = %f)\n",average,alpha*xmin/(alpha-1.));
   draw(pr, "DistributionTruncPareto 1.png", "Dist. pareto.", 50, 1.0);
   motSim_reset();

   alpha = 1.5 ; xmin = 0.5; plafond = 10000000.;
   printf("=======\nPartie avec Pareto (alpha = %f, xmin = %f, plafond = %f)\n",alpha,xmin,plafond);

   rg = randomGenerator_createDoubleRangeTruncPareto(alpha,xmin,plafond);
   randomGenerator_addValueProbe(rg, pr);
   average = 0.0;
   for (n = 0; n < NB_SAMPLES; n++) {
      average += randomGenerator_getNextDouble(rg);
   }
   average /= NB_SAMPLES;
   printf("Moyenne = %f (théorique = %f)\n",average,alpha*xmin/(alpha-1.));
   draw(pr, "DistributionTruncPareto 2.png", "Dist. pareto.", 50, 1.0);
   motSim_reset();

   alpha = 5.; xmin = 0.01;
   printf("=======\nPartie avec Pareto (alpha = %f, xmin = %f, plafond = %f)\n",alpha,xmin,plafond);

   rg = randomGenerator_createDoubleRangeTruncPareto(alpha,xmin,plafond);
   randomGenerator_addValueProbe(rg, pr);
   average = 0.0;
   for (n = 0; n < NB_SAMPLES; n++) {
      average += randomGenerator_getNextDouble(rg);
   }
   average /= NB_SAMPLES;
   printf("Moyenne = %f (théorique = %f)\n",average,alpha*xmin/(alpha-1.));
   draw(pr, "DistributionTruncPareto 3.png", "Dist. pareto.", 50, 1.0);
   motSim_reset();


         
   randomGenerator_delete(rg);
//=========
//Partie avec uniforme
      printf("\n---Tests uniforme---\n");

   rg = randomGenerator_createDoubleRange(0., 1.0); // Distribution uniforme par défaut. J'ai détruit pour recréer...
    printf("=======\nPartie avec distribution uniforme sur [0,1]\n");

   randomGenerator_addValueProbe(rg, pr);
   average = 0.0;
   for (n = 0; n < NB_SAMPLES; n++) {
      average += randomGenerator_getNextDouble(rg);
   }
   average /= NB_SAMPLES;
   printf("Moyenne = %f (théorique = %f)\n",average,0.);
   draw(pr, "DistributionUnif.png", "Dist. unif.", 50, 1.0);
   motSim_reset();
   
   randomGenerator_delete(rg);




  
   return 0;
}
