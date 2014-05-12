#include <stdio.h>
#include <stdlib.h>
#include <motsim.h>
#include <pdu-source.h>
#include <pdu-sink.h>
#include <file_pdu.h>
#include <date-generator.h>
#include <random-generator.h>
#include <srv-gen.h>
#include <probe.h>
#include <gnuplot.h>

  /*  HTTP model - proposed in the document 
   *  Source Traffic Modeling of Wireless Applications 
   */

void tracer(struct probe_t *pr, char *name, int nbBar);

    int main()
{  
   int option;
   
   struct PDU_sink_t        *sink;
   struct srvGen_t          *server; 
   struct filePDU_t         *filePDU; 
   struct dateGenerator_t   *dateGen;
   struct randomGenerator_t *getReqSzGen, *reqSizeGen; // *req_per_session; 
   struct PDUSource_t       *getRequest, *reqPDU; 
   struct probe_t           *sejProbe, *iaProbe, *srvProbe;         //the sensors
   
   float debit = 1000.0;          // bits/second
   float lambda = 1; //0.033;   

   // for Weibull            -  interarrival time = time between requests
   double alpha = 0.5;
   double beta = 4.44;
  
   printf("Select an option for simulation:\n");
   printf("1. Request (press 1)\n");
   printf("2. Reply (press 2)\n");
   scanf("%d", &option);
   printf("option=%d", option);
 /* -------------------------------------------- */ 
    // We create a general simulator
       motSim_create();          
       sink = PDUSink_create();
  
 /* Common settings for request and reply */ 
  
  server = srvGen_create(sink, (processPDU_t)PDUSink_processPDU);
  srvGen_setServiceTime(server, serviceTimeProp, 1.0/debit); 

  filePDU = filePDU_create(server, (processPDU_t)srvGen_processPDU); 

 /* -----------------------------------------------*/
 
   if(option == 1)   // Request  // HTTP - OFF
   {
      // Lognormal distribution for GetRequest size packets
      double req_alpha = 5.84;
      double req_beta  = 0.29;  
      
      dateGen = dateGenerator_createExp(lambda);

      getReqSzGen = randomGenerator_createDouble();
      randomGenerator_setDistributionLognormal(getReqSzGen, req_alpha, req_beta);
      
      getRequest = PDUSource_create(dateGen, filePDU, (processPDU_t)filePDU_processPDU); 
      PDUSource_setPDUSizeGenerator(getRequest, getReqSzGen);

   }
   else if(option==2)    // Reply   // HTTP - ON
   {
   
      // Lognormal distribution -  size of main object
      double a = 1.31;
      double b = 1.41;
      // Lognormal distribution -  size of inline objects
      double ain = -0.75;
      double bin = 2.36;
      // Gamma distribution     -  number of inline objects 
      double galpha = 0.24;
      double gbeta = 23.42;
       
       // The recommended model uses for interarrival time a Weibull distribution
      dateGen = dateGenerator_createWeibull(alpha, beta);

      /* Request size*/
      reqSizeGen = randomGenerator_createDouble();
      randomGenerator_setDistributionComposed(reqSizeGen, a, b, ain, bin, galpha, gbeta);
      /* To do --- the replied page doesn't load sinchronously
        the main object loads first and the inline objects after */
 
      reqPDU = PDUSource_create(dateGen, filePDU, (processPDU_t)filePDU_processPDU);  
  
      /* We associate the size of the request packet to the reqPDU */
      PDUSource_setPDUSizeGenerator(reqPDU, reqSizeGen);
  }
  

 //------------------------- Sensors -------------------
 
 // A sensor of inter-arrivals 
       iaProbe = probe_createExhaustive();
       dateGenerator_addInterArrivalProbe(dateGen, iaProbe);

 // A sensor for the journey/sejour 
       sejProbe = probe_createExhaustive();
       filePDU_addSejournProbe(filePDU, sejProbe);

 // A sensor for the service time 
       srvProbe = probe_createExhaustive();
       srvGen_addServiceProbe(server, srvProbe);

 //-----------------------------------------------------
 
  if(option==1)
   PDUSource_start(getRequest);
  if(option==2) 
   PDUSource_start(reqPDU);
 
   motSim_runUntil(100000.0); //100seconds 
 
   motSim_printStatus();
 
   printf("Number of the packets remaining: %d\n", filePDU_length(filePDU));
   
   printf("Average time of journey = %f \n", probe_mean(sejProbe));

   printf("Average Inter-arrival: %f \n", probe_mean(iaProbe));

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
