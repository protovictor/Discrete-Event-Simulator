/**
 * @file src-tcpss-ex.c
 * @brief An example of src-tcpss usage
 *
 * This program simulates the transmission of a file through an ADSL
 * access link.
 */
#include <motsim.h>
#include <pdu-sink.h>
#include <ll-simplex.h>
#include <src-tcpss.h>
#include <probe.h>

#include <stdlib.h>    // Malloc, NULL, exit, ...
#include <assert.h>
#include <strings.h>   // bzero, bcopy, ...
#include <stdio.h>     // printf, ...

#include <file_pdu.h>
#include <pdu-source.h>
#include <srv-gen.h>
#include <date-generator.h>
#include <random-generator.h>
#include <gnuplot.h>
#include <src-httpss.h>

/*
 * Affichage (via gnuplot) de la probre pr
 * elle sera affichée comme un graphbar de nbBar barres
 * avec le nom name
 */
void tracer(struct probe_t * pr, char * name, int nbBar)
{
   struct probe_t   * gb;
   struct gnuplot_t * gp;

   /* On crée une sonde de type GraphBar */
   gb = probe_createGraphBar(probe_min(pr), probe_max(pr), nbBar);

   /* On convertit la sonde passée en paramètre en GraphBar */
   probe_exhaustiveToGraphBar(pr, gb);

   /* On la baptise */
   probe_setName(gb, name);

   /* On initialise une section gnuplot */
   gp = gnuplot_create();

   /* On recadre les choses */
   gnuplot_setXRange(gp, probe_min(gb), probe_max(gb)/2.0);

   /* On affiche */
   gnuplot_displayProbe(gp, WITH_BOXES, gb);
}


/**
 * @brief Characteristics of the file, access link and network
 *
 */
#define FILE_SIZE 300000

#define ACCESS_LINK_THROUGHPUT 1000000
#define ACCESS_LINK_TRANSM_TIME 0.00001

#define RTT 0.20
#define MTU 1500
#define nbTCP 1


int main()
{
   struct srcTCPSS_t  * src;
   struct llSimplex_t * link;
   struct PDUSink_t   * sink;
   struct probe_t     * pr;
   struct probe_t     * pr_arrival;
   int i, WINDOW_SIZE;
   double data;

   motSim_create();


   // The packets are sent to a sink
   sink = PDUSink_create();

   // Let's put a probe on the sink
   pr = probe_createExhaustive();
   PDUSink_addInputProbe(sink, pr);

   // The client access network
   link = llSimplex_create(sink,
                           PDUSink_processPDU,
			   ACCESS_LINK_THROUGHPUT, ACCESS_LINK_TRANSM_TIME);

   // The source (HTTP)
   WINDOW_SIZE =1; 
   src = srcHTTPSS_init_default(MTU, nbTCP, 0, RTT, WINDOW_SIZE, link, llSimplex_processPDU);

   // Send a file
   srcHTTPSS_sessionStart(src);

   motSim_runUntilTheEnd();

   // Print some info
   printf("%ld packets received\n", probe_nbSamples(pr));
   for (i = 1; i < probe_nbSamples(pr); i++) {
     printf("[%lf] %ld\n", probe_exhaustiveGetDateN(pr, i), (long)probe_exhaustiveGetSampleN(pr, i));
     printf("difference = %lf\n", probe_exhaustiveGetDateN(pr, i)-probe_exhaustiveGetDateN(pr, i-1));
   }

 	// Création d'une sonde exhaustive sur les dates d'arrivées
   	pr_arrival = probe_createExhaustive();

   // On la peuple avec NBECH echantillons par unitée
   for (i = 0; i < probe_nbSamples(pr); i++) {
	 data = probe_exhaustiveGetDateN(pr, i);
         probe_sample(pr_arrival, data);
   };
	
   tracer(pr_arrival, "Interarrivee", 300);

	while (1) {};


   return 0;
}
