#include <stdio.h>
#include <stdlib.h>
#include "file_pdu.h"
#include "srv-gen.h"
#include "probe.h"
#include "session.h"


/**
 *    A multiple session traffic model (it can be used on any kind of traffic model)
 *    SessionList is not really a linked list, it's just a structure who holds a list of sessions and their parameters
 *    In each session there are made some requests ("represented" by PDUs)
 *
 */

struct PDUSourceList_t {
  struct PDUSource_t *PDUSource;
  struct PDUSourceList_t *nextPDUSource;
};

struct SessionList_t {          /* A structure used to store the sessions */
   declareAsNdesObject;
   struct PDUSourceList_t    *PDUSourceList;      //!< Requests made in a session by a user
   struct dateGenerator_t    *dateGen;        //!< Date generator for the session starting time
   struct dateGenerator_t    *PDUdateGen;     //!< Date generator for the request/reply starting time (made in a session)
   struct randomGenerator_t  *SizeGen;        //!< Random generator for the requests/replies size
   struct filePDU_t          *filePDU;        //!< Passes the PDUs from sink to server (is actually a list of created PDU's)

   /* For the specific list of sessions */
   int specific;
   struct PDUSourceList_t    *firstPDUSource;
   struct PDUSourceList_t    *lastPDUSource;
   };

defineObjectFunctions(SessionList);
struct ndesObjectType_t SessionListType = {
  ndesObjectTypeDefaultValues(SessionList)
};



struct SessionList_t* SessionList_Create(struct dateGenerator_t    * dateGen,
                                         struct dateGenerator_t    * PDUdateGen,
                                         struct randomGenerator_t  * SizeGen,
                                         struct filePDU_t          * filePDU)
{
    struct SessionList_t *result = (struct SessionList_t*)sim_malloc(sizeof(struct SessionList_t));

    ndesObjectInit(result, SessionList);
    result->PDUSourceList = NULL;
    result->dateGen = dateGen;             /* The time between two sessions */
    result->PDUdateGen = PDUdateGen;       /* The time between two requests */
    result->SizeGen = SizeGen;             /* The size of the requests packets */
    result->filePDU = filePDU;             /* The file that stores the PDU's and passes them to the server/web server */
    result->specific = 0;

    result->firstPDUSource = NULL;
    result->lastPDUSource = NULL;


 return result;
}

void SessionList_Start(struct SessionList_t *SessionList)
{
    double date;
    struct event_t *event;
    printf_debug(DEBUG_SRC, " IN\n");

    /* We create a new session of requests/replies */
     if(SessionList->specific==0)
     {
        SessionList->PDUSourceList = (struct PDUSourceList_t *)malloc(sizeof(struct PDUSourceList_t));
        SessionList->PDUSourceList->PDUSource = PDUSource_create(SessionList->PDUdateGen,
                                                                 SessionList->filePDU,
                                                                 (processPDU_t)filePDU_processPDU);
        PDUSource_setPDUSizeGenerator(SessionList->PDUSourceList->PDUSource, SessionList->SizeGen);
     }
     else
     {
     if(SessionList->firstPDUSource != NULL)
        SessionList->PDUSourceList->PDUSource = SessionList_GetSession(SessionList);
     }

    /* If the creation was a succes...*/
      if(SessionList->PDUSourceList->PDUSource)
       {
          /* We take a sample of the first request/reply size for the simulation */
          if(PDUSource_getPDUGenerationSizeProbe(SessionList->PDUSourceList->PDUSource))
            {
               probe_sample(PDUSource_getPDUGenerationSizeProbe(SessionList->PDUSourceList->PDUSource) ,
                      (double)PDU_size(PDUSource_getNextPDU(SessionList->PDUSourceList->PDUSource)) );

            }
          /* And we start the session */
          PDUSource_start(SessionList->PDUSourceList->PDUSource);
       }

      printf_debug(DEBUG_SRC, "building next Session ...\n");

    /* We obtain the next session start date */
      date = dateGenerator_nextDate(SessionList->dateGen,  motSim_getCurrentTime() );
    /* If it is a valid date and it is not a deterministic time */
     if(date >= motSim_getCurrentTime() && SessionList->dateGen)
     {
         /* We create another session of requests/replies */
         if(SessionList->specific == 0)
         {
           SessionList->PDUSourceList = (struct PDUSourceList_t *)malloc(sizeof(struct PDUSourceList_t));
           SessionList->PDUSourceList->PDUSource = PDUSource_create(SessionList->PDUdateGen,
                                                                 SessionList->filePDU,
                                                                 (processPDU_t)filePDU_processPDU);
           PDUSource_setPDUSizeGenerator(SessionList->PDUSourceList->PDUSource, SessionList->SizeGen);


         /* We create an event for the start of the session at the generated date... */
         event = event_create((eventAction_t)SessionList_Start, SessionList, date);
         /* And we add it to the simulator */
         motSim_addEvent(event);

         }
         else /* If it's a specific list of sessions */
         {
           if(SessionList->firstPDUSource != NULL)
           {
              /* We get the session from the list */
                SessionList->PDUSourceList->PDUSource = SessionList_GetSession(SessionList);

              /* We create an event for the start of the session at the generated date... */
              event = event_create((eventAction_t)SessionList_Start, SessionList, date);
              /* And we add it to the simulator */
              motSim_addEvent(event);
           }
          else
           {
            printf_debug(DEBUG_SRC, "Aborted (Expired event)!\n");
           }
         }

     }
     else  /* If it is not a valid date, then we print a message(when debugging). If not in debug mode, nothing happens */
     {
         printf_debug(DEBUG_SRC, "Aborted (Expired event)!\n");
     }

 printf_debug(DEBUG_SRC, "OUT \n");
}


struct SessionList_t* SessionList_CreateSpecific()
{
    struct SessionList_t *result = (struct SessionList_t*)sim_malloc(sizeof(struct SessionList_t));

    ndesObjectInit(result, SessionList);
    result->PDUSourceList = NULL;
    result->dateGen = NULL;              /* The time between two sessions */
    result->PDUdateGen = NULL;           /* The time between two requests */
    result->SizeGen = NULL;              /* The size of the requests packets */
    result->filePDU = NULL;              /* The file that stores the PDU's and passes them to the server/web server */

    result->specific = 1;

    result->firstPDUSource = NULL;
    result->lastPDUSource = NULL;

return result;
}


struct SessionList_t* SessionList_AddSession(struct SessionList_t* SessionList, struct PDUSource_t* PDUSource)
{

  if(SessionList->specific == 1)
  {
       struct PDUSourceList_t *newSession = (struct PDUSourceList_t*)malloc(sizeof(struct PDUSourceList_t));
       newSession->PDUSource= PDUSource;

   if(SessionList->firstPDUSource == NULL)            // if the list is empty
       {
         SessionList->PDUSourceList =  newSession;
         SessionList->firstPDUSource = SessionList->PDUSourceList;
         SessionList->lastPDUSource = SessionList->PDUSourceList;
       }
        else
       {
         SessionList->lastPDUSource->nextPDUSource = newSession;
         SessionList->lastPDUSource = newSession;
       }

    SessionList->lastPDUSource->nextPDUSource = NULL;

  }
  else
  {
     printf_debug(DEBUG_SRC, "Cannot add a specific session on a randomly generated list of sessions!");
  }

return SessionList;
}


struct PDUSource_t* SessionList_GetSession(struct SessionList_t* SessionList)
{
   struct PDUSourceList_t* result = (struct PDUSourceList_t*)malloc(sizeof(SessionList->firstPDUSource));
   result = SessionList->firstPDUSource;
  // printf("Source: %p \n", SessionList->firstPDUSource->PDUSource);
   struct PDUSourceList_t* node = result;

  if(node->nextPDUSource != NULL)
    {
       SessionList->firstPDUSource = node->nextPDUSource;
    }
   else
    {
      SessionList->PDUSourceList->PDUSource = NULL;
      SessionList->firstPDUSource = NULL;
      SessionList->lastPDUSource = NULL;
    }
   return result->PDUSource;
}


void SessionList_setStartDate(struct SessionList_t* SessionList, struct dateGenerator_t* dateGen)
{
    SessionList->dateGen = dateGen;
}

void SessionList_setSizeGen(struct SessionList_t* SessionList, struct randomGenerator_t* SizeGen)
{
    SessionList->SizeGen = SizeGen;
}

void SessionList_setFilePDU(struct SessionList_t* SessionList, struct filePDU_t* filePDU)
{
    SessionList->filePDU = filePDU;
}

void SessionList_setPDUdateGen(struct SessionList_t* SessionList, struct dateGenerator_t* PDUdateGen)
{
    SessionList->PDUdateGen = PDUdateGen;
}
















