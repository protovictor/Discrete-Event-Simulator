/**
 * @file Test de NDES : démonstration du DRR
 *
 */

#include <pdu-source.h>
#include <sched_drr.h>
#include <pdu-sink.h>

int main() {
  struct dateSize sequence[] = {
     {10.0, 100},
     {20.2, 200},
     {0, 0}
   };

   struct PDUSource_t * sourcePDU;
   struct PDUSink_t   * sink;

   // Initialisation du simulateur
   motSim_create();

   // Le puits
   sink = PDUSink_create();

   // Initialisation de la source
   sourcePDU = PDUSource_createDeterministic(sequence, sink, PDUSink_processPDU);
   PDUSource_start(sourcePDU);

   motSim_runUntilTheEnd();

   motSim_printStatus();

   return 1;
}
