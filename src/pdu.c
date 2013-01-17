#include <stdlib.h>    // Malloc, NULL, ...
#include <assert.h>

#include <pdu.h>

static int pduNB = 0;

// Pour suivre un peu l'origine des PDU
struct probe_t * PDU_createProbe;
struct probe_t * PDU_reuseProbe;
struct probe_t * PDU_mallocProbe;
struct probe_t * PDU_freeProbe;

// Pointeur sur une PDU libre (pour acc�lerer alloc/free)
struct PDU_t * firstFreePDU = NULL;

int PDU_size(struct PDU_t * PDU){
   return PDU->taille;
}

int PDU_id(struct PDU_t * PDU){
   return PDU->id;
}

void * PDU_private(struct PDU_t * PDU){
   return PDU->data;
}

motSimDate_t PDU_getCreationDate(struct PDU_t * PDU){
   return PDU->creationDate;
}

struct PDU_t * PDU_create(int size, void * private) 
{
   struct PDU_t * PDU ;

   if (firstFreePDU){
      PDU = firstFreePDU;
      firstFreePDU = PDU->next;
      assert(PDU);
      probe_sample(PDU_reuseProbe, (double)PDU->id);
   } else {
      PDU = (struct PDU_t *)sim_malloc(sizeof(struct PDU_t));
      assert(PDU);
      probe_sample(PDU_mallocProbe, (double)PDU->id);
   }

   PDU->taille = size;
   PDU->id = pduNB ++;
   PDU->data = private;
   PDU->creationDate = motSim_getCurrentTime();
   PDU->next = NULL;
   PDU->prev = NULL;

   probe_sample(PDU_createProbe, (double)PDU->id);

   printf_debug(DEBUG_FILE, "PDU %d created (size %d)\n", PDU->id, PDU->taille);

   return PDU;
}

/*
 * Destruction d'une PDU. Les donnees privees doivent avoir
 * ete detruites par l'appelant.
 */
void PDU_free(struct PDU_t * pdu)
{
   probe_sample(PDU_freeProbe, (double)pdu->id);

   if (pdu != NULL) {
      pdu->next = firstFreePDU;
      firstFreePDU = pdu;
   }
}