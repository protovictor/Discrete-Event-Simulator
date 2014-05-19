#include <stdio.h>
#include <motsim.h>
#include <date-generator.h>
#include <random-generator.h>
#include <ndesObject.h>
#include <event.h>
#include <file_pdu.h>
#include <log.h>
#include <pdu.h>
#include <tcp.h>


/*================================================*/
/*      This is a TCP model                       */
/*  In this source is shown how a TCP connection  */
/*  works when transmitting packets from a client */
/*  to a web server (HTTP session)                */
/*================================================*/

        /**  
         *  The client sends a http request to the webserver
         *   The request has information about the content of the webpage
         *   This information reffers to:
         *   - main object size   - Lognormal distributed
         *   - embedded object's number - Gamma distributed
         *   - embedded object's size - Lognormal distributed
         *  The webserver sends the main object and the embedded
         *  objects, in packets of maximum 1500.
         *  Each object is splitted in PDUs of maximum 1500 bytes 
         *  and it is sent to the client.  
         */

struct getRequest_t {
   declareAsNdesObject;
        double mainObjSize;                      //!< the size of the main object of a webpage
        int embObjNr;                            //!< the number of embedded objects
        struct randomGenerator_t *embObjSize;    //!< the size of embedded objects
   };

defineObjectFunctions(getRequest);
struct ndesObjectType_t getRequestType = {
  ndesObjectTypeDefaultValues(getRequest)
}; 


/*---------------------------------------------------------------------*/


struct TCP_client_t {
   declareAsNdesObject;

   //struct dateGenerator_t *time_c;             //!< starting time of a get request - used only in multisession model
   struct randomGenerator_t *mainObjSizeGen;   //!< main object size generator
   struct randomGenerator_t *embObjNrGen;      //!< embedded object number generator
   struct randomGenerator_t *embObjSzGen;      //!< embedded object size
   
   struct PDU_t *pdu;                          //!< the current pdu received from the server
   void *destination;                          //!< the destination of the HTTP_GET request
};

defineObjectFunctions(TCP_client);
struct ndesObjectType_t TCP_clientType = {
  ndesObjectTypeDefaultValues(TCP_client)
}; 


/*----------------------------------------------------------------------*/


struct TCP_server_t {
   declareAsNdesObject;

   struct dateGenerator_t *time_s;              //!< reply transmission starting time 
  

   struct getRequest_t  *HTTP_GET;              //!< the PDU received from the client
   struct PDU_t         *pdu;                   //!< the TCP segment currently transmitted by the server
   struct PDU_t         *nextPdu;               //!< the next pdu that will be transmitted by the server
   int counter;                                 //!< a counter of the transmitted PDUs

   struct probe_t       *PDUGenerationSizeProbe;
   void                 *destination;           //!< the destination where the server sends the PDUs
   processPDU_t         destProcessPDU;         //!< the process that will receive the PDUs
   struct filePDU_t     *filePDU;
};

defineObjectFunctions(TCP_server);
struct ndesObjectType_t TCP_serverType = {
  ndesObjectTypeDefaultValues(TCP_server)
}; 


/*----------------------------------------------------------------------*/


struct getRequest_t* getRequest_Create(double mainsz, int embnr, struct randomGenerator_t *embsz)
{
    struct getRequest_t * result = (struct getRequest_t *)sim_malloc(sizeof(struct getRequest_t));

  result->mainObjSize = mainsz;
  result->embObjNr = embnr;
  result->embObjSize = embsz;
  
 return result;
}


struct TCP_client_t* TCP_clientCreate( //struct dateGenerator_t *dg,
                                      struct randomGenerator_t *maiszg,
                                      struct randomGenerator_t *embnr,
                                      struct randomGenerator_t *embsz,
                                      void *destination)
{   
    struct TCP_client_t* client = (struct TCP_client_t*)sim_malloc(sizeof(struct TCP_client_t));

      //client->time_c = dg;
      client->mainObjSizeGen = maiszg;
      client->embObjNrGen = embnr;
      client->embObjSzGen = embsz; 

      client->destination = destination;

    return client;
}

void TCP_clientSetDestination(struct TCP_client_t * client, void *destination)
{
   client->destination = destination;
}

struct TCP_server_t* TCP_serverCreate(struct dateGenerator_t *dg, void *destination, processPDU_t destProcessPDU)
{
    struct TCP_server_t* server = (struct TCP_server_t*)sim_malloc(sizeof(struct TCP_server_t));

      server->time_s = dg;
      server->HTTP_GET = NULL;
      server->pdu = NULL;
      server->nextPdu = NULL;

      server->PDUGenerationSizeProbe = NULL;
      server->counter = 0;
      server->destination = destination;
      server->destProcessPDU = destProcessPDU;
      server->filePDU = NULL;

    return server;   
}


/*-----------------------------------------------------------*/


void TCP_session_start(struct TCP_client_t* client, struct TCP_server_t* server)
{
   HTTP_GET_send(client, server);
   
   TCP_session_SendPage(server);
}

/**
 * @brief This function generically sends to the server the parameters 
 *        from the http get request. These parameters describes how the
 *        page will be transmitted from server to client
 * @param client  sends the http request
 * @param server  receives the http request from the client
 */

void HTTP_GET_send(struct TCP_client_t* client, struct TCP_server_t* server)
{
  int mainsz;
  int embnr;

  mainsz = (int) (randomGenerator_getNextDouble(client->mainObjSizeGen) * MAX_SIZE) % MAX_SIZE; 
  //printf("main object size: %d bytes\n ", mainsz);

  embnr =  (int)(randomGenerator_getNextDouble(client->embObjNrGen)* MAX_NUMBER);
  //printf("inline objects: %d \n", embnr);

  server->HTTP_GET = getRequest_Create(mainsz, embnr, client->embObjSzGen); 
  
  /* We create the file which will process the PDUs */
  server->filePDU = filePDU_create(client, (processPDU_t)filePDU_processPDU); 
}


void TCP_session_SendPage(struct TCP_server_t* server)
{
  
 
  /* We first create the tcp segment for the main object */ 
     server->pdu = PDU_create(server->HTTP_GET->mainObjSize, NULL);
  
  /* We take a size probe of the main object */
     if (server->PDUGenerationSizeProbe)
        // probe_sample(server->PDUGenerationSizeProbe, (double)PDU_size(server->pdu));
      
     if(server->pdu)  /* if the creation of the pdu was successful, we send it to the client */
     {
        if ((server->destProcessPDU) && (server->destination)) 
         (void)server->destProcessPDU(server->destination,
                                     (getPDU_t)Server_getPDU,
                                     server);
 
     }

     TCP_Send_EmbeddedObjects(server);
    
}


void TCP_Send_EmbeddedObjects(struct TCP_server_t *server)
{
  double date;
  int size;
  struct event_t * event;
 
   if(server->pdu)
     PDU_free(server->pdu);

   server->pdu = server->nextPdu;

   if(server->pdu)
   {

    /* We take samples from the pdu */
       if (server->PDUGenerationSizeProbe) 
        {
         //probe_sample(server->PDUGenerationSizeProbe, (double)PDU_size(server->pdu));
        }
 
    /* We send the pdu to the client */
     (void)server->destProcessPDU(server->destination,
                                     (getPDU_t)Server_getPDU,
                                     server);

   }


   /* We continue the simulation if we have embedded objects left to send to the client */
   if(motSim_getNbRanEvents() <= (Http_getEmbeddedNumber(server->HTTP_GET)+1) )
   {    
        /* We get the size of the pdu */
        do{
         size = (int) (randomGenerator_getNextDouble(server->HTTP_GET->embObjSize) * MAX_SIZE) % MAX_SIZE;
         }while(size <= 0);

     //printf("object size: %d bytes\n", size);
        /* We get the time when the pdu will be transmitted */
        date = dateGenerator_nextDate(server->time_s, motSim_getCurrentTime());
        
        server->pdu = PDU_create(size, NULL);
       
        /* We create an event to run at time specified in date */
        event = event_create((eventAction_t)TCP_Send_EmbeddedObjects, server, date);
        /* We add the event to the simulator */
        motSim_addEvent(event);
   }
}


struct PDU_t * Server_getPDU(struct TCP_server_t *server)
{
   struct PDU_t *pdu = server->pdu;
   server->pdu = NULL;
   return pdu;
}

/*
int client_processPDU( void *destination,
                       getPDU_t getPDU,
                       void *source )
{



}
*/


int Http_getEmbeddedNumber(struct getRequest_t *request)
{
  return request->embObjNr;
}



























