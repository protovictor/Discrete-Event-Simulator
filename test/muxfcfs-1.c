/*
 * Programme de test du multiplexeur FCFS
 *
 *
 */
#include <stdio.h>     // printf, ...

#include <motsim.h>
#include <muxfcfs.h>
#include <pdu-sink.h>
#include <pdu-source.h>

#define NBSRC 10

int main()
{
   struct muxfcfs_t    * mux;
   struct PDUSink_t    * sink;
   struct PDUSource_t  * source[NBSRC];
   struct probe_t      * sinkInputProbe;

   int n;   

   /* Creation du simulateur */
   motSim_create();

   /* Le puits */
   sink = PDUSink_create();

   /* On place une sonde sur le puits */
   sinkInputProbe = probe_createExhaustive();
   PDUSink_setInputProbe(sink, sinkInputProbe);

   /* Le multiplexeur */
   mux = muxfcfs_create(sink, PDUSink_processPDU);

   /* Les sources */
   for (n = 0; n < NBSRC; n++) {
      source[n] = PDUSource_create(dateGenerator_createExp(5.0), mux, muxfcfs_processPDU);

   }

   /* On active les sources */
   for (n = 0; n < NBSRC; n++) {
      PDUSource_start(source[n]);
   };

   motSim_runUntil(100000.0);

   motSim_printStatus();

   printf("IA = %f\n", probe_IAMean(sinkInputProbe));
   return 0;
}

