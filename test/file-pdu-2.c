/*----------------------------------------------------------------------*/
/*   Test de NDES : tests des files de PDU à capacité limitée.          */
/*----------------------------------------------------------------------*/

#include <stdlib.h>    // Malloc, NULL, exit, ...
#include <assert.h>
#include <strings.h>   // bzero, bcopy, ...
#include <stdio.h>     // printf, ...

#include <file_pdu.h>
#include <pdu-source.h>
#include <srv-gen.h>
#include <date-generator.h>

int main() {
   struct PDUSource_t     * sourcePDU;
   struct dateGenerator_t * dateGenExp;
   struct filePDU_t       * filePDU;
   int                      taille = 1;
   double                   un = 1.0;

   /* Creation du simulateur */
   motSim_create();

   /* La file */
   filePDU = filePDU_create(NULL, NULL);
   filePDU_setMaxSize(filePDU, 10000);

   /* Création d'un générateur de date */
   dateGenExp = dateGenerator_createExp(1.0);

   /* La source */
   sourcePDU = PDUSource_create(dateGenExp, filePDU, filePDU_processPDU);

   /* On génère des PDU de taille 1 */
   PDUSource_setPDUSizeGenerator(sourcePDU, randomGenerator_createUIntDiscrete(1, &taille, &un));

   /* On active la source */
   PDUSource_start(sourcePDU);
   motSim_runUntil(25000.0);

   motSim_printStatus();
   printf("%d PDU dans la file\n", filePDU_length(filePDU));
   printf("Debit : %f\n", filePDU_getInputThroughput(filePDU));

   return 1;
}
