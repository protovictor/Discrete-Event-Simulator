#include <stdlib.h>    // Malloc, NULL, exit, ...
#include <assert.h>
#include <strings.h>   // bzero, bcopy, ...
#include <stdio.h>     // printf, ...

#include <file_pdu.h>
#include <sched_ks.h>
#include <dvb-s2-ll.h>


/*----------------------------------------------------------------------*/
/*                                                                      */
/*----------------------------------------------------------------------*/
int main() {
   struct filePDU_t * files[NB_MODCOD][NB_QOS_LEVEL];
   struct DVBS2ll_t * dvbs2ll;

   int mc;
   int q, m;

   /* Création de la couche 2 DVB-S2 */
   dvbs2ll = DVBS2ll_create();

   /* Creation des files */
   for (m = 0; m < NB_MODCOD; m++) {
      for (q = 0; q < NB_QOS_LEVEL; q++) {
         files[m][q] = filePDU_create();
      }
   }

   /* Insersion de paquets dans le systeme */
   filePDU_insert(files[0][0], PDU_create(64, NULL));
   filePDU_insert(files[0][0], PDU_create(64, NULL));
   filePDU_insert(files[0][0], PDU_create(1500, NULL));

   filePDU_insert(files[0][1], PDU_create(1500, NULL));
   filePDU_insert(files[0][1], PDU_create(1500, NULL));
   filePDU_insert(files[0][1], PDU_create(64, NULL));

   filePDU_insert(files[1][0], PDU_create(1500, NULL));
   filePDU_insert(files[1][0], PDU_create(64, NULL));
   filePDU_insert(files[1][0], PDU_create(1500, NULL));

   filePDU_insert(files[1][1], PDU_create(64, NULL));
   filePDU_insert(files[1][1], PDU_create(1500, NULL));
   filePDU_insert(files[1][1], PDU_create(64, NULL));


   scheduler_knapsack_exhaustif(files, dvbs2ll);
    
   return 1;
}

/*
 * Résultat obtenu
 *


************ MODCOD 0 ************

[ 64/  64.00  64/  64.00  1500/1500.00 ]  [ 1500/3000.00  1500/3000.00  64/ 128.00 ]  
[ 1500/3000.00  64/ 128.00  1500/3000.00 ]  [ 64/ 256.00  1500/6000.00  64/ 256.00 ]  


************ Solution ************
 2  0 
 0  3 
  Interet 6640.00
  Volume 1756/1900


************ MODCOD 1 ************

[ 1500/1500.00  64/  64.00  1500/1500.00 ]  [ 64/ 128.00  1500/3000.00  64/ 128.00 ]  


************ Solution ************
 0  0 
 0  3 
  Interet 3256.00
  Volume 1628/1900

*/
