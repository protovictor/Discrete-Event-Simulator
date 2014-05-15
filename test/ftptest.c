#include <stdio.h>
#include <ftp.h>
#include <gnuplot.h>

void tracer(struct probe_t *pr, char *name, int nbBar);

int main()
{

   struct FTP_t               *fileTransfer;
   struct probe_t             *sejProbe, *iaProbe, *srvProbe;
   struct randomGenerator_t   *fileSize;
   struct dateGenerator_t     *readingTime;
   struct filePDU_t           *filePDU;
   struct PDUSource_t         *Source;
   struct PDUsink_t           *sink;
   struct srvGen_t            *server;
   double duration  = 100000.0;

   motSim_create();
   sink = PDUSink_create();

/* We create a fileTransfer with the default recommended parameters */

   fileTransfer = FTP_CreateFileTransfer();
   FTP_LoadParameters(fileTransfer,sink);

/* We get the values we need for putting sensors in the simulation */
   readingTime = FTP_getReadingTimeGen(fileTransfer);
   filePDU = FTP_getFilePDU(fileTransfer);
   server = FTP_getServer(fileTransfer);
   Source = FTP_getPDUSource(fileTransfer);

 
   //------------------------- Sensors -------------------
 
 // A sensor of inter-arrivals 
       iaProbe = probe_createExhaustive();
       dateGenerator_addInterArrivalProbe(readingTime, iaProbe);

 // A sensor for the journey/sejour 
       sejProbe = probe_createExhaustive(); 
       filePDU_addSejournProbe(filePDU, sejProbe);

 // A sensor for the service time 
       srvProbe = probe_createExhaustive();
       srvGen_addServiceProbe(server, srvProbe);

 //--------------------------------------

   PDUSource_start(Source);
  
   motSim_runUntil(duration); 
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
