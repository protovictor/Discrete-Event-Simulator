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

   /* Creation du simulateur */
   motSim_create();

   /* Le puits */
   sink = PDUSink_create();

   /* La file */
   filePDU = filePDU_create(NULL, NULL);

   /* Création d'un générateur de date */
   dateGenExp = dateGenerator_createExp(1.0);

   /* La source */
   sourcePDU = PDUSource_create(dateGenExp, filePDU, filePDU_processPDU);

   /* Les sondes */
   //   sp = probe_create(NULL);
   //filePDU_setSejournProbe(filePDU, sp);

   /* On active la source */
   PDUSource_start(sourcePDU);
   motSim_runUntil(5000.0);

   motSim_printStat();

   //   printf("Temps moyen de sejour dans la file = %f\n", probe_mean(sp));
   return 1;
}
