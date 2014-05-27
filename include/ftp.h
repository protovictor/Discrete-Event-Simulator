#include <stdio.h>
#include "http.h"  // for the MSS value
#include "motsim.h"
#include "pdu.h"
#include "srv-gen.h"
#include "random-generator.h"
#include "date-generator.h"


#define MAX_FileSize 5000000 // 5 MB

struct PDUList_t;
struct FTP_Session_t;
/**
 * @brief The function initialises the parameters of a FTP Session model
 * @param dateGen a pointer to the date generator used to generate random dates
 * for each ftp session
 * @param sizeGen a pointer to the size generator used to generate random size
 * for the files that need to be transferred
 * @param destination the destination of the transmitted files
 * @param destProcessPDU the function that will process the transmitted files
 */
struct FTP_Session_t* FTP_Session_Create(struct dateGenerator_t *dateGen,
                                          struct randomGenerator_t *sizeGen,
                                          void *destination,
                                          processPDU_t destProcessPDU);

/**
 * @brief Starts a new FTP Session and prepares the date for the next one
 * @param Session a pointer to the structure that holds information about
 * a session's parameters
 */
void FTP_Session_Start(struct FTP_Session_t *Session);

/**
 * @brief The function splits a file into multiple PDUs of maximum MSS size
 * @param Session a pointer to the current session
 * @param size the size of the current file to transmit
 */
struct FTP_Session_t* FTP_Session_PDUList_Create(struct FTP_Session_t *Session, int size);

/**
 * @brief The function transmits a file
 * @param Session a pointer to the current session
 */
void FTP_Transmit_File(struct FTP_Session_t *Session);


struct PDU_t* FTP_Session_getPDU(struct FTP_Session_t *Session);

int FTP_GetSessionNr(struct FTP_Session_t *Session);
