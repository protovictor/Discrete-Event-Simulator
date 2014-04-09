#include <motsim.h>
#include <gnuplot.h>
#include <http.h>
#include <probe.h>

void tracer(struct probe_t *pr, char *name, int nbBar);

 int main()
{
  struct probe_t          *sejProbe, *iaProbe, *srvProbe;
  struct httpRequest_t    *Request;
  struct httpReply_t    *Reply;
  struct PDUSource_t      *PDUSource;
  struct filePDU_t        *filePDU;
  struct dateGenerator_t  *dateGen;
  struct srvGen_t         *server;
  struct PDUSink_t        *sink;
  int option;

  motSim_create();
  sink = PDUSink_create();
  
  printf("Select an option for simulation:\n");
  printf("1. Request (press 1)\n");
  printf("2. Reply (press 2)\n");
  scanf("%d", &option);  

  if(option==1) //We create a Request with the default recommended values
  {  Request = http_CreateRequest(); 

     // ! Optional
     // By example, we can change some parameters
     // We change lamda - parameter for the date generator
     httpRequest_setLambda(Request, 0.7);

     httpRequest_LoadParameters(Request, sink);

     dateGen = httpRequest_GetDateGen(Request);
     filePDU = httpRequest_GetFilePDU(Request);
     server = httpRequest_GetServer(Request);
     PDUSource = httpRequest_GetPDUSource(Request);

  }
  if(option==2)
  {  Reply = http_CreateReply();

     httpReply_LoadParameters(Reply, sink);
      
     dateGen = httpReply_GetDateGen(Reply);
     filePDU = httpReply_GetFilePDU(Reply);
     server = httpReply_GetServer(Reply);
     PDUSource = httpReply_GetPDUSource(Reply);

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
