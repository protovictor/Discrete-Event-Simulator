#include <stdio.h>
#include <stdlib.h>
#include <motsim.h>
#include <pdu-sink.h>
#include <srv-gen.h>
#include <date-generator.h>
#include <random-generator.h>
#include <pdu-source.h>
#include <probe.h>
#include <gnuplot.h>
#include <file_pdu.h>

  void tracer(struct probe_t *pr, char *name, int nbBar);
 
 int main()
{
  
  struct PDUSink_t        *sink;             //declaration of a sink
  struct srvGen_t         *server;           //declaration of a server
  struct filePDU_t        *filePDU;          //declaration of the filePDU
  struct dateGenerator_t  *dateGenExp;       //declaration of a data generator
  struct PDUSource_t      *sourcePDU;        //declaration of a source
  struct probe_t          *sejProbe, *iaProbe, *srvProbe, *szProbe;         //the sensors
  struct randomGenerator_t *sizeGen;

  float mu = 10.0;                           // parameter of the server
  float lambda = 5.0;                        // intensity of the arrival process
  float frequencePackets = 5.0;              // Average = packets/size
  float averageSize = 1000.0;                // Average size of the packets
  float debit = 10000.0;                     // Measured in bits/second
  

 /* Create a simulator */
       motSim_create(); 
 /* Create a sink */
       sink = PDUSink_create();

 /* Create a server */
       server = srvGen_create(sink, (processPDU_t)PDUSink_processPDU);       
 /* Initialisation of the server */       
 //    srvGen_setServiceTime(server, serviceTimeExp, mu);
       srvGen_setServiceTime(server, serviceTimeProp, 1.0/debit);  

 /* Create a file PDU */
       filePDU = filePDU_create(server, (processPDU_t)srvGen_processPDU);

 /* Create a data generator */
       dateGenExp = dateGenerator_createExp(frequencePackets);

 /* Create a source */
       sourcePDU = PDUSource_create(dateGenExp, filePDU, (processPDU_t)filePDU_processPDU);
       
 /* A sensor of inter-arrivals */
       iaProbe = probe_createExhaustive();
       dateGenerator_addInterArrivalProbe(dateGenExp, iaProbe);

 /* A sensor for the journey/sejour */
       sejProbe = probe_createExhaustive();
       filePDU_addSejournProbe(filePDU, sejProbe);

 /* A sensor for the service time */
       srvProbe = probe_createExhaustive();
       srvGen_addServiceProbe(server, srvProbe);

 /* Create a generator */
       sizeGen = randomGenerator_createUInt();
       randomGenerator_setDistributionExp(sizeGen, 1.0/averageSize);
 
 /* The influence over the source */
       PDUSource_setPDUSizeGenerator(sourcePDU, sizeGen);

 /* A sensor for sizeGen */
       szProbe = probe_createExhaustive();
       randomGenerator_addValueProbe(sizeGen,szProbe);

 /*-------------------------------------------------------------------*/
 /* Launching the simulator */


 /* We activate the source */
       PDUSource_start(sourcePDU);

 /* We establish that the simulation should last 100 seconds = 100 000 ms */
       motSim_runUntil(100000.0);

       motSim_printStatus();

       
   printf("Number of the packets from the file: %d\n", filePDU_length(filePDU));
   
   printf("Average time of journey = %f \n", probe_mean(sejProbe));

   printf("Average Inter-arrival: %f (1/lambda = %f) \n", probe_mean(iaProbe), 1.0/lambda);

   printf("Average service time = %f (1/mu = %f) \n", probe_mean(srvProbe), 1.0/mu);

   tracer(iaProbe, "Interarrivee\n", 100);
   tracer(sejProbe, "Temps de séjour\n", 100);


  
return 0;
}



   void tracer(struct probe_t *pr, char *name, int nbBar)
  { 
       struct probe_t    *gb;
       struct gnuplot_t  *gp;

     gb = probe_createGraphBar(probe_min(pr), probe_max(pr), nbBar);

     probe_exhaustiveToGraphBar(pr, gb);
     probe_setName(gb, name);
     gp = gnuplot_create();
     gnuplot_setXRange(gp, probe_min(gb), probe_max(gb)/2.0);
     gnuplot_displayProbe(gp, WITH_BOXES, gb);
  }


  




















