#include <srv-gen.h>
#include <session.h>  
#include <ndesObject.h>

struct replyPacket_t {  
                       declareAsNdesObject;  //!< C'est un ndesObject
                       struct PDU_t *mainObj;         // main object
                       struct PDUSource_t *inlineObj; // ninline PDUs
                     };
 
struct replyQueue_t { 
                       declareAsNdesObject;  //!< C'est un ndesObject
                       struct dateGenerator_t * dateGen;
                       struct randomGenerator_t * sizeMGen, *sizeIGen, *nObjGen;
                        
                       void * destination;
                       processPDU_t destProcessPDU;

                       struct probe_t *  PDUGenerationSizeProbe;
                       struct replyPacket_t *repPDU;
                       struct replyPacket_t *next_repPDU;

                       struct dateSize * sequence; //!< Pour le cas déterministe
                       int detNextIdx; //!< Prochain indice dans le cas déterministe
                     };

defineObjectFunctions(replyQueue);
struct ndesObjectType_t replyQueueType = {
  ndesObjectTypeDefaultValues(replyQueue)
};

defineObjectFunctions(replyPacket);
struct ndesObjectType_t replyPacketType = {
  ndesObjectTypeDefaultValues(replyPacket)
};


 struct replyQueue_t * replyQueue_Create(struct dateGenerator_t *dateGen,
                                        void * destination,
                                        processPDU_t destProcessPDU)
{

   struct replyQueue_t * result = (struct replyQueue_t *)
              sim_malloc(sizeof(struct replyQueue_t));

   ndesObjectInit(result, replyQueue);

   result->repPDU = NULL;
   result->next_repPDU = NULL;
   result->dateGen = dateGen;
   result->destProcessPDU = destProcessPDU;
   result->destination = destination;
   result->sizeMGen = NULL;
   result->sizeIGen = NULL;
   result->nObjGen  = NULL;
   result->PDUGenerationSizeProbe = NULL;

   // Pour les sources déterministes (à refaire un jour)
   result->sequence = NULL;
   result->detNextIdx = 0;

   // Ajout à la liste des choses à réinitialiser avant une prochaine simu
   motsim_addToResetList(result, (void (*)(void *))replyQueue_start);
   
   return result;
}


 void replyPacket_setPDUSizeMGenerator(struct replyQueue_t * reply_packet, 
                                        struct randomGenerator_t *rg)
{   
  reply_packet->sizeMGen = rg;
}

 void replyPacket_setPDUSizeIGenerator(struct replyQueue_t * reply_packet, 
                                       struct randomGenerator_t *nrg,
                                       struct randomGenerator_t *srg)
{
  reply_packet->nObjGen = nrg;
  reply_packet->sizeIGen = srg;
}
/*
 void replyQueue_buildNew(struct replyQueue_t *reply)
 {
    double date;
    struct event_t * event;
    unsigned int size = 0; 
    long totalPDUsize;   

     if(reply->repPDU)
     {    
       // printf_debug(DEBUG_SRC, " Destruction de %d \n ", reply->repPDU->id);
        PDU_free(reply->repPDU);

     }

     reply->repPDU = reply->nextPdu;

     if(reply->repPDU)
     {
              
       if(reply->PDUGenerationSizeProbe)
       {  
          totalPDUsize = reply->replyPacket->repPDU->mainObj->size + 
                         reply->replyPacket->repPDU->inlineObj->objNum * 
                         reply->replyPacket->repPDU->inlineObj->sizeIGen;
          probe_sample(reply->PDUGenerationSizeProbe, (double)totalPDUsize); 
       }
       
       if( (reply->destProcessPDU) && (reply->destination) )
       {
          
           (void)reply->destProcessPDU(reply->destination, (getPDU_t)replyPacket_getPackets, reply);

       }
  
     }
     
    // no deterministic source !!!   ... to do deterministic source
      date = dateGenerator_nextDate(reply->dateGen, motSim_getCurrentTime());
      size = reply->sizeGen?( randomGenerator_getNextUInt(reply->sizeMGen) +
                              randomGenerator_getNextUInt(reply->sizeIGen) * 
                              randomGenerator_getNextUInt(reply->nObjGen) ):0;

    //without date >= motSim_getCurrentTime() ... to do

 }


  struct replyPacket_t* replyPacket_getPackets(struct replyQueue_t *replyQueue)
{
  struct replyPacket_t *newpacket = replyQueue->repPDU;
  replyQueue->repPDU = NULL;



  long size = replyQueue->sizeMGen + replyQueue->nObjGen * replyQueue->sizeIGen;


 // printf_debug(DEBUG_SRC, "releasing packet %d (size %d) \n", replyQueue->repPDU->id, size);
  

  return newpacket;
}


  void replyQueue_start(struct replyQueue_t *reply)
{
   if (reply->repPDU)
   {
      PDU_free(repPDU->mainObj);
      PDU_free(repPDU->inlineObj->pdu);
      reply->repPDU = NULL;
   }

      replyQueue_buildNew(reply->replyPDU); //de facut

}


*/





