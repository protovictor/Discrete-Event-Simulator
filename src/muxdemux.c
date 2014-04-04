/**
 */
#include <limits.h>   // UINT_MAX
#include <muxdemux.h>
#include <ndesObject.h>
#include <ndesObjectFile.h>

/**
 * @brief A multiplexing sender
 *
 */
struct muxDemuxSender_t {
   void * destination;
   processPDU_t destProcessPDU;

   struct ndesObjectFile_t * sapList; //<! Sorted in ascending identifier
};

/**
 * @brief A multiplexer service access point
 */
struct muxDemuxSenderSAP_t {
   declareAsNdesObject; //< C'est un ndesObject 
   unsigned int identifier; //<! The SAPI
   struct muxDemuxSender_t * sender; //<! The corresponding sender
};

/**
 * @brief An encapsulation structure
 */
struct muxDemuxEncaps_t {
   unsigned int sapi;
   struct PDU_t * pdu;
};

struct muxDemuxEncaps_t * muxDemuxEncaps_create(struct muxDemuxSenderSAP_t * sap, struct PDU_t * pdu)
{
  struct muxDemuxEncaps_t * result = (struct muxDemuxEncaps_t * )sim_malloc(sizeof(struct muxDemuxEncaps_t ));

   result->sapi = sap->identifier;
   result->pdu = pdu;

   return result;
}

void muxDemuxEncaps_delete(struct muxDemuxEncaps_t * encaps)
{
   free(encaps);
}


/**
 * @brief Définition des fonctions spécifiques liées au ndesObject
 */
defineObjectFunctions(muxDemuxSenderSAP);

/**
 * @brief Les SAP sont aussi des ndesObject
 */
struct ndesObjectType_t  muxDemuxSenderSAP= {
   ndesObjectTypeDefaultValues(muxDemuxSenderSAP)
};

/**
 * @brief Sender (multiplexer) creator
 */
struct muxDemuxSender_t * muxDemuxSender_Create(void * destination,
						processPDU_t destProcessPDU)
{
   struct muxDemuxSender_t * result = (struct muxDemuxSender_t *)sim_malloc(sizeof(struct muxDemuxSender_t ));

   result->destination = destination;
   result->destProcessPDU = destProcessPDU;
   result->sapList = ndesObjectFile_create(&muxDemuxSenderSAP);
 
   return result;
}

/**
 * @brief Creation of a new Service Access Point
 * @param sender A pointer to the multiplexer structure
 * @param newSAPI The desired identifier (non zero) or unspecified
 * (zero)
 * @result A new SAP or NULL (identifier unavailable)
 */
struct muxDemuxSenderSAP_t * muxDemuxSender_createNewSAP(struct muxDemuxSender_t * sender,
							 unsigned int newSAPI)
{
   unsigned int nextSAPI = 1;

   struct ndesObjectFileIterator_t *i;
   struct muxDemuxSenderSAP_t * s; //!< For iteration
   struct muxDemuxSenderSAP_t * p; //!< The SAP after which to insert
   struct muxDemuxSenderSAP_t * result = NULL;

   // If the chosen SAPI is non zero, we check for its availability,
   // else we search the next available value. 
   i = ndesObjectFile_createIterator(sender->sapList);
   s = (struct muxDemuxSenderSAP_t *)ndesObject_getPrivate(ndesObjectFile_iteratorGetNext(i));
   while (s != NULL) {
      // Looking for an available identifier
      if (s->identifier == nextSAPI) {
         if (nextSAPI == UINT_MAX) {
            ndesObjectFile_deleteIterator(i);
            return NULL;
  	 }
         nextSAPI++;
      }
      // Aborting on "collisions"
      if ((newSAPI != 0) && (s->identifier == newSAPI)) {
	 ndesObjectFile_deleteIterator(i);
         return NULL;
      }
      s = (struct muxDemuxSenderSAP_t *)ndesObject_getPrivate(ndesObjectFile_iteratorGetNext(i));
   }
   ndesObjectFile_deleteIterator(i);
   // The new SAP can be created
   if (newSAPI == 0) {
      newSAPI = nextSAPI;
   }

   // Creation and initilisation
   result = (struct muxDemuxSenderSAP_t *)sim_malloc(sizeof(struct muxDemuxSenderSAP_t));
   result->identifier = newSAPI;
   result->sender = sender;

   // Now we search for its position within the file
   p = NULL; // A priori it is the first
   i = ndesObjectFile_createIterator(sender->sapList);
   s = (struct muxDemuxSenderSAP_t *)ndesObject_getPrivate(ndesObjectFile_iteratorGetNext(i));
   while ((s != NULL) && (s->identifier < newSAPI)) {
      p=s;
      s = (struct muxDemuxSenderSAP_t *)ndesObject_getPrivate(ndesObjectFile_iteratorGetNext(i));
   } 
   ndesObjectFile_deleteIterator(i);

   // Insertion
   if (p) {
      ndesObjectFile_insertAfter(sender->sapList, p, result);
   } else {
      ndesObjectFile_insert(sender->sapList, result);
   }
   return result;
}

/**
 * @brief The sender input fonction
 */
int muxDemuxSenderProcessPDU(void * receiver,
			    getPDU_t getPDU,
			    void * source)
{
   struct muxDemuxSenderSAP_t * sap = (struct muxDemuxSenderSAP_t * )receiver;
   struct muxDemuxSender_t    * sender = receiver->sender;
   struct PDU_t * pdu;
   struct muxDemuxEncaps_t *encaps;

   // Is it just a test ?
   if (source == NULL) {
      return sender->destProcessPDU(sender->destination, NUL);
   } else {
      pdu = getPDU(source);
      // Encapsulation (overheadless)
      encaps = muxDemuxEncaps_create(sap, pdu);
      return sender->destProcessPDU(sender->destination, PDU_create(PDU_size(pdu), encaps));
   }
}

/*********************************************************************************
            RECEIVER SIDE
 */

/**
 * @brief A demultiplexing receiver
 *
 */
struct muxDemuxReceiver_t {
   struct ndesObjectFile_t * sapList; //<! Sorted in ascending identifier
};

/**
 * @brief A multiplexer service access point
 */
struct muxDemuxReceiverSAP_t {
   declareAsNdesObject; //< C'est un ndesObject 
   unsigned int identifier; //<! The SAPI
   struct muxDemuxReceiver_t * receiver; //<! The corresponding sender
};

/**
 * @brief Receiver (demultiplexer) creator
 */
struct muxDemuxReceiver_t * muxDemuxReceiver_Create()
{
   struct muxDemuxReceiver_t * result = (struct muxDemuxReceiver_t *)sim_malloc(sizeof(struct muxDemuxReceiver_t ));

   result->sapList = ndesObjectFile_create(&muxDemuxReceiverSAP);
 
   return result;
}

/**
 * @brief Creation of a new Service Access Point
 * @param sender A pointer to the multiplexer structure
 * @param newSAPI The desired identifier (non zero) or unspecified
 * (zero)
 * @result A new SAP or NULL (identifier unavailable)
 */
struct muxDemuxReceiverSAP_t * muxDemuxReceiver_createNewSAP(struct muxDemuxReceiver_t * sender,
							 unsigned int newSAPI)
{
   unsigned int nextSAPI = 1;

   struct ndesObjectFileIterator_t *i;
   struct muxDemuxReceiverSAP_t * s; //!< For iteration
   struct muxDemuxReceiverSAP_t * p; //!< The SAP after which to insert
   struct muxDemuxReceiverSAP_t * result = NULL;

   // If the chosen SAPI is non zero, we check for its availability,
   // else we search the next available value. 
   i = ndesObjectFile_createIterator(sender->sapList);
   s = (struct muxDemuxReceiverSAP_t *)ndesObject_getPrivate(ndesObjectFile_iteratorGetNext(i));
   while (s != NULL) {
      // Looking for an available identifier
      if (s->identifier == nextSAPI) {
         if (nextSAPI == UINT_MAX) {
            ndesObjectFile_deleteIterator(i);
            return NULL;
  	 }
         nextSAPI++;
      }
      // Aborting on "collisions"
      if ((newSAPI != 0) && (s->identifier == newSAPI)) {
	 ndesObjectFile_deleteIterator(i);
         return NULL;
      }
      s = (struct muxDemuxReceiverSAP_t *)ndesObject_getPrivate(ndesObjectFile_iteratorGetNext(i));
   }
   ndesObjectFile_deleteIterator(i);
   // The new SAP can be created
   if (newSAPI == 0) {
      newSAPI = nextSAPI;
   }

   // Creation and initilisation
   result = (struct muxDemuxReceiverSAP_t *)sim_malloc(sizeof(struct muxDemuxReceiverSAP_t));
   result->identifier = newSAPI;
   result->sender = sender;

   // Now we search for its position within the file
   p = NULL; // A priori it is the first
   i = ndesObjectFile_createIterator(sender->sapList);
   s = (struct muxDemuxReceiverSAP_t *)ndesObject_getPrivate(ndesObjectFile_iteratorGetNext(i));
   while ((s != NULL) && (s->identifier < newSAPI)) {
      p=s;
      s = (struct muxDemuxReceiverSAP_t *)ndesObject_getPrivate(ndesObjectFile_iteratorGetNext(i));
   } 
   ndesObjectFile_deleteIterator(i);

   // Insertion
   if (p) {
      ndesObjectFile_insertAfter(sender->sapList, p, result);
   } else {
      ndesObjectFile_insert(sender->sapList, result);
   }
   return result;
}

/**
 * @brief The receiver input fonction
 */
int muxDemuxReceiverProcessPDU(void * rcv,
   			    getPDU_t getPDU,
			    void * source)
{
   struct muxDemuxReceiver_t    * receiver = (struct muxDemuxReceiver_t    *)rcv;
   struct muxDemuxReceiverSAP_t * s;
   struct PDU_t * pdu;
   struct muxDemuxEncaps_t *encaps;
   struct ndesObjectFileIterator_t *i;

   // First, let's get the PDU
   attention, bug a corriger : il faut paser un getPDU null, pas une pdu nulle pour le tes

   // Then we need to find the output SAP
   i = ndesObjectFile_createIterator(receiver->sapList);
   s = (struct muxDemuxReceiverSAP_t *)ndesObject_getPrivate(ndesObjectFile_iteratorGetNext(i));
   while ((s != NULL) && (s->identifier < newSAPI)) {
      p=s;
      s = (struct muxDemuxReceiverSAP_t *)ndesObject_getPrivate(ndesObjectFile_iteratorGetNext(i));
   } 
   ndesObjectFile_deleteIterator(i);

   // Is it just a test ?
   if (source == NULL) {
      return sender->destProcessPDU(sender->destination, NUL);
   } else {
      pdu = getPDU(source);
      // Encapsulation (overheadless)
      encaps = muxDemuxEncaps_create(sap, pdu);
      return sender->destProcessPDU(sender->destination, PDU_create(PDU_size(pdu), encaps));
   }
}
