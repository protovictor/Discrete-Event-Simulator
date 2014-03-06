/**
 * @file rg-its.c
 * @brief Basic examples for random generators based on ITS
 *
 * This files shows how to use ITS (Inverse Transform Sampling) in
 * order to build a random generator. An exponential and a pareto
 * distribution are implemented.
 */

#include <stdio.h>
#include <math.h>

#include <motsim.h>
#include <random-generator.h>

/**
 * @brief Inverse of CDF for exponential distribution
 */
double expDistQ(double x, double lambda)
{
   return - log(1.0 -x) / lambda;
}

/**
 * @brief Inverse of CDF for pareto distribution
 */
double paretoDistQ(double x, double alpha, double xmin)
{
   return  xmin/(pow(x, 1.0/alpha));
}

/**
 * Number of samples to generate
 */
#define NB_SAMPLES 1000

int main() {
   struct randomGenerator_t * rg;
   double lambda = 10.0;
   double alpha = 2.0; //!< Must be > 1
   double xmin = 5.0;
   double average;
   int n;

   motSim_create();  //!< Always needed

   rg = randomGenerator_createDouble();

   /**
    * Defining a random generator with exponential distribution
    */
   randomGenerator_setQuantile1Param(rg, expDistQ, lambda);
   printf("exponential distribution (lambda = %f) exp. = %f\n", lambda, 1.0/lambda);

   // Running 
   average = 0.0;
   for (n = 0; n < NB_SAMPLES; n++) {
      average += randomGenerator_getNextDouble(rg);
   }
   average /= NB_SAMPLES;
   printf("avg (%d samples) : %f\n", NB_SAMPLES, average);

   /**
    * Defining a random generator with pareto distribution
    */
   randomGenerator_setQuantile2Param(rg, paretoDistQ, alpha, xmin);
   printf("pareto distribution (alpha = %f, xmin = %f) exp. = %f\n", alpha, xmin, alpha*xmin/(alpha-1.0));

   average = 0.0;
   for (n = 0; n < NB_SAMPLES; n++) {
      average += randomGenerator_getNextDouble(rg);
   }
   average /= NB_SAMPLES;
   printf("avg (%d samples) : %f\n", NB_SAMPLES, average);
   
   return 0;
}
