#include <motsim.h>
#include <gnuplot.h>
#include <http.h>
#include <probe.h>

//void tracer(struct probe_t *pr, char *name, int nbBar);


 int main()
{

  struct probe_t           *sejProbe, *iaProbe, *srvProbe;
  struct TCP_client_t      *Client;
  struct TCP_server_t      *WebServer;
  struct randomGenerator_t *mainObjSize, *embObjNr, *embObjSize; 
  struct dateGenerator_t   *dateGen;
  double alpha, beta;
  double duration = 100000.0;
  
  motSim_create();

  alpha = 1.31;
  beta = 1.41;

  mainObjSize = randomGenerator_createDoubleLognormal(alpha, beta);
  randomGenerator_setMinMax(mainObjSize, 0, 1);
  alpha = -0.75; 
  beta = 2.36;

  embObjSize = randomGenerator_createDoubleLognormal(alpha, beta);
  randomGenerator_setMinMax(embObjSize, 0, 1);
 
  alpha = 0.24; 
  beta = 23.42;
  embObjNr = randomGenerator_createDoubleGamma(alpha, beta);
  randomGenerator_setMinMax(embObjNr, 0, 1);

  Client = TCP_clientCreate(mainObjSize, embObjNr, embObjSize); 
 
 /*------------------------------------------------------------*/
  alpha = 0.5;
  beta = 4.44;
  dateGen = dateGenerator_createWeibull(alpha, beta);

  WebServer = TCP_serverCreate(dateGen, Client, filePDU_processPDU);

  TCP_clientSetDestination(Client, WebServer);

  //------------------------- Sensors -------------------
 
 // A sensor of inter-arrivals 
       iaProbe = probe_createExhaustive();
       dateGenerator_addInterArrivalProbe(dateGen, iaProbe);

 // A sensor for the journey/sejour 
    //   sejProbe = probe_createExhaustive(); 
    //   filePDU_addSejournProbe(filePDU, sejProbe);

 // A sensor for the service time 
    //   srvProbe = probe_createExhaustive();
    //   srvGen_addServiceProbe(server, srvProbe);

 //--------------------------------------


  TCP_session_start(Client, WebServer);
  motSim_runUntil(duration);
  motSim_printStatus();

  printf("Average Inter-arrival: %f \n", probe_mean(iaProbe));


return 0;
}
