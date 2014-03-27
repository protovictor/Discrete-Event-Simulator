/**
 * @file hdlc.c
 */
#include <hdlc.h>

/**
 * @brief 
 */
struct hdlc_t {
   void * outLink; //!< Output lowlevel
   processPDU_t sendPDU;

   struct PDU_t * window;

   int state;
};

#define HDLC_STATE_DISCONNECTED   0
#define HDLC_WAITING_SRM          1

struct hdlc_t * hdlc_create()
{
   struct hdlc_t * result = sim_malloc(sizeof(struct hdlc_t));

   printf_debug(DEBUG_ALWAYS, "IN\n");

   result->outLink = NULL;
   result->sendPDU = NULL;
   result->state = HDLC_STATE_DISCONNECTED;

   printf_debug(DEBUG_ALWAYS, "OUT\n");

   return result;
}

void hdlc_setOutLink(struct hdlc_t * h,
		     void * destination,
		     processPDU_t destProcessPDU)
{
   printf_debug(DEBUG_ALWAYS, "IN\n");

   h->outLink = destination;
   h->sendPDU = destProcessPDU;

   printf_debug(DEBUG_ALWAYS, "OUT\n");
}

/**
 * @brief Initialization of a connection
 */
int hdlc_connect(struct hdlc_t * h, int mode)
{
   printf_debug(DEBUG_ALWAYS, "IN\n");

   h->window = PDU_create(0, NULL);  //!< Create a S..M Frame
   printf_debug(DEBUG_ALWAYS, "PDU created\n");

   h->sendPDU(h->outLink, hdlc_getPDU, h);   

   printf_debug(DEBUG_ALWAYS, "OUT\n");

   return 0; //!< OK
}

/**
 * @brief Initialization of an entity waiting for an incomming
 * connection 
 */
void hdlc_init(struct hdlc_t * h)
{
   printf_debug(DEBUG_ALWAYS, "IN\n");

   printf_debug(DEBUG_ALWAYS, "OUT\n");
}

/**
 * @brief Emission d'un message
 */
int hdlc_send(struct hdlc_t * h, struct PDU_t * data)
{
   struct PDU_t * pdu;

   pdu = PDU_create(0, data); //!< HDLC I-PDU creation

   h->sendPDU(h->outLink, hdlc_getPDU, pdu);

   return 0; //!< OK
}

/**
 * @brief Traitement d'une PDU envoyée par l'entité homologue
 */
void hdlc_processPDU(struct hdlc_t * h,
                     getPDU_t getPDU,
                     void * source)
{
   struct PDU_t * pdu;

   pdu = getPDU(source);

   printf_debug(DEBUG_ALWAYS, "pdu received\n");
}

/**
 * @brief Fonction d'entrée/sortie
 */
struct PDU_t * hdlc_getPDU(struct hdlc_t * h)
{
   return h->window;
}

/**
 * @brief Traitement d'une SDU soumise par la couche supérieure
 */
void hdlc_processSDU(struct hdlc_t * h,
                     getPDU_t getPDU,
                     void * source)
{
   struct PDU_t * sdu;

   sdu = getPDU(source);

   h->window = PDU_create(0, sdu);

   printf_debug(DEBUG_ALWAYS, "sdu received\n");
}

