#include <stdio.h>
#include "motsim.h"
#include "ndesObject.h"
#include "event.h"
#include "pdu.h"
#include "tcp.h"


/* A List o PDU's */
struct PDUList_t {
   declareAsNdesObject;

  struct PDU_t *pdu;
  struct PDUList_t *nextPdu;
  };

defineObjectFunctions(PDUList);
struct ndesObjectType_t PDUListType = {
  ndesObjectTypeDefaultValues(PDUList)
};


struct getRequest_t {
   declareAsNdesObject;
        int mainObjSize;                         //!< the size of the main object of a webpage
        int embObjNr;                            //!< the number of embedded objects
        struct randomGenerator_t *embObjSize;    //!< the size of embedded objects
   };

defineObjectFunctions(getRequest);
struct ndesObjectType_t getRequestType = {
  ndesObjectTypeDefaultValues(getRequest)
};



struct TCP_client_t {
   declareAsNdesObject;

   struct dateGenerator_t *time_c;             //!< starting time of a get request - used only in multisession model
   struct randomGenerator_t *mainObjSizeGen;   //!< main object size generator
   struct randomGenerator_t *embObjNrGen;      //!< embedded object number generator
   struct randomGenerator_t *embObjSzGen;      //!< embedded object size

   void *destination;
   processPDU_t destProcessPDU;

};

defineObjectFunctions(TCP_client);
struct ndesObjectType_t TCP_clientType = {
  ndesObjectTypeDefaultValues(TCP_client)
};



struct TCP_server_t {
   declareAsNdesObject;

   struct dateGenerator_t *time_s;              //!< reply transmission starting time

   struct getRequest_t  *HTTP_GET;              //!< the PDU received from the client

   struct PDUList_t *firstPdu;
   struct PDUList_t *lastPdu;

   int PDUnr;                                   //!< the number of PDUs in the list
   double duration;                             //!< time of transmition of an embedded object

   void                 *destination;           //!< the destination where the server sends the PDUs
   processPDU_t         destProcessPDU;         //!< the process that will receive the PDUs

};

defineObjectFunctions(TCP_server);
struct ndesObjectType_t TCP_serverType = {
  ndesObjectTypeDefaultValues(TCP_server)
};

struct TCP_Session_t {
   declareAsNdesObject;

   struct TCP_client_t *Client;
   struct TCP_server_t *Server;
};


defineObjectFunctions(TCP_Session);
struct ndesObjectType_t TCP_SessionType = {
  ndesObjectTypeDefaultValues(TCP_Session)
};


/*----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/

/*==========================================*/
/*      The functions used for creation     */
/*==========================================*/


struct getRequest_t* getRequest_Create(int mainsz, int embnr, struct randomGenerator_t *embsz)
{
    struct getRequest_t * result = (struct getRequest_t *)sim_malloc(sizeof(struct getRequest_t));

  result->mainObjSize = mainsz;
  result->embObjNr = embnr;
  result->embObjSize = embsz;

 return result;
}

struct TCP_client_t* TCP_clientCreate( struct dateGenerator_t *dg,
                                      struct randomGenerator_t *maiszg,
                                      struct randomGenerator_t *embnr,
                                      struct randomGenerator_t *embsz,
                                      void *destination,
                                      processPDU_t destProcessPDU)
{
    struct TCP_client_t* client = (struct TCP_client_t*)sim_malloc(sizeof(struct TCP_client_t));

      client->time_c = dg;
      client->mainObjSizeGen = maiszg;
      client->embObjNrGen = embnr;
      client->embObjSzGen = embsz;

      client->destination = destination;
      client->destProcessPDU = destProcessPDU;

    return client;
}

struct TCP_server_t* TCP_serverCreate(struct dateGenerator_t *dg, void *destination, processPDU_t destProcessPDU)
{
    struct TCP_server_t* server = (struct TCP_server_t*)sim_malloc(sizeof(struct TCP_server_t));

      server->time_s = dg;
      server->HTTP_GET = NULL;

      server->firstPdu = NULL;
      server->lastPdu = NULL;
      server->PDUnr = 0;
      server->duration = 0;


      server->destination = destination;
      server->destProcessPDU = destProcessPDU;


    return server;
}


/*-----------------------------------------------------------*/


/*============================================*/
/*      The functions used to send packets    */
/*============================================*/

/**
 *  Create TCP Session
 */

struct TCP_Session_t* TCP_Session_Create(struct TCP_client_t *client, struct TCP_server_t *server)
{
    struct TCP_Session_t *Session = (struct TCP_Session_t*)sim_malloc(sizeof(struct TCP_Session_t));
    Session->Client = client;
    Session->Server = server;

 return Session;
}


/**
 *   Start TCP session
 *   (ignoring the connection establishment - three way handshake)
 */
void TCP_Session_start(struct TCP_Session_t *Session)
{
  double date_nextSession;
  struct event_t *event;


   /* A client sends a request to a webserver */
   HTTP_GET_send(Session->Client, Session->Server);


   /* The server responds with the corresponding page */
   TCP_Session_SendPage(Session->Server);


   date_nextSession = dateGenerator_nextDate(Session->Client->time_c, motSim_getCurrentTime());

   event = event_create((eventAction_t)TCP_Session_start, Session, date_nextSession);
   motSim_addEvent(event);

}


/**
 * This function is used to "send" a HTTP_GET request from a client
 * to a webserver
 */

void HTTP_GET_send(struct TCP_client_t* client, struct TCP_server_t* server)
{
  int mainsz;
  int embnr;

  mainsz = (int) (randomGenerator_getNextDouble(client->mainObjSizeGen) * MAX_SIZE) % MAX_SIZE;

  embnr =  (int)(randomGenerator_getNextDouble(client->embObjNrGen)* MAX_NUMBER);

  server->HTTP_GET = getRequest_Create(mainsz, embnr, client->embObjSzGen);

}


void TCP_Session_SendPage(struct TCP_server_t* server)
{
  double date;

  /* The main object can exceed the MSS, so we split it into multilpe PDUs which
   * will be transmitted serially to the client
   */

    server = TCP_server_PDUList_Create(server, server->HTTP_GET->mainObjSize);


  /* The transmission date for the next embedded object */
    date = dateGenerator_nextDate(server->time_s, motSim_getCurrentTime());

  /* The duration for transmitting the current object */
    server->duration = date - motSim_getCurrentTime();


  /* We send the main object of the webpage */
    TCP_Transmit_Object(server);

  /* We send the embedded objects of the webpage */
    TCP_Send_EmbeddedObjects(server);
}

struct TCP_server_t* TCP_server_PDUList_Create(struct TCP_server_t* server, int size)
{

  int n, i;
  int lastsize = size % MSS;
  /* We calculate the number of PDUs in which we will split the packet with the size */
  n = size / MSS;
  if( lastsize != 0)
  n+=1;
  server->PDUnr = n;

  for(i=1; i<n; i++)
  {
    struct PDUList_t *node = (struct PDUList_t *) sim_malloc(sizeof(struct PDUList_t));
    node->pdu = PDU_create(MSS, NULL);
    node->nextPdu = NULL;
    if(server->firstPdu == NULL)
    {
      server->firstPdu = node;
      server->lastPdu = node;
      server->lastPdu->nextPdu = NULL;
    }
    else
    {
      server->lastPdu->nextPdu = node;
      server->lastPdu = node;
    }
  }

    if(lastsize != 0)
    {
     struct PDUList_t *node = (struct PDUList_t *) sim_malloc(sizeof(struct PDUList_t));
     node->pdu = PDU_create(lastsize, NULL);
     server->lastPdu->nextPdu = node;
     server->lastPdu = node;
    }

return server;
}

void TCP_Transmit_Object(struct TCP_server_t *server)
{
    struct event_t *event;
    /* The transmission of PDUs is periodical */

    double date, period;
    period = server->duration / server->PDUnr;
    date = motSim_getCurrentTime() + period;


    if(server->firstPdu)
    {
       if ((server->destProcessPDU) && (server->destination))
         (void)server->destProcessPDU(server->destination,
                                     (getPDU_t)Server_getPDU,
                                     server);


       event = event_create((eventAction_t)TCP_Transmit_Object, server, date);
       motSim_addEvent(event);
    }

}


void TCP_Send_EmbeddedObjects(struct TCP_server_t *server)
{
   struct event_t *event;
   struct dateGenerator_t *dtGen;
   double date;
   double period;
   int size;
   int check;

   /* We get the size(in bytes) for the next embedded object*/
   do
   {
    size = (int)(randomGenerator_getNextDouble(server->HTTP_GET->embObjSize) * MAX_SIZE ) % MAX_SIZE;
   }while(size<=0);


   /* We fragment the object into several PDUs of maximum size equal to MSS */
   server = TCP_server_PDUList_Create(server, size);

   /* The transmission date for the next embedded object */
   date = dateGenerator_nextDate(server->time_s, motSim_getCurrentTime());

   /* The duration for transmitting the current object */
   server->duration = date - motSim_getCurrentTime();


   /* We transmit the current object */
   TCP_Transmit_Object(server);

   /* We decrement the number of embedded objects that we need to transmit */
    TCP_setEmbeddedNumber(server->HTTP_GET, server->HTTP_GET->embObjNr -1);
    check = TCP_getEmbeddedNumber(server->HTTP_GET);

   /* If we have embedded objects left to transmit, we continue the simulation */
   if(check!=0)
   {
    /* We create an event for the transmission of the next embedded object */
    event = event_create((eventAction_t)TCP_Send_EmbeddedObjects, server, date);
    /* We add it to the simulator */
    motSim_addEvent(event);
   }
}


struct PDU_t * Server_getPDU(struct TCP_server_t *server)
{
   struct PDUList_t *pdunode = server->firstPdu;

   if(pdunode->nextPdu != NULL)          /* if the list has more than one node */
   server->firstPdu = pdunode->nextPdu;  /* the first node gets the value of the node after him */
   else
   server->firstPdu = NULL;

   return pdunode->pdu;
}


int TCP_getEmbeddedNumber(struct getRequest_t *request)
{
  return request->embObjNr;
}


void TCP_setEmbeddedNumber(struct getRequest_t *request, int value)
{
  request->embObjNr = value;
}



void Request_LoadDefault(struct randomGenerator_t *mainsz,
                          struct randomGenerator_t *embnr,
                          struct randomGenerator_t *embsz)
{

  randomGenerator_setDistributionLognormal(mainsz, 1.31, 1.41);

  randomGenerator_setDistributionGamma(embnr, 0.24, 23.42);

  randomGenerator_setDistributionLognormal(embsz, -0.75, 2.36);

}
