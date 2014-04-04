/**
 * @file muxdemux.c
 * @brief Simple mux/demux test
 */

#include <muxdemux.h>
#include <date-generator.h>
#include <pdu-source.h>
#include <pdu-sink.h>

#define NB_CHANNELS 10

int main()
{
   struct PDUSource_t * sources[NB_CHANNELS];
   struct PDUSink_t   * sinks[NB_CHANNELS];
    
   struct muxDemuxSender_t * sm;
   struct muxDemuxReceiver_t * rd;

   struct muxDemuxSenderSAP_t * ssap;
   int n;

   // Sinks creation
   for (n = 0; n < NB_CHANNELS ; n++){
      sinks[n] = PDUSink_create();
   }

   // Demultiplexer

   // link

   // Multiplexer
   sm = muxDemuxSender_Create(sinks[0], PDUSink_processPDU);

   // Sources creation
   for (n = 0; n < NB_CHANNELS ; n++){
      // New SAP creation
      ssap = muxDemuxSender_createNewSAP(sm, n+1);
      if (ssap == NULL) {
         printf("Could not use SAPI %d\n", n+1);
      }
      sources[n] = PDUSource_create(dateGenerator_createExp(10.0), ssap, muxDemuxSenderProcessPDU);
   }

   return 0;
}
