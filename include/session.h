#include <random-generator.h>
#include <date-generator.h>
#include <pdu.h>
#include <pdu-sink.h>
#include <pdu-source.h>
#include <event.h>
#include <motsim.h>


/* We declare the structure which holds the list of sessions and their parameters */

struct SessionList_t;

/* We declare the structure which holds the list of sessions */
struct PDUSourceList_t;

/**
 * @brief This function creates an empty list of sessions 
 * @param dateGen the generator for each session start date
 * @param PDUdateGen the generator for each request/reply start date 
 * @param SizeGen the generator for each request/reply size packet
 * @param filePDU the filePDU which contains the PDUs
 */
struct SessionList_t* SessionList_Create(struct dateGenerator_t * dateGen,
                                      struct dateGenerator_t * PDUdateGen,
                                      struct randomGenerator_t * SizeGen,
                                      struct filePDU_t * filePDU);

/**
 * @brief The function starts building sessions and creating events for
 * the simulator
 * @param SessionList a pointer to the Session List (previously created)
 */
void SessionList_Start(struct SessionList_t *SessionList);

/**
 * @brief This function creates an empty list of different sessions
 * where each one of them is created separately and added to the list 
 * before the simulation starts.
 * Created for the case when different users are sending requests to a server,
 * but each one has a different behaviour (known at the beggining of the
 * simulation, a deterministic behaviour), or when the same server has many purposes to serve.
 */
struct SessionList_t* SessionList_CreateSpecific();

/**
 * @brief The function sets the start time of each session to be deterministic
 * (We specifically know the time when each session starts, and this is a member 
 * of the list startDate)
 * @param SessionList a pointer to the list of sessions 
 * @param startDate a pointer to the list of date values
 */
//void SessionList_setDeterministic(struct SessionList_t* SessionList, double *startDate);

/**
 * @brief The function sets the start date of each session to be randomly generated
 * @param SessionList a pointer to the list of sessions
 * @param dateGen a pointer to the date generator
 */
void SessionList_setStartDate(struct SessionList_t* SessionList, struct dateGenerator_t* dateGen);

/**
 * @brief The function sets the random generator for the packet's size
 * @param SessionList a pointer to the list of sessions
 * @param SizeGen a pointer to the random generator
 */
void SessionList_setSizeGen(struct SessionList_t* SessionList, struct randomGenerator_t* SizeGen);

/**
 * @brief The function sets the filePDU which holds the PDUs
 * @param SessionList a pointer to the list of sessions
 * @param filePDU holds the PDUs, it is used to create each session
 */
void SessionList_setFilePDU(struct SessionList_t* SessionList, struct filePDU_t* filePDU);

/**
 * @brief The function sets the date generator for the start of each request/reply in a session
 * @param SessionList a pointer to the list of sessions
 * @param PDUdateGen a pointer to the date generator for each sessions replies/requests
 */
void SessionList_setPDUdateGen(struct SessionList_t* SessionList, struct dateGenerator_t* PDUdateGen);

/**
 * @brief This function adds a new session to the list of sessions
 * @param SessionList a pointer to the list of sessions
 * @param PDUSource the source of requests from a session
 */
struct SessionList_t* SessionList_AddSession(struct SessionList_t* SessionList, struct PDUSource_t* PDUSource);

/**
 * @brief This function gets the next session from the list of sessions
 * @param SessionList a pointer to the list of sessions
 */
struct PDUSource_t* SessionList_GetSession(struct SessionList_t* SessionList);







