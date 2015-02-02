/**
 * @file src-tcpss.c
 * @brief A very basic TCP source model (slow start only)
 *
 * This module implements a very basic TCP source model as described
 * in [1]. This model is relevant to study the consequences of short
 * connections on the client acces link (this is the point of [1]).
 *
 * [1] "cdma2000 Evaluation Methodology" 3GPP2
 * C. R1002-0. Version1.0. December 10, 2004
 */
#include <src-tcpss.h>
#include <event.h>
#include <motsim.h>
#include <file_pdu.h>

#define TCP_BASE_HEADER_SIZE 40

/**
 * @brief Definition of a source
 */
struct srcTCPSS_t {
   int                windowSize ;
   int                MSS;
   double             RTT;

   int                backlog;   //!< the number of unsent bytes
   struct filePDU_t * outputQueue; //!< An internal queue where
				   //!segments are queued waiting to
				   //!be actually sent

   void        * destination;    //!< L'objet auquel sont destinées les PDUs
   processPDU_t  destProcessPDU; //!< La fonction permettant de
				 //!signaler à la destination la
				 //!présence de la PDU
};


/**
 * @brief Creation/initialization of a source
 * @param MTU is the maximum transmission unit of the link
 * @param RTTmd is the Round Trip Time minus transmission time on the access link
 * @param initialWindow is the initial value of cwnd
 * @param destination is a pointer to the destination entity
 * @param destProcessPDU is the PDU processing function of the destination
 */
struct srcTCPSS_t * srcTCPss_create(int MTU,
                                    double RTTmd,
                                    int initialWindow,
                                    void * destination,
				    processPDU_t destProcessPDU)
{
   struct srcTCPSS_t * result = (struct srcTCPSS_t *) sim_malloc(sizeof(struct srcTCPSS_t ));

   result->windowSize = initialWindow;
   result->MSS = MTU - TCP_BASE_HEADER_SIZE ;
   result->RTT = RTTmd;
   result->backlog = 0;
   result->destination = destination;
   result->destProcessPDU = destProcessPDU;
   result->outputQueue = filePDU_create(NULL, NULL);

   return result;
}

struct PDU_t * srcTCPss_getPDU(void * src);

/**
 * @brief Build a segment and notify the destination
 * @param src is a pointer to the source
 */
void srcTCPss_sendSegment(struct srcTCPSS_t * src)
{
   int segmentSize; 
   struct PDU_t * segment;

   // Send no more than available and MSS
   segmentSize = min(src->backlog, src->MSS);

   if (segmentSize > 0) {
      printf_debug(DEBUG_ALWAYS, "Will send a segment of %d bytes (out of %d remaining)\n", segmentSize, src->backlog);

      src->backlog -= segmentSize;

      // We need to build this new segment
      segment = PDU_create(segmentSize, NULL);

      // We insert it in the outputQueue 
      filePDU_insert(src->outputQueue, segment);

      printf_debug(DEBUG_ALWAYS, "Segment in the queue, notifying the destination\n");

      // Now we notify the destination 
      src->destProcessPDU(src->destination, srcTCPss_getPDU, src);
   }
}

/**
 * @brief Send several segments
 * @param src is a pointer to the source
 * @param nb is the number of segments to send
 */
void srcTCPss_sendSegments(struct srcTCPSS_t * src, int nb)
{
   int n;

   for (n=0; n< nb; n++){
      srcTCPss_sendSegment(src);
   }
}

/**
 * @brief Send two segments
 * @param s is a pointer to the source
 */
void srcTCPss_send2Segments(void * s)
{
   struct srcTCPSS_t * src = (struct srcTCPSS_t *) s;

   srcTCPss_sendSegments(src, 2);
}

/**
 * @brief Simulate transmission of nbBytes bytes
 * @param src is a pointer to the source
 * @param nbBytes is the number of bytes to transmit
 */
void srcTCPss_sendFile(struct srcTCPSS_t * src,
                       int nbBytes)
{
   src->backlog = nbBytes;

   printf_debug(DEBUG_ALWAYS, "Will send %d bytes\n", nbBytes);

   // Send as many segments as possible
   srcTCPss_sendSegments(src, min(src->windowSize, nbBytes/src->MSS));

   // Record future window size for the next transmission
   src->windowSize += nbBytes/src->MSS;
}

/**
 * @brief get a PDU from a source
 * @param s is a pointer to the source
 * @result A PDU if available or NULL
 */
struct PDU_t * srcTCPss_getPDU(void * s)
{
   struct srcTCPSS_t * src = (struct srcTCPSS_t *) s;

   printf_debug(DEBUG_ALWAYS, "IN\n");

  // If a PDU is available, we send it and (slow start) schedule 2 new
  // PDUs, if available
  if (filePDU_length(src->outputQueue) > 0) {
     printf_debug(DEBUG_ALWAYS, "scheduling two more segments\n");
     event_add(srcTCPss_send2Segments, src, motSim_getCurrentTime() + src->RTT);
     printf_debug(DEBUG_ALWAYS, "returning a segment\n");
     return filePDU_extract(src->outputQueue);
  } else { // Should not occur ...
     printf_debug(DEBUG_ALWAYS, "OUT\n");
     return NULL;
  }
}
