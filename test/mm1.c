/*----------------------------------------------------------------------*/
/*   Test de NDES : simulation d'un système M/M/1                       */
/*----------------------------------------------------------------------*/

#include <stdlib.h>    // Malloc, NULL, exit, ...
#include <assert.h>
#include <strings.h>   // bzero, bcopy, ...
#include <stdio.h>     // printf, ...

#include <file_pdu.h>
#include <pdu-source.h>
#include <pdu-sink.h>
#include <srv-gen.h>
#include <date-generator.h>
#include <probe.h>
#include <gnuplot.h>


void tracer(struct probe_t * pr, char * name, int nbBar)
{
   struct probe_t   * gb;
   struct gnuplot_t * gp;

   gb = probe_createGraphBar(probe_min(pr), probe_max(pr), nbBar);
   probe_exhaustiveToGraphBar(pr, gb);
   probe_setName(gb, name);

   gp = gnuplot_create();
   gnuplot_setXRange(gp, probe_min(gb), probe_max(gb)/2.0);
   gnuplot_displayProbe(gp, WITH_BOXES, gb);

}

int main() {
   struct PDUSource_t     * sourcePDU;
   struct dateGenerator_t * dateGenExp;
   struct filePDU_t       * filePDU;
   struct srvGen_t        * serveur;
   struct PDUSink_t       * sink;

   struct probe_t         * sejProbe, * iaProbe, * srvProbe;

   float                    lambda = 5.0,
                                mu = 10.0; // ATTENTION : a passer en param

   /* Creation du simulateur */
   motSim_create();

   /* Le puits */
   sink = PDUSink_create();

   /* Le serveur */
   serveur = srvGen_create(sink, PDUSink_processPDU);

   /* La file */
   filePDU = filePDU_create(serveur, srvGen_processPDU);

   /* Création d'un générateur de date */
   dateGenExp = dateGenerator_createExp(lambda);

   /* La source */
   sourcePDU = PDUSource_create(dateGenExp, filePDU, filePDU_processPDU);

   /* Les sondes */
   iaProbe = probe_createExhaustive();
   dateGenerator_setInterArrivalProbe(dateGenExp, iaProbe);

   sejProbe = probe_createExhaustive();
   filePDU_addSejournProbe(filePDU, sejProbe);

   srvProbe = probe_createExhaustive();
   srvGen_setServiceProbe(serveur, srvProbe);

   /* On active la source */
   PDUSource_start(sourcePDU);
   motSim_runUntil(100000.0);

   motSim_printStatus();
   printf("%d paquets restant dans  la file\n", filePDU_length(filePDU));
   printf("Temps moyen de sejour dans la file = %f\n", probe_mean(sejProbe));
   printf("Interarive moyenne     = %f (1/lambda = %f)\n", probe_mean(iaProbe), 1.0/lambda);
   printf("Temps de service moyen = %f (1/mu     = %f)\n", probe_mean(srvProbe), 1.0/mu);

   tracer(iaProbe, "Interarrivee", 100);
   tracer(sejProbe, "Temps de séjour", 100);

   printf("*** ^C pour finir ;-)\n");
   pause();

   return 1;
}
