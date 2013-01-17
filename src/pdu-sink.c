#include <stdio.h>     // printf
#include <stdlib.h>    // Malloc, NULL, exit...

#include <pdu-sink.h>

struct PDUSink_t {
   struct probe_t * insertProbe;

};

struct PDUSink_t * PDUSink_create()
{
   struct PDUSink_t * result = (struct PDUSink_t *)sim_malloc(sizeof(struct PDUSink_t));

   result->insertProbe = NULL;

   return result;
}

/*
 * Affectation d'une sonde sur les evenements d'insertion
 */
void PDUSink_setInputProbe(struct PDUSink_t * sink, struct probe_t * insertProbe)
{
   sink->insertProbe = insertProbe;
}


void PDUSink_processPDU(struct PDUSink_t * pduSink, getPDU_t getPDU, void * source)
{
   struct PDU_t * pdu = getPDU(source);

   if (pduSink->insertProbe){
      probe_sample(pduSink->insertProbe, PDU_id(pdu));
   }

   if (pdu) {
      PDU_free(pdu);
   }
}


