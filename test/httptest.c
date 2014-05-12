#include <motsim.h>
#include <gnuplot.h>
#include <httpmodel.h>
#include <probe.h>

void tracer(struct probe_t *pr, char *name, int nbBar);

 int main()
{
  struct probe_t          *sejProbe, *iaProbe, *srvProbe;
  struct httpSim_t        *simHTTP;
  struct PDUSource_t      *PDUSource;
  struct filePDU_t        *filePDU;
  struct dateGenerator_t  *dateGen;
  struct srvGen_t         *server;
  struct PDUSink_t        *sink;
  int option;

  printf("Select an option for simulation:\n");
  printf("1. Request (press 1)\n");
  printf("2. Reply (press 2)\n");
  scanf("%d", &option);
 
  motSim_create();
  sink = PDUSink_create();
  
  if(option==1)
   simHTTP = httpSim_CreateRequest(sink);
  if(option==2)
   simHTTP = httpSim_CreateReply(sink);
 
  dateGen = httpSim_GetDateGen(simHTTP);
  filePDU = httpSim_GetFilePDU(simHTTP);
  server = httpSim_GetServer(simHTTP);
  PDUSource = httpSim_GetPDUSource(simHTTP);
 

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

 //--------------------------------------

   PDUSource_start(PDUSource);
  
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
