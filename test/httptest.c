#include <stdio.h>
#include "motsim.h"
#include "http.h"
#include "pdu-sink.h"
#include "srv-gen.h"


 int main()
{

  struct TCP_client_t      *Client;
  struct TCP_server_t      *WebServer;
  struct TCP_Session_t     *Session;

  struct randomGenerator_t *mainObjSize, *embObjNr, *embObjSize;
  struct dateGenerator_t   *dateGen, *dg;

  struct probe_t           *sejProbe, *iaProbe, *bsProbe;
  struct PDUSink_t         *sink;
  struct filePDU_t         *filePDU;
  struct srvGen_t          *base_station;  /* the base station who forwards the packets to the webserver */

  double duration = 100000.0;        /* the maximum duration of the simulation */
  double mean = 50;                  /* mean = 50ms - exponential distribution*/

  motSim_create();
  sink = PDUSink_create();


  base_station = srvGen_create(sink, (processPDU_t)PDUSink_processPDU);
  srvGen_setServiceTime(base_station, serviceTimeExp, mean);

  filePDU = filePDU_create(base_station, (processPDU_t)srvGen_processPDU);

  /* We create random generators for the request parameters*/
  mainObjSize = randomGenerator_createDouble();
  embObjNr = randomGenerator_createDouble();
  embObjSize = randomGenerator_createDouble();
  /* And we set them to default distributions and parameters */
  Request_LoadDefault(mainObjSize, embObjNr, embObjSize);

  dg = dateGenerator_createPeriodic(15000);


  /* Just a generic client , in fact it is created just to hold the parameters of the requested page */
  Client = TCP_clientCreate(dg, mainObjSize, embObjNr, embObjSize, sink, (processPDU_t)PDUSink_processPDU);

 /*------------------------------------------------------------*/
  double alpha = 0.5;
  double beta = 4.44;
  dateGen = dateGenerator_createWeibull(alpha, beta);

  WebServer = TCP_serverCreate(dateGen, filePDU, (processPDU_t)filePDU_processPDU);


  //------------------------- Sensors -------------------

 // A sensor of inter-arrivals of embedded objects
       iaProbe = probe_createExhaustive();
       dateGenerator_addInterArrivalProbe(dateGen, iaProbe);

 // A sensor for the journey/sejour
       sejProbe = probe_createExhaustive();
       filePDU_addSejournProbe(filePDU, sejProbe);

 // A sensor for the base station service time
       bsProbe = probe_createExhaustive();
       srvGen_addServiceProbe(base_station, bsProbe);

 //------------------------------------------------------

  Session = TCP_Session_Create(Client, WebServer);
  TCP_Session_start(Session);

  motSim_runUntil(duration);
  motSim_printStatus();

  printf("Average Inter-arrival: %f \n", probe_mean(iaProbe));
  printf("Average time of journey: %f \n", probe_mean(sejProbe));
  printf("Average time of service: %f \n", probe_mean(bsProbe));


return 0;
}

