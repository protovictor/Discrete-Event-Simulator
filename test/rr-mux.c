/**
 * @file rr-mux.c
 * @brief This simple programm tests the Round Robin scheduler, the
 * muxdemux entity and the filters
 *
 * Multiple sources (NB_SOURCES) are multiplexed in a signle channel
 * with the help of a multiplexing entity. Multiple chanels
 * (NB_CHANNELS) are processed by a round robin scheduler, sent
 * through a simple link to a sink.
 *
 * All the sources have the same PDU rate, but source s in channel c
 * sends PDU of constant size s*c Kbits so that bit rates differ.
 *
 * With the help of filters, throughput is measured for each source
 * without demultiplexing.
 */

#include <math.h>
#include <muxdemux.h>
#include <date-generator.h>
#include <pdu-source.h>
#include <sched_rr.h>
#include <pdu-sink.h>
#include <file_pdu.h>
#include <ll-simplex.h>
#include <pdu-filter.h>

#define NB_CHANNELS 5
#define NB_SOURCES  5

#define THROUGHPUT 1000000.00
#define PROPAGATION 0.010
#define LAMBDA 10.0
#define FRAMESIZE 1000
#define DURATION 10000.0

int main()
{
   struct PDUSource_t * sources[NB_CHANNELS][NB_SOURCES];
   struct PDUSink_t   * sink;

   struct probe_t     * prSrc[NB_CHANNELS][NB_SOURCES];
   struct probe_t     * prChn[NB_CHANNELS];

   struct filePDU_t   * queues[NB_CHANNELS]; // Sources muxed in a single channel
   struct llSimplex_t * link;
   struct rrSched_t   * sched;
   struct muxDemuxSender_t    * sm[NB_CHANNELS];
   struct muxDemuxSenderSAP_t * ssap;

   char name[32];

   int c, s;
   int result = 0;
   double sum;


   motSim_create();

   // Sink creation
   sink = PDUSink_create();

   // Link
   link = llSimplex_create(sink, PDUSink_processPDU, THROUGHPUT, PROPAGATION);

   //   // Link layer probe
   //prLnk = probe_createExhaustive();
   
   // Scheduler
   sched = rrSched_create(link, llSimplex_processPDU);

   // Queues, muxes and sources
   for (c = 0; c < NB_CHANNELS ; c++){
      queues[c] = filePDU_create(sched, rrSched_processPDU);
      rrSched_addSource(sched, queues[c], filePDU_getPDU);

      // Queue level probe
      prChn[c] = probe_createTimeSliceThroughput(DURATION/100.0);
      sprintf(name, "Channel %d", c);
      probe_setName(prChn[c], name);

      filePDU_addExtractSizeProbe(queues[c], prChn[c]);

      // Multiplexer
      sm[c] = muxDemuxSender_create(queues[c], filePDU_processPDU);

      // The sources
      for (s = 0; s < NB_SOURCES; s++) {
         // New SAP creation
         ssap = muxDemuxSender_createNewSAP(sm[c], s+1);
         if (ssap == NULL) {
            printf("Could not use sender SAPI %d\n", c+1);
         }

         // Specific probe
         prSrc[c][s] =  probe_createTimeSliceThroughput(DURATION/100.0);
         probe_setFilter(prSrc[c][s], muxDemuxSender_createFilterFromSAP(ssap));
	 sprintf(name, "Source %d-%d", c, s);
         probe_setName(prSrc[c][s], name);
	 filePDU_addExtractSizeProbe(queues[c], prSrc[c][s]);

         // Source itself
         sources[c][s] = PDUSource_create(dateGenerator_createExp(LAMBDA), ssap, muxDemuxSender_processPDU);
         PDUSource_setPDUSizeGenerator(sources[c][s], randomGenerator_createUIntConstant((c+1)*(s+1)*FRAMESIZE));
         PDUSource_start(sources[c][s]);
      }
   }

   motSim_runUntil(DURATION);
   printf("\n");

   // Print and check the results
   for (c = 0; c < NB_CHANNELS ; c++){
      sum = 0.0;
      printf("Channel %d : ", c);
      for (s = 0; s < NB_SOURCES; s++) {
         printf("%10.1f  ", probe_mean(prSrc[c][s]));
 	 sum += probe_mean(prSrc[c][s]);
         if (s > 0) {
	   if (fabs(probe_mean(prSrc[c][s])/(s+1) - probe_mean(prSrc[c][s-1])/s) > probe_mean(prSrc[c][s])/20.0) {
	       printf("!");
               result = 1;
	    } else {
	       printf(" ");
	    }
	 }
      }
      printf(" : %10.1f", probe_mean(prChn[c]));
      if (fabs(sum - probe_mean(prChn[c])) > 1.0) {
         printf(" ERROR \n");
         result = 1;
      } else {
         printf("\n");
      }
   }

   return result;
}
