#include <motsim.h>
#include <gnuplot.h>
#include <http.h>
#include <probe.h>

/*=========================================================*/
/*  A multiple http-session model                          */
/*  The model is represented by a series of http sessions, */
/*  all of the same type (replies or requests, not mixed). */
/*  All the requests are made on the same server and all   */
/*  the replies are sent to the same client                */
/*=========================================================*/


void tracer(struct probe_t *pr, char *name, int nbBar);

 int main()
{
  struct probe_t          *sejProbe, *iaProbe, *srvProbe;
  struct httpRequest_t    *Request;
  struct httpReply_t      *Reply;
  struct filePDU_t        *filePDU;
  struct dateGenerator_t  *dateGen, *dtGen;
  struct randGenerator_t  *sizeGen;
  struct srvGen_t         *server;
  struct PDUSink_t        *sink;
  struct SessionList_t    *Sessions;

  int option;
  int avg_nb_session = 60;       /* the average number of sessions in a simulation */
  double duration = 60000.0;     /* the duration of a simulation */
  double period = duration / (double)avg_nb_session; 
 
  motSim_create();
  sink = PDUSink_create();
  
  printf("Select an option for simulation:\n");
  printf("1. Request (press 1)\n");
  printf("2. Reply (press 2)\n");
  scanf("%d", &option);  

  if(option==1) //We create a Session of Requests with the default recommended values
  {  
     /* We create a single session, just to get the default values we need later */
     Request = http_CreateRequest();   

     /* We load the default parameters */    
     httpRequest_LoadParameters(Request, sink);

     /* We get the values we need for the simulator */
     dateGen = httpRequest_GetDateGen(Request);    
     filePDU = httpRequest_GetFilePDU(Request);
     server = httpRequest_GetServer(Request);
     sizeGen = httpRequest_GetSizeGen(Request);
    
     /* We create a date generator for each session start time */
     dtGen = dateGenerator_createPeriodic(period); 
     /* We create the list of sessions */
     Sessions = SessionList_Create(dtGen, dateGen, sizeGen, filePDU);

  }
  if(option==2) //We create a Session of Replies with the default recommended values
  {  
     /* We create a single session, just to get the default values we need later */
     Reply = http_CreateReply();

     /* We load the default parameters */   
     httpReply_LoadParameters(Reply, sink);
      
     /* We get the values we need for the simulator */
     dateGen = httpReply_GetDateGen(Reply);
     filePDU = httpReply_GetFilePDU(Reply);
     server = httpReply_GetServer(Reply);
     sizeGen = httpReply_GetSizeGen(Reply);      

     /* We create a date generator for each session start time */
     dtGen = dateGenerator_createPeriodic(period);
   
     /* We create the list of sessions */
     Sessions = SessionList_Create(dtGen, dateGen, sizeGen, filePDU);

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

  /* We start building sessions */
   SessionList_Start(Sessions);
 
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
