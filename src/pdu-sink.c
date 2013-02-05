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


int PDUSink_processPDU(void * s, getPDU_t getPDU, void * source)
{
   printf_debug(DEBUG_PDU, "in\n");
  
   struct PDUSink_t * pduSink = (struct PDUSink_t * )s;
   struct PDU_t * pdu = getPDU(source);

   // Si c'est juste pour tester si je suis pret
   if ((getPDU == NULL) || (source == NULL)) { 
      printf_debug(DEBUG_PDU, "c'etait un test\n");
      return 1;
   }

   if (pduSink->insertProbe){
      probe_sample(pduSink->insertProbe, PDU_id(pdu));
   }

   if (pdu) {
      PDU_free(pdu);
   }

   printf_debug(DEBUG_PDU, "out\n");
   return 1;
}


