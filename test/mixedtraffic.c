#include <motsim.h>
#include <gnuplot.h>
#include <probe.h>
#include <session.h>
#include <srv-gen.h>
#include <pdu.h>
#include <file_pdu.h>

/*==================================================================================*/
/*   A mixed traffic model which consists of multiple sessions.                     */
/*   In each session a client sends packets to a server.                            */
/*   It's just an example on how to use the multiple session model, it doesn't have */
/*   a specific correspondace in real-life.                                         */
/*   The multiple session model can represent a series of similar sessions (with    */
/*   randomly generated parameters) or some previously defined sessions. It can     */
/*   represent the case when a server has many purposes in the same time.           */
/*   The example below consists from 10 sessions, the first 5 have some parameters, */
/*   and the last 5 have other parameters.                                          */
/*==================================================================================*/

void tracer(struct probe_t *pr, char *name, int nbBar);

int main()
{
  struct probe_t            *sejProbe, *iaProbe, *srvProbe;
  struct filePDU_t          *filePDU;
  struct dateGenerator_t    *dateGen, *dtGen;
  struct randomGenerator_t  *sizeGen, *szGen;
  struct srvGen_t           *server;
  struct PDUSink_t          *sink;
  struct PDUSource_t        *PDUSource;
  struct SessionList_t      *Sessions;
    
  int i;
    
  motSim_create();
  sink = PDUSink_create();

  /* Some parameters we use for each session */
  double alpha = 0.43;
  double beta = 0.89;
  double lambda = 0.79; 
  double galpha = 0.16; 
  double gbeta = 0.66;
  double lalpha = 0.92;
  double lbeta = 0.76;

  double debit = 1000;  /* a parameter of the server , the server service time is proportional to this */ 
 
  /* We create a specific list of sessions */
  Sessions = SessionList_CreateSpecific();
 
  /* The date generator for the each session starting time */
  dtGen = dateGenerator_createLognormal(alpha, beta);
  

  /* The date generator for the departure of each packet */
  dateGen = dateGenerator_createExp(lambda); 
  
 
  /* The random generators which gives us random size for the packets */
  sizeGen = randomGenerator_createDouble();
  randomGenerator_setDistributionGamma(sizeGen, galpha, gbeta);
  szGen = randomGenerator_createDouble(); 
  randomGenerator_setDistributionLognormal(szGen, lalpha, lbeta);

  server = srvGen_create(sink, (processPDU_t)PDUSink_processPDU);
  srvGen_setServiceTime(server, serviceTimeProp, 1.0/debit);
  
  /* We create the file which holds the PDUs that will be passed to the server */
  filePDU = filePDU_create(server, (processPDU_t)srvGen_processPDU); 
  

   SessionList_setStartDate(Sessions, dtGen);
   SessionList_setPDUdateGen(Sessions, dateGen);
   SessionList_setFilePDU(Sessions, filePDU);

 /**
  * Lets say we have 5 sessions, each with packet's size given randomly, 
  * by a Gamma distribution
  * and another 5 by a Lognormal distribution
  */

  for (i=1; i<=5; i++)
  {
   PDUSource = PDUSource_create(dateGen, filePDU, (processPDU_t)filePDU_processPDU); 
   PDUSource_setPDUSizeGenerator(PDUSource, sizeGen); 
  
   Sessions = SessionList_AddSession(Sessions, PDUSource);  
  }
 
  for (i=1; i<=6; i++)
  {
   PDUSource = PDUSource_create(dateGen, filePDU, (processPDU_t)filePDU_processPDU); 
   PDUSource_setPDUSizeGenerator(PDUSource, szGen); 
   
   Sessions = SessionList_AddSession(Sessions, PDUSource); 
  }
  

 /* We can put some sensors to give us some statistics about the simulation */  
 //------------------------- Sensors -------------------
 
 // A sensor of inter-arrivals of the sessions
      iaProbe = probe_createExhaustive();
       dateGenerator_addInterArrivalProbe(dtGen, iaProbe);

 // A sensor for the journey/sejour 
       sejProbe = probe_createExhaustive(); 
       filePDU_addSejournProbe(filePDU, sejProbe);

 // A sensor for the service time 
       srvProbe = probe_createExhaustive();
       srvGen_addServiceProbe(server, srvProbe);

 //-----------------------------------------------------


  /* Now we can start the sessions */

   SessionList_Start(Sessions);
 
   motSim_runUntil(30000.0);   // 30 seconds 
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


