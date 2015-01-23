/**
 * @file muxdemux.c
 * @brief Simple mux/demux test
 */

#include <math.h>
#include <muxdemux.h>
#include <date-generator.h>
#include <pdu-source.h>
#include <pdu-sink.h>
#include <file_pdu.h>

#define NB_CHANNELS 5
#define NB_PDU 7

struct dateSize sequence[NB_CHANNELS][NB_PDU+1] = {
   {
     {0.0, 1},
     {1.0, 1},
     {2.0, 1},
     {3.0, 1},
     {4.0, 1},
     {5.0, 1},
     {6.0, 1},
     {0, 0}
   },
   {
     {0.0, 10},
     {1.0, 10},
     {2.0, 10},
     {3.0, 10},
     {4.0, 10},
     {5.0, 10},
     {6.0, 10},
     {0, 0}
   },
   {
     {0.0, 100},
     {1.0, 100},
     {2.0, 100},
     {3.0, 100},
     {4.0, 100},
     {5.0, 100},
     {6.0, 100},
     {0, 0}
   },
   {
     {0.0, 1000},
     {1.0, 1000},
     {2.0, 1000},
     {3.0, 1000},
     {4.0, 1000},
     {5.0, 1000},
     {6.0, 1000},
     {0, 0}
   },
   {
     {0.0, 10000},
     {1.0, 10000},
     {2.0, 10000},
     {3.0, 10000},
     {4.0, 10000},
     {5.0, 10000},
     {6.0, 10000},
     {0, 0}
   }
};

int main()
{
   struct PDUSource_t * sources[NB_CHANNELS];
   struct PDUSink_t   * sinks[NB_CHANNELS];
   struct probe_t     * pr[NB_CHANNELS];

   struct filePDU_t * link;

   struct muxDemuxSender_t * sm;
   struct muxDemuxReceiver_t * rd;

   struct muxDemuxSenderSAP_t * ssap;
   struct muxDemuxReceiverSAP_t * rsap;
   int n;
   int result = 0;

   motSim_create();

   // Sinks creation
   for (n = 0; n < NB_CHANNELS ; n++){
      sinks[n] = PDUSink_create();
      pr[n] = probe_createExhaustive();
      PDUSink_addInputProbe(sinks[n], pr[n]);
   }

   // Demultiplexer
   rd = muxDemuxReceiver_create();

   // Create a SAP for each channel
   for (n = 0; n < NB_CHANNELS ; n++){
      // New SAP creation
      rsap = muxDemuxReceiver_createNewSAP(rd, n+1, sinks[n], PDUSink_processPDU);
      if (rsap == NULL) {
         printf("Could not use receiver SAPI %d\n", n+1);
      }
   }

   // link
   link = filePDU_create(rd, muxDemuxReceiver_processPDU);

   // Multiplexer
   sm = muxDemuxSender_create(link, filePDU_processPDU);

   // Sources creation
   for (n = 0; n < NB_CHANNELS ; n++){
      // New SAP creation
      ssap = muxDemuxSender_createNewSAP(sm, n+1);
      if (ssap == NULL) {
         printf("Could not use sender SAPI %d\n", n+1);
      }
      sources[n] = PDUSource_createDeterministic(sequence[n], ssap, muxDemuxSender_processPDU);
      PDUSource_start(sources[n]);
   }

   motSim_runUntilTheEnd();

   printf("\n");
   for (n = 0; n < NB_CHANNELS ; n++){
      printf("Received on %d : %ld (%f)\n", n, probe_nbSamples(pr[n]), probe_nbSamples(pr[n])?probe_nbSamples(pr[n])*probe_mean(pr[n]):0.0);
      if ((probe_nbSamples(pr[n]) != NB_PDU) || (probe_mean(pr[n]) != pow(10.0, n))) {
 	result = 1;
      }
   }

   return result;
}
