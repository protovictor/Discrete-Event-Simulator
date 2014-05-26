#include <stdio.h>
#include "event.h"
#include "ftp.h"
#include "ndesObject.h"

/**
 *  FTP is a protocol used for transferring files
 * The model recommended in cdma2000 Evaluation Methodology uses
 * as important parameters in a file transfer:
 * the date for the beggining of a FTP session
 * the size of the file that needs to be transmited
 *
 */
struct FTP_Session_t{
    declareAsNdesObject;
    struct dateGenerator_t    *readingTime;      //!< the time between two ftp sessions
    struct randomGenerator_t  *sizeGen;          //!< the random generator for the size of files

    struct PDUList_t  *firstPdu;         //!< the first pdu in the list that need to be transmitted
    struct PDUList_t  *lastPdu;          //!< the last pdu in the list that need to be transmitted
    int PDUnr;                           //!< the number of PDUs in the list
    double duration;                     //!< the duration of a session

    void *destination;                    //!< the destination of the files
    processPDU_t destProcessPDU;          //!< a pointer to the function that will process the files

    int SessionID;
};

defineObjectFunctions(FTP_Session);
struct ndesObjectType_t FTP_SessionType = {
  ndesObjectTypeDefaultValues(FTP_Session)
};


struct PDUList_t {
   declareAsNdesObject;

  struct PDU_t *pdu;
  struct PDUList_t *nextPdu;
  };
defineObjectFunctions(PDUList);
struct ndesObjectType_t PDUListType = {
  ndesObjectTypeDefaultValues(PDUList)
};


struct FTP_Session_t* FTP_Session_Create(struct dateGenerator_t *dateGen,
                                          struct randomGenerator_t *sizeGen,
                                          void *destination,
                                          processPDU_t destProcessPDU)
{
    struct FTP_Session_t* Session = (struct FTP_Session_t*)sim_malloc(sizeof(struct FTP_Session_t));

    Session->readingTime = dateGen;
    Session->sizeGen = sizeGen;

    Session->firstPdu = NULL;
    Session->lastPdu = NULL;
    Session->PDUnr = 0;
    Session->duration = 0.0;

    Session->destination = destination;
    Session->destProcessPDU = destProcessPDU;

    Session->SessionID = 0;

 return Session;
}

void FTP_Session_Start(struct FTP_Session_t *Session)
{
    struct event_t *event;
    double nextDate;
    int size;

    /* We get the date for the next session */
    nextDate = dateGenerator_nextDate(Session->readingTime, motSim_getCurrentTime());

    /* The size for the file we want to transfer in this session */
    do{
    size = (int) (randomGenerator_getNextDouble(Session->sizeGen) * MAX_FileSize) % MAX_FileSize;
    }while(size <= 0);

    /* The duration of the current session */
    Session->duration = nextDate - motSim_getCurrentTime();

    /* We split the file into multiple smaller PDUs to transmit them */
    Session = FTP_Session_PDUList_Create(Session, size);

    /* We transmit the current file  through a sequence of PDU transmissions */
    FTP_Transmit_File(Session);

    /* We increment the SessionID value */
    Session->SessionID +=1;

    /* We create an event for the next session */
      event = event_create((eventAction_t)FTP_Session_Start, Session, nextDate);
    /* We add the event to the simulator */
      motSim_addEvent(event);

}


struct FTP_Session_t* FTP_Session_PDUList_Create(struct FTP_Session_t *Session, int size)
{
    int n, i,lastsize;

    n = size / MSS;
    lastsize = size % MSS;
    if (lastsize!=0)
     n+=1;
    Session->PDUnr = n;


    for(i=1; i<n; i++)
  {
    struct PDUList_t *node = (struct PDUList_t *)sim_malloc(sizeof(struct PDUList_t));
    node->pdu = PDU_create(MSS, NULL);
    node->nextPdu = NULL;
    if(Session->firstPdu == NULL)
    {
      Session->firstPdu = node;
      Session->lastPdu = node;
      Session->lastPdu->nextPdu = NULL;
    }
    else
    {
      Session->lastPdu->nextPdu = node;
      Session->lastPdu = node;
    }
  }

    if(lastsize != 0)
    {
     struct PDUList_t *node = (struct PDUList_t *)sim_malloc(sizeof(struct PDUList_t));
     node->pdu = PDU_create(lastsize, NULL);
     Session->lastPdu->nextPdu = node;
     Session->lastPdu = node;
    }

return Session;
}


void FTP_Transmit_File(struct FTP_Session_t *Session)
{

   struct event_t *event;
    /* The transmission of PDUs is periodical */

    double date, period;
    period = Session->duration / Session->PDUnr;
    date = motSim_getCurrentTime() + period;


    if(Session->firstPdu)
    {
       if ((Session->destProcessPDU) && (Session->destination))
         (void)Session->destProcessPDU(Session->destination,
                                     (getPDU_t)FTP_Session_getPDU,
                                     Session);


       event = event_create((eventAction_t)FTP_Transmit_File, Session, date);
       motSim_addEvent(event);
    }

}

struct PDU_t* FTP_Session_getPDU(struct FTP_Session_t *Session)
{
   struct PDUList_t *pdunode = Session->firstPdu;

   if(pdunode->nextPdu != NULL)          /* if the list has more than one node */
   Session->firstPdu = pdunode->nextPdu;  /* the first node gets the value of the node after him */
   else
   Session->firstPdu = NULL;

   return pdunode->pdu;
}

int FTP_GetSessionNr(struct FTP_Session_t *Session)
{
   return Session->SessionID;
}


