/*----------------------------------------------------------------------*/
/*   Test de NDES : tests de la gestion des files de PDU                */
/*----------------------------------------------------------------------*/

#include <stdlib.h>    // Malloc, NULL, exit, ...
#include <assert.h>
#include <strings.h>   // bzero, bcopy, ...
#include <stdio.h>     // printf, ...

#include <file_pdu.h>
#include <pdu-source.h>
#include <pdu-sink.h>
#include <srv-gen.h>
#include <date-generator.h>

int main() {
   struct PDUSource_t     * sourcePDU;
   struct dateGenerator_t * dateGenExp;
   struct filePDU_t       * filePDU;
   struct PDUSink_t       * sink;
   struct probe_t         * sp;

   /* Creation du simulateur */
   motSim_create();

   /* Le puits */
   sink = PDUSink_create();

   /* La file */
   filePDU = filePDU_create(sink, PDUSink_processPDU);

   /* Création d'un générateur de date */
   dateGenExp = dateGenerator_createExp(1.0);

   /* La source */
   sourcePDU = PDUSource_create(dateGenExp, filePDU, filePDU_processPDU);

   /* Les sondes */
   sp = probe_createExhaustive();
   filePDU_addSejournProbe(filePDU, sp);

   /* On active la source */
   PDUSource_start(sourcePDU);
   motSim_runUntil(25000.0);

   motSim_printStatus();
   printf("%d PDU dans la file\n", filePDU_length(filePDU));

   printf("Temps moyen de sejour dans la file = %f\n", probe_mean(sp));
   return 1;
}
