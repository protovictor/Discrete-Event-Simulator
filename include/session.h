#include <stdio.h>
#include <stdlib.h>
#include <event.h>
#include <pdu-source.h>
#include <random-generator.h>
#include <date-generator.h>
#include <pdu.h>
#include <motsim.h>

 struct replyPacket_t;
 struct replyQueue_t;

 struct replyQueue_t * replyQueue_Create(struct dateGenerator_t *dateGen,
                                        void * destination,
                                        processPDU_t destProcessPDU); 

 void replyPacket_setPDUSizeMGenerator(struct replyQueue_t * reply_packet, 
                                       struct randomGenerator_t *rg);
 void replyPacket_setPDUSizeIGenerator(struct replyQueue_t * reply_packet,
                                       struct randomGenerator_t *nObjGen,
                                       struct randomGenerator_t *sizeIGen);
 void replyPacket_addPDUGenerationSizeProbe(struct replyQueue_t * replyPacket, 
                                            struct probe_t *PDUGenerationSizeProbe);
   
 void replyQueue_buildNew(struct replyQueue *reply);
                                        
 void replyQueue_start(struct replyQueue_t *reply); 

 struct replyPacket_t* replyPacket_getPackets(struct replyQueue_t *reply); 
 


