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

#define NB_PDU 10000
int main() {
   struct PDUSource_t     * sourcePDU;
   struct dateGenerator_t * dateGenExp;
   struct filePDU_t       * filePDU;
   unsigned int             taille = 1;
   double                   un = 1.0;
   int                      result;

   /* Creation du simulateur */
   motSim_create();

   /* La file */
   filePDU = filePDU_create(NULL, NULL);
   filePDU_setMaxSize(filePDU, NB_PDU);

   /* Création d'un générateur de date */
   dateGenExp = dateGenerator_createExp(1.0);

   /* La source */
   sourcePDU = PDUSource_create(dateGenExp, filePDU, filePDU_processPDU);

   /* On génère des PDU de taille 1 */
   PDUSource_setPDUSizeGenerator(sourcePDU, randomGenerator_createUIntDiscreteProba(1, &taille, &un));

   /* On active la source */
   PDUSource_start(sourcePDU);
   motSim_runUntil(1.2*NB_PDU);  // WARNING : creer un PDUSource_startAt()  et un stopAt()

   motSim_printStatus();
   printf("%d PDU dans la file\n", filePDU_length(filePDU));
   printf("Debit : %f\n", filePDU_getInputThroughput(filePDU));

   if (filePDU_length(filePDU) == NB_PDU) {
      result = 0;
   } else {
      result = 1;
   }

   return result;
}
