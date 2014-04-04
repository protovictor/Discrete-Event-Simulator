/**
 * @file muxdemux.h
 * @brief A trivial multiplexing/demultiplexing protocol
 *
 */
#ifndef __DEF_MUXDEMUX__
#define __DEF_MUXDEMUX__

#include <pdu.h>
#include <ndesObject.h>
#include <ndesObjectFile.h>

/**
 * @brief A multiplexing sender
 *
 */
struct muxDemuxSender_t;

/**
 * @brief A multiplexer service access point
 */
struct muxDemuxSenderSAP_t;

/**
 * @brief The sender input fonction
 */
processPDU_t muxDemuxSenderProcessPDU;

/**
 * @brief Sender (multiplexer) creator
 */
struct muxDemuxSender_t * muxDemuxSender_Create(void * destination,
						processPDU_t destProcessPDU);

/**
 * @brief Creation of a new Service Access Point on a sender
 * @param sender Pointer to the associated sender
 * @param newSAPI SAP identifier (0 if unspecified)
 */
struct muxDemuxSenderSAP_t * muxDemuxSender_createNewSAP(struct muxDemuxSender_t * sender,
							 unsigned int newSAPI);

struct muxDemuxSenderSAP_t * muxDemuxSender_getSAP(struct muxDemuxSender_t *);

 
/**
 * @brief Input function for a sender
 */
int muxDemuxSenderSAP_processPDU(void * muxSenderSAP,
				 getPDU_t getPDU,
				 void * source);


/**
 * @brief Receiver (demultiplexer) creator
 */
struct muxDemuxReceiver_t * muxDemuxReceiver_Create(void * dest);



#endif
